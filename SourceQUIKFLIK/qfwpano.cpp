#include "qfwpano.hpp"
#include "qfworld.hpp"
#include "quikflik.hpp"
#include "moov.hpp"
#include "pluginio.hpp"

#include <math.h>
#include <builtin.h>
#include <ithread.hpp>
#include <iwindow.hpp>
#include <iclipbrd.hpp>

#define NODE_DEFAULT_PAN 361.0

const float pi = 3.14159265;
const float radPerDeg = pi / 180.0;
const float keyboardFovStep = 5.0 / 6.0;

Boolean QFPanoramaRenderer::trigTablesLoaded = false;
fixed QFPanoramaRenderer::secant[TRIG_ENTRIES];

extern "C" void _System renderFrame(RendererSetup* setup);
extern "C" void _System quickRenderFrame(RendererSetup* setup);

/************************************************************************/
/* CONSTRUCTOR                                                          */
/************************************************************************/
QFPanoramaRenderer::QFPanoramaRenderer(QFWorldPlayer& qfWorldPlayer, MovieAtom& movieAtom, HMMIO movieFile)
                  : QFWorldRenderer(qfWorldPlayer, movieAtom, movieFile)
{
  panoramaSample = 0;
  setup = 0;
  filePosition = 0;
  panorama = 0;
  hotSpots = 0;
  frame = 0;
  panScale = .0;
  panLineBytes = hotSpotLineBytes = 0;
  hotSpotLoadBuffer = 0;
  hotSpotMap = 0;
  videoTimeScale = hotSpotTimeScale = 0;
  hotSpotSetup = 0;
  lastRenderedTilt = lastRenderedFov = -1.0;
  previousHotSpotID = 0;

  panoramaMediaInfo = panoramaTrack->mediaAtom->mediaInformationAtom->
    baseMediaInfoHeaderAtom->panoramaMediaInfoAtom;
  panoramaTimeScale = panoramaTrack->mediaAtom->mediaHeaderAtom.timeScale;
  panoramaMap = panoramaTrack->mediaAtom->mediaInformationAtom->sampleTableAtom;
  panoramaDescription = &panoramaMap->sampleDescriptionAtom->sampleDescriptionTable.
                        elementAtPosition(1)->panoramaDescription;

  minPanReg = panoramaDescription->hPanStart;
  maxPanReg = panoramaDescription->hPanEnd;
  minTilt = min(panoramaDescription->vPanBottom, panoramaDescription->vPanTop);
  maxTilt = max(panoramaDescription->vPanBottom, panoramaDescription->vPanTop);
  minFov = panoramaDescription->minimumZoom;
  if (!minFov)
    minFov = panoramaTrack->trackHeaderAtom.height * (maxTilt - minTilt) /
             (panoramaDescription->sceneSizeX * 8); //8 screen pixels/pan pel
  maxFovReg = panoramaDescription->maximumZoom;
  if (!maxFovReg || (maxFovReg > maxTilt - minTilt))
    maxFovReg = maxTilt - minTilt;

  panFovAdjustment = (float)(movieSize().width()) / movieSize().height();

  panScale = (panoramaDescription->sceneSizeY - 1) / (maxPanReg - minPanReg);
  panLineBytes = (panoramaDescription->sceneSizeX * sizeof(RGB) + 3) >> 2 << 2;

  // Override video map to that specified in the panorama description.  Set hot spot map.
  ITabularSequence<TrackAtom>::Cursor* trackCursor = movieAtom.trackList.newCursor();
  for (trackCursor->setToFirst(); trackCursor->isValid(); trackCursor->setToNext())
  {
    const TrackAtom& currentTrack = trackCursor->element();
    MediaInformationAtom* mia = currentTrack.mediaAtom->mediaInformationAtom;
    if (mia)
    {
      SampleTableAtom* current = mia->sampleTableAtom;
      if (current && current->chunkOffsetAtom && current->sampleSizeAtom &&
          current->sampleToChunkAtom && current->timeToSampleAtom)
      {
        if (currentTrack.trackHeaderAtom.track == panoramaDescription->sceneTrackID)
        {
          videoMap = current;
          videoTimeScale = currentTrack.mediaAtom->mediaHeaderAtom.timeScale;
        }
        else
          if (currentTrack.trackHeaderAtom.track == panoramaDescription->hotSpotTrackID)
          {
            hotSpotMap = current;
            hotSpotTimeScale = currentTrack.mediaAtom->mediaHeaderAtom.timeScale;
          }
      }
    }
  } /* While track */
  delete trackCursor;

  setCodec(videoMap, videoSetup);
  if (hotSpotMap)
  {
    hotSpotSetup = new CodecSetup(0);
    setCodec(hotSpotMap, hotSpotSetup);
    hotSpotLoadBuffer = new BYTE[hotSpotSetup->bufferRectl.xRight * hotSpotSetup->bufferRectl.yTop];
    hotSpotLineBytes = (panoramaDescription->hotSpotSizeX + 3) >> 2 << 2;
  }
}

