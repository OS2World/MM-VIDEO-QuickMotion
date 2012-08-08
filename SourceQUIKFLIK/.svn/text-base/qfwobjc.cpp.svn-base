#include "qfwobjc.hpp"
#include "qfworld.hpp"
#include "moov.hpp"
#include "pluginio.hpp"

#include <ithread.hpp>
#include <iwindow.hpp>
#include <iclipbrd.hpp>

QFObjectRenderer::QFObjectRenderer(QFWorldPlayer& qfWorldPlayer, MovieAtom& movieAtom, HMMIO movieFile)
                : QFWorldRenderer(qfWorldPlayer, movieAtom, movieFile)
{
  columns = rows = 0;
  currentColumn = currentRow = -1;
  degreesPerColumn = degreesPerRow = .0;
  filePosition = 0;
  frame = 0;

  if (movieAtom_p.userDataAtom && movieAtom_p.userDataAtom->navigationAtom)
  {
    const NavigationAtom& navigationAtom = *(movieAtom_p.userDataAtom->navigationAtom);

    columns = navigationAtom.numberOfColumns;
    minPanReg = navigationAtom.startHPan;
    maxPanReg = navigationAtom.endHPan;
    if (columns > 1)
      degreesPerColumn = (maxPanReg - minPanReg) /
                         (minPanReg == .0 && maxPanReg == 360.0 ? columns : columns - 1);

    rows = navigationAtom.numberOfRows;
    minTilt = navigationAtom.startVPan;
    maxTilt = navigationAtom.endVPan;
    if (rows > 1)
      degreesPerRow = (maxTilt - minTilt) / (rows - 1);

    defaultPan_p = navigationAtom.initialHPan;
    defaultTilt_p = navigationAtom.initialVPan;

    //Lock to a view 1/2 of pan range, centered on default pan
    minPan = max(defaultPan_p - (maxPanReg - minPanReg) / 4.0, minPanReg);
    maxPan = min(defaultPan_p + (maxPanReg - minPanReg) / 4.0, maxPanReg);
  }

  setCodec(videoMap, videoSetup);
}

void QFObjectRenderer::start()
{
  loading = true;
  delete loadThread;
  loadThread = new IThread(new IThreadMemberFn<QFObjectRenderer>(*this, QFObjectRenderer::load));
}