/************************************************************************/
/* START                                                                */
/************************************************************************/
void QFPanoramaRenderer::start()
{
  LONG errorCode;

  qfwp.setScreenText("Preparing VR panorama...");

  // Create last-rendered offsets structure
  lastRender = new unsigned long[movieSize().width() * movieSize().height()];

  // Create renderer setup structure
  setup = new RendererSetup;
  setup->panoramaSpot = 0;
  setup->width1 = movieSize().width() + 1;
  setup->panLineBytes = panLineBytes;
  setup->totalBytes = panLineBytes * panoramaDescription->sceneSizeY;
  setup->lastRender = lastRender + (movieSize().height() + 1) / 2 * movieSize().width();
  setup->lastRenderLineBytes = movieSize().width() * sizeof(unsigned long);
  setup->secant = secant;
  setup->tiltBytes = new unsigned long[TRIG_ENTRIES];
  setup->panBytes = new unsigned long[TRIG_ENTRIES];

  float tanMaxTilt = _fptan(maxTilt * radPerDeg);
  unsigned long maxTiltPel = panoramaDescription->sceneSizeX - 1;
  float tiltScale = maxTiltPel / (tanMaxTilt - _fptan(minTilt * radPerDeg));
  float radPanScale = panScale / radPerDeg;

  for (int x = 0; x < TRIG_ENTRIES; x++)
  {
    float phi = _fasin(2.0 * x / TRIG_ENTRIES - 1.0);
    setup->tiltBytes[x] = min((unsigned long)((tanMaxTilt - _fptan(phi)) * tiltScale),
                              maxTiltPel) * sizeof(RGB);
    float theta = _fasin(float(x) / TRIG_ENTRIES);
    setup->panBytes[x] = (unsigned long)(theta * radPanScale) * panLineBytes;
    if (!trigTablesLoaded)
      secant[x] = float2fixed(1.0/_fcos(phi));
  }
  trigTablesLoaded = true;

  // Allocate panorama buffer
  panorama = new BYTE[setup->totalBytes];

  // Allocate hot spot buffer
  if (hotSpotMap)
  {
    ULONG hotSpotsSize = panoramaDescription->hotSpotSizeX * panoramaDescription->hotSpotSizeY;
    hotSpots = new BYTE[hotSpotsSize];
  }

  // Create frame buffer and last-rendered offset buffer
  errorCode = DiveAllocImageBuffer(dive, &frame, FOURCC_BGR3, movieSize().width(), movieSize().height(), 0, 0);

  // Load default node or only node
  nodeID = (panoramaMediaInfo ? panoramaMediaInfo->defNodeID : -1);
  initPan = NODE_DEFAULT_PAN;
  loading = true;
  delete loadThread;
  loadThread = new IThread(new IThreadMemberFn<QFPanoramaRenderer>(*this, QFPanoramaRenderer::load));
}