void QFObjectRenderer::load()
{
  ULONG column, row, bytesToRead;
  LONG errorCode;

  qfwp.setScreenText("Preparing VR object...");

  // Allocate array of image buffer numbers and temporary buffer for compressed frames
  frame = new PULONG[columns];
  for (column = 0; column < columns; column++)
    frame[column] = 0;
  buffer = new BYTE**[columns];

  try
  {
    //Read video track into image buffers
    for (column = 0; column < columns; column++)
    {
      frame[column] = new ULONG[rows];
      for (row = 0; row < rows; row++)
        frame[column][row] = 0;
      buffer[column] = new BYTE*[rows];

      for (row = 0; row < rows; row++)
      {
        if (videoMap->chunkCursor->isValid())
        {
          if (videoMap->sampleSize) /* Fixed sample size media */
            bytesToRead = videoMap->sampleSize;
          else /* Using sample size table for media */
            bytesToRead = videoMap->sampleCursor->element();

          /* Seek to better-known file position, if available */
          if (filePosition != videoMap->newFilePosition)
            filePosition = mmioSeek(movieFile_p, videoMap->newFilePosition, SEEK_SET);

          /* Read */
          filePosition += mmioRead(movieFile_p, videoSetup->compressedFrame, bytesToRead);

          /* If instance is being deleted and no exception thrown (not plugin instance), get out of here */
          if (!loading)
            throw (NoMoreData());

          /* Prepare frame */
          prepareFrame(column, row, bytesToRead);

          /* Update position tracking variables */
          videoMap->newFilePosition += bytesToRead;
          videoMap->currentTime += videoMap->averageDuration;
          videoMap->sampleInMedia++;
          videoMap->sampleInChunk++;
          if (!videoMap->sampleSize)
            videoMap->sampleCursor->setToNext();

          /* Update chunk cursors if we have hit the end of the chunk */
          if (videoMap->sampleInChunk > videoMap->samplesPerChunk)
          {
            videoMap->chunkCursor->setToNext();

            /* Update samples-per-chunk value, if necessary */
            if (videoMap->chunkCursor->isValid())
            {
              videoMap->newFilePosition = videoMap->chunkCursor->element();
              if (videoMap->sampleToChunkCursor->isValid() &&
                  videoMap->chunkOffsetAtom->chunkOffsetTable.position(*(videoMap->chunkCursor)) ==
                  videoMap->sampleToChunkCursor->element().firstChunk)
             {
                videoMap->samplesPerChunk = videoMap->sampleToChunkCursor->element().samplesPerChunk;
                videoMap->sampleToChunkCursor->setToNext();
              }
            }
            videoMap->sampleInChunk = 1;
          } /* New chunk */
        } /* videoMap->chunkCursor->isValid() */
        else
        {
          // Came up short on data.  Adjust last column and step.
          columns = column;
          degreesPerColumn = ((columns < 2) ? .0 :
                              (maxPanReg - minPanReg) /
                              (minPanReg == .0 && maxPanReg == 360.0 ? columns : columns - 1));
          break;
        }
      } /* row */
    } /* column */

    //Unlock registered version
    QUIKFLIK(this);

    qfwp.setScreenText("");
    window_p->setFocus();
    window_p->show();
    visibleRegionEnabled(false); // Set up blitter first time
    setView(defaultPan_p, defaultTilt_p, defaultFov_p);
    update();
    WinSetVisibleRegionNotify(window_p->handle(), TRUE);
  }
  catch (NoMoreData noMoreData) {}

  loading = false;
}

void QFObjectRenderer::prepareFrame(ULONG column, ULONG row, ULONG length)
{
  LONG errorCode;
  PBYTE convertedBuffer;
  ULONG scanLineBytes, scanLines;

  // Decompress frame into codec-format buffer
  videoSetup->decompress.ulSrcBufLen = length;
  videoSetup->decompress.pSrcBuf = videoSetup->compressedFrame;
  videoSetup->decompress.ulDstBufLen = 0;
  errorCode = DiveBeginImageBufferAccess(diveForLoading, videoSetup->decompressedFrame,
                                         (PBYTE*)&videoSetup->decompress.pDstBuf, &scanLineBytes, &scanLines);
  errorCode = videoSetup->sendCodecMessage(&videoSetup->codecHandle, MMIOM_CODEC_DECOMPRESS,
                                          (LONG)&videoSetup->decompress, 0);

  if (videoSetup->colorFormat == FOURCC_BGR3)
  {
    PBYTE bufferSpot = buffer[column][row] = new BYTE[scanLineBytes * videoSetup->rectl.yTop];
    for (int line = 0; line < videoSetup->rectl.yTop; line++) //Invert codec output
      memcpy(bufferSpot + line * scanLineBytes,
               (PBYTE)(videoSetup->decompress.pDstBuf) + (videoSetup->rectl.yTop - line - 1) * scanLineBytes,
               scanLineBytes);
    errorCode = DiveEndImageBufferAccess(diveForLoading, videoSetup->decompressedFrame);
  }
  else
  {
    errorCode = DiveEndImageBufferAccess(diveForLoading, videoSetup->decompressedFrame);
    errorCode = DiveBlitImage(diveForLoading, videoSetup->decompressedFrame, videoSetup->convertedFrame);
    errorCode = DiveBeginImageBufferAccess(diveForLoading, videoSetup->convertedFrame, &convertedBuffer, &scanLineBytes, &scanLines);
    buffer[column][row] = new BYTE[scanLineBytes * videoSetup->rectl.yTop];
    memcpy(buffer[column][row], convertedBuffer + (scanLines - videoSetup->rectl.yTop) * scanLineBytes,
           scanLineBytes * videoSetup->rectl.yTop);
    errorCode = DiveEndImageBufferAccess(diveForLoading, videoSetup->convertedFrame);
  }

  // Create permanent run-time buffer from regular buffer
  errorCode = DiveAllocImageBuffer(dive, &frame[column][row], FOURCC_BGR3, movieSize().width(), movieSize().height(),
                                   scanLineBytes, buffer[column][row]);
}