/************************************************************************/
/* LOAD                                                                 */
/************************************************************************/
void QFPanoramaRenderer::load()
{
  Boolean firstTime = (initPan == NODE_DEFAULT_PAN);
  float nodeTime = .0;

  if (nodeID > 0)
  {
    for (int i = panoramaMediaInfo->numEntries - 1;
         i >= 0 && panoramaMediaInfo->idToTime[i].nodeID != nodeID;
         i--);
    if (i < 0) //Could not find time for node ID
      return;
    else
      nodeTime = panoramaMediaInfo->idToTime[i].nodeTime;
  }

  try
  {
    // Load panorama track media
    delete panoramaSample;
    panoramaSample = new PanoramaSample();
    panoramaMap->seekByTime(nodeTime, SEEK_SET);
    filePosition = mmioSeek(movieFile_p, panoramaMap->newFilePosition, SEEK_SET);
    ULONG sampleSize = (panoramaMap->sampleSize ? panoramaMap->sampleSize : panoramaMap->sampleCursor->element());
    panoramaSample->load(movieFile_p, sampleSize);

    //Set default view to node default if not passed in
    if (firstTime)
    {
      initPan = panoramaSample->panoSampleHeaderAtom.defHPan;
      initTilt = panoramaSample->panoSampleHeaderAtom.defVPan;
      initFov = panoramaSample->panoSampleHeaderAtom.defZoom;
    }
    else
    {
      initFov = (initFov == .0 ? fov : initFov);
    }

    //Sometimes the pano media has bad values so assign reasonable defaults if they are out of range
    if (initPan >= minPanReg && initPan <= maxPanReg || (minPanReg == .0 && maxPanReg == 360.0))
    {
      defaultPan_p = 360.0 - initPan;
      for (;defaultPan_p >= 360.0; defaultPan_p -= 360.0);
      for (;defaultPan_p < .0; defaultPan_p += 360.0);
    }
    else
      defaultPan_p = 360.0 - (minPanReg + (maxPanReg - minPanReg) / 2);

    //Tilt
    if (initTilt >= minTilt && initTilt <= maxTilt)
      defaultTilt_p = initTilt;
    else
      defaultTilt_p = .0;

    //Unregistered values
    minPan = max(defaultPan_p - (maxPanReg - minPanReg) / 4.0, minPanReg);
    maxPan = min(defaultPan_p + (maxPanReg - minPanReg) / 4.0, maxPanReg);
    maxFov = min((maxPan - minPan) / panFovAdjustment, maxFovReg);

    //Fov
    if (initFov >= minFov && initFov <= maxFov)
      defaultFov_p = initFov;
    else
      defaultFov_p = maxFov;

    videoMap->seekByTime(nodeTime * videoTimeScale / panoramaTimeScale, SEEK_SET);
    loadPlane(panorama, setup->totalBytes, (panoramaDescription->sceneNumFramesX == 1 ? 0 : panLineBytes),
              videoMap, videoSetup, panoramaDescription->sceneNumFramesX, panoramaDescription->sceneNumFramesY);

    //Unlock registered version
    QUIKFLIK(this);

    if (hotSpotMap)
    {
      hotSpotMap->seekByTime(nodeTime * hotSpotTimeScale / panoramaTimeScale, SEEK_SET);
      loadPlane(hotSpots, panoramaDescription->hotSpotSizeX * panoramaDescription->hotSpotSizeY,
                panoramaDescription->hotSpotSizeX, hotSpotMap, hotSpotSetup,
                panoramaDescription->hotSpotNumFramesX, panoramaDescription->hotSpotNumFramesY);
    }

    // Show window
    if (firstTime)
    {
      qfwp.setScreenText("");
      window_p->setFocus();
      window_p->show();
      visibleRegionEnabled(false);
    }

    setView(defaultPan_p, defaultTilt_p, defaultFov_p);
    updateEnabled_p = true;
    update();

    if (firstTime)
    {
      WinSetVisibleRegionNotify(window_p->handle(), TRUE);
      IKeyboardHandler::handleEventsFor(window_p);
    }
  }
  catch (NoMoreData noMoreData) {}

  loading = false;
}

/************************************************************************/
/* LOAD PLANE                                                           */
/************************************************************************/
void QFPanoramaRenderer::loadPlane(PBYTE planeBuffer, ULONG planeSize, ULONG planeLineBytes,
                                   SampleTableAtom* trackMap, CodecSetup* codecSetup,
                                   ULONG numFramesX, ULONG numFramesY)
{
 ULONG bytesToRead;

  //Read dice into panorama buffer
  for (int dieY = numFramesY - 1; dieY >= 0; dieY--)
  {
    loadSpot = planeBuffer + planeSize * dieY / numFramesY; //Assumes 3 divides numFramesY
    for (int dieX = 1; dieX <= numFramesX; dieX++)
    {
      if (trackMap->chunkCursor->isValid())
      {
        if (trackMap->sampleSize) /* Fixed sample size media */
          bytesToRead = trackMap->sampleSize;
        else /* Using sample size table for media */
          bytesToRead = trackMap->sampleCursor->element();

        /* Seek to better-known file position, if available */
        if (filePosition != trackMap->newFilePosition)
          filePosition = mmioSeek(movieFile_p, trackMap->newFilePosition, SEEK_SET);

        /* Read */
        filePosition += mmioRead(movieFile_p, codecSetup->compressedFrame, bytesToRead);

        /* If instance is being deleted and no exception thrown (not plugin instance), get out of here */
        if (!loading)
          throw (NoMoreData());

        /* Load die */
        loadDie(bytesToRead, codecSetup, planeLineBytes);

        /* Update position tracking variables */
        trackMap->newFilePosition += bytesToRead;
        trackMap->currentTime += trackMap->averageDuration;
        trackMap->sampleInMedia++;
        trackMap->sampleInChunk++;
        if (!trackMap->sampleSize)
          trackMap->sampleCursor->setToNext();

        /* Update chunk cursors if we have hit the end of the chunk */
        if (trackMap->sampleInChunk > trackMap->samplesPerChunk)
        {
          trackMap->chunkCursor->setToNext();

          /* Update samples-per-chunk value, if necessary */
          if (trackMap->chunkCursor->isValid())
          {
            trackMap->newFilePosition = trackMap->chunkCursor->element();
            if (trackMap->sampleToChunkCursor->isValid() &&
                trackMap->chunkOffsetAtom->chunkOffsetTable.position(*(trackMap->chunkCursor)) ==
                trackMap->sampleToChunkCursor->element().firstChunk)
            {
              trackMap->samplesPerChunk = trackMap->sampleToChunkCursor->element().samplesPerChunk;
              trackMap->sampleToChunkCursor->setToNext();
            }
          }
          trackMap->sampleInChunk = 1;
        } /* New chunk */
      } /* trackMap->chunkCursor->isValid() */
      loadSpot += planeLineBytes / numFramesX;
    } /* row */
  } /* column */
}