ISize QFObjectRenderer::movieSize()
{
  const ImageDescription& imageDescription = videoMap->sampleDescriptionAtom->
    sampleDescriptionTable.elementAtPosition(1)->imageDescription;

  return ISize(imageDescription.width, imageDescription.height);
}

void QFObjectRenderer::refresh()
{
  LONG errorCode;

  if (currentColumn >= 0 && frame && frame[currentColumn] && frame[currentColumn][currentRow])
    errorCode = DiveBlitImage(dive, frame[currentColumn][currentRow], DIVE_BUFFER_SCREEN);
}

/************************************************************************/
/* UPDATE                                                               */
/************************************************************************/
void QFObjectRenderer::update()
{
  LONG hypotheticalColumn = (degreesPerColumn == .0 ? .0 : (pan - minPanReg) / degreesPerColumn);
  LONG hypotheticalRow = (degreesPerRow == .0 ? .0 : (tilt - minTilt) / degreesPerRow);

  if (currentColumn == hypotheticalColumn && currentRow == hypotheticalRow)
    return;

  currentColumn = hypotheticalColumn;
  currentRow = hypotheticalRow;

  refresh();
}

/************************************************************************/
/* PAN STEP                                                             */
/************************************************************************/
float QFObjectRenderer::panStep(QFWorldRenderer::InputDevice inputDevice, float stepFov)
{
  if (inputDevice == QFWorldRenderer::keyboard)
    return -degreesPerColumn;
  else
    return -180.0 / window_p->size().width();
}

/************************************************************************/
/* TILT STEP                                                            */
/************************************************************************/
float QFObjectRenderer::tiltStep(QFWorldRenderer::InputDevice inputDevice, float stepFov)
{
  if (inputDevice == QFWorldRenderer::keyboard)
    return degreesPerRow;
   else
    return 180.0 / window_p->size().height();
}