/************************************************************************/
/* LOAD DIE                                                             */
/************************************************************************/
void QFPanoramaRenderer::loadDie(ULONG length, CodecSetup *codecSetup, ULONG planeLineBytes)
{
  LONG errorCode;
  PBYTE convertedBuffer;
  ULONG scanLineBytes, scanLines;

  // Decompress frame into codec-format buffer
  codecSetup->decompress.ulSrcBufLen = length;
  codecSetup->decompress.pSrcBuf = codecSetup->compressedFrame;
  codecSetup->decompress.ulDstBufLen = 0;

  if (codecSetup->diveInstance_p)
  {
    errorCode = DiveBeginImageBufferAccess(diveForLoading, codecSetup->decompressedFrame,
                                           (PBYTE*)&codecSetup->decompress.pDstBuf, &scanLineBytes, &scanLines);
    errorCode = codecSetup->sendCodecMessage(&codecSetup->codecHandle, MMIOM_CODEC_DECOMPRESS, (LONG)&codecSetup->decompress, 0);

    //Paste decompressed output into appropriate spot in panorama
    if (codecSetup->colorFormat == FOURCC_BGR3)
    {
      for (int line = 0; line < codecSetup->rectl.yTop; line++) //Invert codec output
        memcpy(loadSpot + line * panLineBytes,
               (PBYTE)(codecSetup->decompress.pDstBuf) + (codecSetup->rectl.yTop - line - 1) * scanLineBytes,
               scanLineBytes);
      errorCode = DiveEndImageBufferAccess(diveForLoading, codecSetup->decompressedFrame);
    }
    else //Use DIVE to convert to BGR3
    {
      errorCode = DiveEndImageBufferAccess(diveForLoading, codecSetup->decompressedFrame);
      errorCode = DiveBlitImage(diveForLoading, codecSetup->decompressedFrame, codecSetup->convertedFrame);
      errorCode = DiveBeginImageBufferAccess(diveForLoading, codecSetup->convertedFrame, &convertedBuffer, &scanLineBytes, &scanLines);
      if (planeLineBytes)
        for (int line = scanLines - codecSetup->rectl.yTop; line < scanLines; line++)
          memcpy(loadSpot + line * planeLineBytes, convertedBuffer + line * scanLineBytes, scanLineBytes);
      else
        memcpy(loadSpot, convertedBuffer + (scanLines - codecSetup->rectl.yTop) * scanLineBytes,
               scanLineBytes * codecSetup->rectl.yTop);
      errorCode = DiveEndImageBufferAccess(diveForLoading, codecSetup->convertedFrame);
    }
  }
  else //Non-DIVE codec setup (hot spots)
  {
    codecSetup->decompress.pDstBuf = hotSpotLoadBuffer;
    errorCode = codecSetup->sendCodecMessage(&codecSetup->codecHandle, MMIOM_CODEC_DECOMPRESS, (LONG)&codecSetup->decompress, 0);
    for (int line = 0; line < codecSetup->rectl.yTop; line++) //Invert codec output
      memcpy(loadSpot + line * hotSpotLineBytes,
             hotSpotLoadBuffer + (codecSetup->rectl.yTop - line - 1) * hotSpotSetup->bufferRectl.xRight,
             hotSpotSetup->bufferRectl.xRight);
  }
}
/************************************************************************/
/* MOVIE SIZE                                                           */
/************************************************************************/
ISize QFPanoramaRenderer::movieSize()
{
  return ISize(panoramaTrack->trackHeaderAtom.width, panoramaTrack->trackHeaderAtom.height);
}

/************************************************************************/
/* CHARACTER KEY PRESS                                                  */
/************************************************************************/
Boolean QFPanoramaRenderer::characterKeyPress(IKeyboardEvent &event)
{
  switch (event.character())
  {
    case '=':
    case '+':
      setView(currentPan(), currentTilt(), currentFov() * fovStep(QFWorldRenderer::keyboard));
      update();
      return true;

    case '-':
    case '_':
      setView(currentPan(), currentTilt(), currentFov() / fovStep(QFWorldRenderer::keyboard));
      update();
      return true;

    default:
      return false;
  }
}

/************************************************************************/
/* REFRESH                                                              */
/************************************************************************/
void QFPanoramaRenderer::refresh()
{
  LONG errorCode;

  if (frame)
    errorCode = DiveBlitImage(dive, frame, DIVE_BUFFER_SCREEN);
}

/************************************************************************/
/* UPDATE                                                               */
/************************************************************************/
void QFPanoramaRenderer::update()
{
  if (!updateEnabled_p)
    return;

  unsigned long panOffset = (unsigned long)((pan + minPanReg) * panScale) * panLineBytes;
  setup->panoramaSpot = panorama + panOffset;
  setup->startPanSpot = -panOffset;
  setup->endPanSpot = setup->totalBytes - panOffset;

  // Construct frame buffer from panorama buffer
  PBYTE frameBuffer;
  ULONG scanLines;
  LONG errorCode = DiveBeginImageBufferAccess(dive, frame, &frameBuffer, &setup->scanLineBytes, &scanLines);

  setup->scanLines1 = scanLines + 1;
  setup->frameBuffer = frameBuffer + (setup->scanLines1) / 2 * setup->scanLineBytes;

  if (tilt != lastRenderedTilt || fov != lastRenderedFov)
  {
    // Floating-point working variables
    float tiltRadians = tilt * radPerDeg;
    float sinTilt = _fsin(tiltRadians);
    float cosTilt = _fcos(tiltRadians);
    float tanTilt = _fptan(tiltRadians);
    float sin2Tilt = sinTilt * sinTilt;
    float cos2Tilt = cosTilt * cosTilt;
    float tan2Tilt = tanTilt * tanTilt;
    setup->tanTilt = float2fixed(tanTilt);
    setup->tan2Tilt1 = float2fixed824(tan2Tilt + 1.0);

    // Find vMin and vMax implied by the tilt and fov.  There are 2 solutions for each.
    float sin2TestPhi = pow(_fsin((tilt - fov / 2.0) * radPerDeg), 2.0);
    float discriminant = _fsqrt(max(.0, sin2Tilt - (sin2TestPhi - cos2Tilt) * (sin2TestPhi * tan2Tilt + sin2TestPhi - tan2Tilt)));
    float vMinPlus = (sinTilt + discriminant) / (sin2TestPhi - cos2Tilt);
    float vMinMinus = (sinTilt - discriminant) / (sin2TestPhi - cos2Tilt);

    sin2TestPhi = pow(_fsin((tilt + fov / 2.0) * radPerDeg), 2.0);
    discriminant = _fsqrt(max(.0, sin2Tilt - (sin2TestPhi - cos2Tilt) * (sin2TestPhi * tan2Tilt + sin2TestPhi - tan2Tilt)));
    float vMaxPlus = (sinTilt + discriminant) / (sin2TestPhi - cos2Tilt);
    float vMaxMinus = (sinTilt - discriminant) / (sin2TestPhi - cos2Tilt);

    float baseV = (vMinMinus > 0 || fabs(vMinPlus + vMaxPlus) < .0005 || fabs(vMinPlus + vMaxMinus) < .0005 ?
                   vMinPlus : vMinMinus);   // Find the pair where vMin == -vMax
    float deltaV = -2.0 * baseV / scanLines;

    //Enables a running computation of v^2 with no multiplications
    setup->twoDeltaV2 = float2fixed824(2.0 * deltaV * deltaV);
    setup->tiltedDeltaV = float2fixed(-2.0 * baseV * cosTilt / scanLines);
    setup->deltaU = float2fixed(deltaV);

    renderFrame(setup);

    lastRenderedTilt = tilt;
    lastRenderedFov = fov;
  }
  else
    quickRenderFrame(setup);

  errorCode = DiveEndImageBufferAccess(dive, frame);

  refresh();
}

/************************************************************************/
/* PAN STEP                                                             */
/************************************************************************/
float QFPanoramaRenderer::panStep(QFWorldRenderer::InputDevice inputDevice, float stepFov)
{
  if (stepFov == 0)
    stepFov = fov;

  switch (inputDevice)
  {
    case QFWorldRenderer::keyboard:
      return stepFov / 8.0;

    case QFWorldRenderer::mouse:
      return 4.0 * stepFov / window_p->size().width();

    case QFWorldRenderer::timer:
      return panScale;
  }

  return QFWorldRenderer::panStep(inputDevice, stepFov); //Never called
}

/************************************************************************/
/* TILT STEP                                                            */
/************************************************************************/
float QFPanoramaRenderer::tiltStep(QFWorldRenderer::InputDevice inputDevice, float stepFov)
{
  if (stepFov == 0)
    stepFov = fov;

  if (inputDevice == QFWorldRenderer::keyboard)
    return stepFov / 8.0;
  else
    return 2.0 * stepFov / window_p->size().height();
}

/************************************************************************/
/* FOV STEP                                                             */
/************************************************************************/
float QFPanoramaRenderer::fovStep(QFWorldRenderer::InputDevice inputDevice)
{
  if (inputDevice == QFWorldRenderer::keyboard)
    return keyboardFovStep;
  else
    return pow(maxFovReg / minFov, 1.0 / window_p->size().height());
}

/************************************************************************/
/* CAPTURE FRAME                                                        */
/************************************************************************/
void QFPanoramaRenderer::captureFrame(IString saveAsFileName, FOURCC ioProc)
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

  errorCode = DiveBeginImageBufferAccess(dive, frame, &frameBuffer, &scanLineBytes, &scanLines);

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

  errorCode = DiveEndImageBufferAccess(dive, frame);
}