/************************************************************************/
/* CAPTURE FRAME                                                        */
/************************************************************************/
void QFObjectRenderer::captureFrame(IString saveAsFileName, FOURCC ioProc)
{
  HMMIO frameFile;
  MMIOINFO info = {0};
  LONG bytesWritten;
  MMIMAGEHEADER header = {0};
  PBYTE frameBuffer;
  ULONG scanLineBytes, scanLines;
  LONG errorCode;

  info.fccIOProc = ioProc;
  info.ulTranslate = MMIO_TRANSLATEHEADER | MMIO_TRANSLATEDATA;

  errorCode = DiveBeginImageBufferAccess(dive, frame[currentColumn][currentRow],
                                         &frameBuffer, &scanLineBytes, &scanLines);

  header.ulHeaderLength = sizeof(MMIMAGEHEADER);
  header.ulContentType = MMIO_IMAGE_PHOTO;
  header.ulMediaType = MMIO_MEDIATYPE_IMAGE;
  header.mmXDIBHeader.XDIBHeaderPrefix.ulMemSize = scanLineBytes * scanLines; /*  Length of bitmap. */
  header.mmXDIBHeader.XDIBHeaderPrefix.ulPelFormat = FOURCC_BGR3; /*  FOURCC code defining the pel format. */
  header.mmXDIBHeader.BMPInfoHeader2.cbFix = sizeof (BITMAPINFOHEADER2);
  header.mmXDIBHeader.BMPInfoHeader2.cx = movieSize().width();
  header.mmXDIBHeader.BMPInfoHeader2.cy = movieSize().height();
  header.mmXDIBHeader.BMPInfoHeader2.cPlanes = 1; /*  Number of planes. */
  header.mmXDIBHeader.BMPInfoHeader2.cBitCount = 24; /*  Bits per pel. */
  header.mmXDIBHeader.BMPInfoHeader2.ulCompression = BCA_UNCOMP;
  header.mmXDIBHeader.BMPInfoHeader2.cbImage = header.mmXDIBHeader.XDIBHeaderPrefix.ulMemSize;
  header.mmXDIBHeader.BMPInfoHeader2.usRecording = BRA_BOTTOMUP; /*  Must be BRA_BOTTOMUP. */
  header.mmXDIBHeader.BMPInfoHeader2.usRendering = BRH_NOTHALFTONED; /*  Not used. */

  frameFile = mmioOpen(saveAsFileName, &info, MMIO_CREATE | MMIO_WRITE);
  mmioSetHeader(frameFile, &header, sizeof(MMIMAGEHEADER), &bytesWritten, 0, 0);
  mmioWrite(frameFile, frameBuffer, header.mmXDIBHeader.XDIBHeaderPrefix.ulMemSize);
  mmioClose(frameFile, 0);

  errorCode = DiveEndImageBufferAccess(dive, frame[currentColumn][currentRow]);
}

/************************************************************************/
/* COPY FRAME                                                           */
/************************************************************************/
void QFObjectRenderer::copyFrame()
{
  IClipboard clipboard(window_p->handle());
  PBYTE frameBuffer;
  ULONG scanLineBytes, scanLines;
  BITMAPINFOHEADER2 bmp = {0};
  BITMAPINFOHEADER2 bmpInfo;
  HBITMAP hbm;
  LONG errorCode;

  errorCode = DiveBeginImageBufferAccess(dive, frame[currentColumn][currentRow],
                                         &frameBuffer, &scanLineBytes, &scanLines);

  bmp.cbFix = sizeof(BITMAPINFOHEADER2);
  bmp.cx = movieSize().width();
  bmp.cy = movieSize().height();
  bmp.cPlanes = 1;
  bmp.cBitCount = 24;
  bmp.ulCompression = BCA_UNCOMP;
  bmp.cbImage = scanLineBytes * scanLines;
  bmp.usRecording = BRA_BOTTOMUP; /*  Must be BRA_BOTTOMUP. */
  bmp.usRendering = BRH_NOTHALFTONED; /*  Not used. */
  bmp.ulColorEncoding = BCE_RGB;

  bmpInfo = bmp;

  IGraphicContext context;
  hbm = GpiCreateBitmap(context.handle(), &bmp, CBM_INIT, frameBuffer, (PBITMAPINFO2)&bmpInfo);

  clipboard.setBitmap(hbm);

  GpiDeleteBitmap(hbm);
  errorCode = DiveEndImageBufferAccess(dive, frame[currentColumn][currentRow]);
}

/************************************************************************/
/* DESTRUCTOR                                                           */
/************************************************************************/
QFObjectRenderer::~QFObjectRenderer()
{
  if (loading)
  {
    loading = false;
    IThread::current().waitFor(*loadThread);
    delete loadThread;
  }

  if (frame)
  {
    for (ULONG column = 0; column < columns; column++)
    {
      if (frame[column])
      {
        for (ULONG row = 0; row < rows; row++)
        {
           if (frame[column][row])
           {
             DiveFreeImageBuffer(dive, frame[column][row]);
             delete[] buffer[column][row];
           }
        }
        delete[] frame[column];
        delete[] buffer[column];
      }
    }
  }

  delete[] frame;
  delete[] buffer;
}