/************************************************************************/
/* COPY FRAME                                                           */
/************************************************************************/
void QFPanoramaRenderer::copyFrame()
{
  IClipboard clipboard(window_p->handle());
  PBYTE frameBuffer;
  ULONG scanLineBytes, scanLines;
  BITMAPINFOHEADER2 bmp = {0};
  BITMAPINFOHEADER2 bmpInfo;
  HBITMAP hbm;
  LONG errorCode;

  errorCode = DiveBeginImageBufferAccess(dive, frame, &frameBuffer, &scanLineBytes, &scanLines);

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
  errorCode = DiveEndImageBufferAccess(dive, frame);
}

/************************************************************************/
/* HOT SPOT AT                                                          */
/************************************************************************/
long QFPanoramaRenderer::hotSpotAt(IPoint point)
{
  long hotSpotID = 0;

  if (hotSpots && setup && setup->panoramaSpot)
  {
    point = point * movieSize() / window_p->size();
    long lastRenderOffset = lastRender[point.x() + point.y() * movieSize().width()];
    if (lastRenderOffset >= setup->endPanSpot)
      lastRenderOffset -= setup->totalBytes;
    else
      if (lastRenderOffset < setup->startPanSpot)
        lastRenderOffset += setup->totalBytes;
    hotSpotID = hotSpots[(lastRenderOffset - setup->startPanSpot) / sizeof(RGB)];
  }

  if (hotSpotID != previousHotSpotID)
  {
    qfwp.qf.status(qfwp.qf.hotSpotURL(hotSpotID));
    previousHotSpotID = hotSpotID;
  }

  return hotSpotID;
}

/************************************************************************/
/* HOT SPOT CLICKED                                                     */
/************************************************************************/
void QFPanoramaRenderer::hotSpotClicked(long hotSpotID)
{
  //First, try to process as plugin-overridden link to another URL
  IString url = qfwp.qf.hotSpotURL(hotSpotID);
  if (url.length())
  {
    qfwp.qf.linkTo(url);
    return;
  }

  HotSpot *hotSpot = 0;
  PanoLink *panoLink = 0;

  if (!panoramaSample->hotSpotTableAtom)
    return;

  for (int i = 0; i < panoramaSample->hotSpotTableAtom->numHotSpots; i++)
  {
    if (panoramaSample->hotSpotTableAtom->hotSpots[i].hotSpotID == hotSpotID)
    {
      hotSpot = &panoramaSample->hotSpotTableAtom->hotSpots[i];
      break;
    }
  }

  if (!hotSpot)
    return;

  if (!memcmp(&hotSpot->type, "link", 4))
  {
    for (int i = 0; i < panoramaSample->linkTableAtom->numLinks; i++)
    {
      if (panoramaSample->linkTableAtom->links[i].linkID == hotSpot->typeData)
      {
        panoLink = &panoramaSample->linkTableAtom->links[i];
        break;
      }
    }

    if (!panoLink)
      return;

    updateEnabled_p = false;
    WinSetPointer(IWindow::desktopWindow()->handle(), ISystemPointerHandle(ISystemPointerHandle::wait));

    nodeID = panoLink->toNodeID;
    initPan = panoLink->toHPan;
    initTilt = panoLink->toVPan;
    initFov = panoLink->toZoom;
    loading = true;
    delete loadThread;
    loadThread = new IThread(new IThreadMemberFn<QFPanoramaRenderer>(*this, QFPanoramaRenderer::load));
  }
}

/************************************************************************/
/* DESTRUCTOR                                                           */
/************************************************************************/
QFPanoramaRenderer::~QFPanoramaRenderer()
{
  if (loading)
  {
    loading = false;
    IThread::current().waitFor(*loadThread);
    delete loadThread;
  }

  if (window_p)
  {
    WinSetVisibleRegionNotify(window_p->handle(), FALSE);
    IKeyboardHandler::stopHandlingEventsFor(window_p);
  }

  if (setup)
  {
    delete[] setup->tiltBytes;
    delete[] setup->panBytes;
    delete setup;
  }

  delete[] panorama;
  delete[] hotSpots;
  delete[] hotSpotLoadBuffer;
  delete[] lastRender;
  delete hotSpotSetup;
  delete panoramaSample;

  if (frame)
    DiveFreeImageBuffer(dive, frame);
}

