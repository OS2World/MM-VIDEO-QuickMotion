#include "quikflik.h"
#include "qfwrend.hpp"
#include "moov.hpp"

#include <iwindow.hpp>
#include <ithread.hpp>


/************************************************************************/
/* CodecSetup CONSTRUCTOR                                               */
/************************************************************************/
CodecSetup::CodecSetup(HDIVE diveInstance)
{
  diveInstance_p = diveInstance;
  codecHandle = 0;
  sendCodecMessage = 0;
  memset(&decompress, '\0', sizeof(decompress));
  memset(&videoDecompress, '\0', sizeof(videoDecompress));
  memset(&rectl, '\0', sizeof(rectl));
  memset(&bufferRectl, '\0', sizeof(bufferRectl));
  compressedFrame = 0;
  decompressedFrame = 0;
  convertedFrame = 0;
  memset(&setupBlitterForLoading_p, '\0', sizeof(setupBlitterForLoading_p));
  colorFormat = 0;
}

/************************************************************************/
/* CodecSetup DESTRUCTOR                                                */
/************************************************************************/
CodecSetup::~CodecSetup()
{
  if (decompressedFrame)
    DiveFreeImageBuffer(diveInstance_p, decompressedFrame);

  if (convertedFrame)
    DiveFreeImageBuffer(diveInstance_p, convertedFrame);

  delete[] compressedFrame;

  if (sendCodecMessage)
    sendCodecMessage(&codecHandle, MMIOM_CODEC_CLOSE, 0, 0);
}

/************************************************************************/
/* CCONSTRUCTOR                                                         */
/************************************************************************/
QFWorldRenderer::QFWorldRenderer(QFWorldPlayer& qfWorldPlayer, MovieAtom& movieAtom, HMMIO movieFile)
               : qfwp(qfWorldPlayer),
                 movieAtom_p(movieAtom),
                 movieFile_p(movieFile)
{
  window_p = 0;
  dive = diveForLoading = 0;
  paletteHandle_p = 0;
  panoramaTrack = 0;
  videoMap = 0;
  videoSetup = 0;
  pan = tilt = fov = .0;
  minPan = minTilt = minFov = maxPan = maxTilt = maxFov = maxFovReg = .0;
  panFovAdjustment = 1.0;
  defaultPan_p = defaultTilt_p = defaultFov_p = .0;
  updateEnabled_p = true;
  loadThread = 0;
  loading = false;

  usingPaletteManager_p = false;
  paletteHandle_p = 0;

  // Initialize cursors.  Find video map.
  ITabularSequence<TrackAtom>::Cursor* trackCursor = movieAtom_p.trackList.newCursor();
  for (trackCursor->setToFirst(); trackCursor->isValid(); trackCursor->setToNext())
  {
    if (!memcmp(trackCursor->element().mediaAtom->mediaHandlerReferenceAtom->componentSubType, "STpn", 4))
      panoramaTrack = (TrackAtom*)&trackCursor->element();

    MediaInformationAtom* mia = trackCursor->element().mediaAtom->mediaInformationAtom;
    if (mia)
    {
      SampleTableAtom* current = mia->sampleTableAtom;
      if (current && current->chunkOffsetAtom && current->sampleSizeAtom &&
          current->sampleToChunkAtom && current->timeToSampleAtom)
      {
        current->track = &trackCursor->element();
        current->chunkCursor = current->chunkOffsetAtom->chunkOffsetTable.newCursor();
        current->chunkCursor->setToFirst();
        if (current->chunkCursor->isValid())
          current->newFilePosition = current->chunkCursor->element();
        current->sampleSize = current->sampleSizeAtom->sampleSize;
        current->sampleCursor = current->sampleSizeAtom->sampleSizeTable.newCursor();
        current->sampleCursor->setToFirst();
        current->sampleToChunkCursor = current->sampleToChunkAtom->sampleToChunkTable.newCursor();
        current->sampleToChunkCursor->setToFirst();
        current->samplesPerChunk = current->sampleToChunkCursor->element().samplesPerChunk;
        current->sampleToChunkCursor->setToNext();
        current->sampleInChunk = 1;
        current->sampleInMedia = 1;
        current->currentTime = .0;

        /* Set video track shortcut to this track, if appropriate */
        if (mia->videoMediaInfoHeaderAtom && !videoMap)
          videoMap = current;
      } /* If current */
    } /* If mia */
  } /* While track */
  delete trackCursor;

  if (!videoMap) throw(Unsupported("Movie does not contain video."));

  // Set up DIVE
  FOURCC fccFormats[100] = {0};
  DIVE_CAPS diveCaps = {0};
  ULONG errorCode;

  diveCaps.ulStructLen = sizeof(DIVE_CAPS);
  diveCaps.ulFormatLength = 400;
  diveCaps.pFormatData = fccFormats;

  errorCode = DiveQueryCaps(&diveCaps, DIVE_BUFFER_SCREEN);
  if (errorCode || (diveCaps.ulDepth < 8)) throw(Unsupported("No DIVE support found on this system."));

  errorCode = DiveOpen(&diveForLoading, TRUE, 0);
  if (errorCode) throw(Unsupported("DIVE could not be opened."));

  errorCode = DiveOpen(&dive, FALSE, 0);
  if (errorCode)
  {
    DiveClose(diveForLoading);
    throw(Unsupported("DIVE could not be opened."));
  }

  usingPaletteManager_p = (diveCaps.ulDepth < 16);
  if (usingPaletteManager_p)
    paletteHandle_p = GpiCreatePalette(IThread::current().anchorBlock(), 0,
                                       LCOLF_CONSECRGB, MAX_PALETTE, (PULONG)paletteEntries);

  //Create primary codec object
  videoSetup = new CodecSetup(diveForLoading);

  //Initialize screen blitter
  memset(&setupBlitter_p, '\0', sizeof(setupBlitter_p));
  setupBlitter_p.ulStructLen = sizeof(SETUP_BLITTER);
  setupBlitter_p.fInvert = TRUE;
  setupBlitter_p.ulSrcPosX = 0;
  setupBlitter_p.ulSrcPosY = 0;
  setupBlitter_p.ulDitherType = 1;
  setupBlitter_p.lDstPosX = 0;
  setupBlitter_p.lDstPosY = 0;
  setupBlitter_p.lScreenPosX = 0;
  setupBlitter_p.lScreenPosY = 0;
  setupBlitter_p.fccSrcColorFormat = FOURCC_BGR3;
  setupBlitter_p.fccDstColorFormat = FOURCC_SCRN;
}

/************************************************************************/
/* SET CODEC                                                            */
/************************************************************************/
//This function should be a member of CodecSetup
void QFWorldRenderer::setCodec(SampleTableAtom* trackMap, CodecSetup* codecSetup)
{
  ULONG errorCode;

  // Set up palette
  const ImageDescription& imageDescription = trackMap->sampleDescriptionAtom->
    sampleDescriptionTable.elementAtPosition(1)->imageDescription;
  ULONG bitCount = (imageDescription.depth == 32 ? 32 : imageDescription.depth & 0x1F);

  SHORT colorTable = (bitCount < 16 && imageDescription.colorTable == -1 ?
    bitCount : imageDescription.colorTable); /* Force default palette if bitcount < 16 and no palette given */

  memset(&paletteEntries, '\0', sizeof(paletteEntries));
  memset(&palette_p, '\0', sizeof(palette_p));
  if (colorTable == 0) /*Special movie palette stored in sample description */
  {
    palette_p.ulNumColors = imageDescription.firstColorEntry.blue + 1;
    palette_p.prgb2Entries = paletteEntries;
    for (USHORT paletteEntry = 0; paletteEntry < palette_p.ulNumColors; paletteEntry++)
    {
      paletteEntries[paletteEntry].bBlue = imageDescription.palette[paletteEntry].blue;
      paletteEntries[paletteEntry].bGreen = imageDescription.palette[paletteEntry].green;
      paletteEntries[paletteEntry].bRed = imageDescription.palette[paletteEntry].red;
      paletteEntries[paletteEntry].fcOptions = 0;
    }
  }
  else if (colorTable != -1) /* Specific palette */
  {
    if (getDefaultPalette(colorTable, paletteEntries, sizeof(RGB2) << bitCount) == MMIO_SUCCESS)
    {
      palette_p.ulNumColors = 1 << bitCount;
      palette_p.prgb2Entries = paletteEntries;
    }
  }

  // Open CODEC
  ULONG codecDepthFlag;
  memset(&codecIniFileInfo, '\0', sizeof(codecIniFileInfo));
  memset(&sourceHeader, '\0', sizeof(sourceHeader));
  memset(&destinationHeader, '\0', sizeof(destinationHeader));
  codecIniFileInfo.ulStructLen = sizeof(codecIniFileInfo);
  codecIniFileInfo.fcc = mmioFOURCC('M', 'o', 'o', 'V');
  codecIniFileInfo.ulCompressType = imageDescription.codecFourcc;
  codecIniFileInfo.ulCapsFlags = CODEC_DECOMPRESS | CODEC_ORIGIN_LOWERLEFT;

  switch (bitCount)
  {
    case 1:
      codecDepthFlag = CODEC_4_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_PALETTIZED;
      break;
    case 2:
      codecDepthFlag = CODEC_4_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_PALETTIZED;
      break;
    case 4:
      codecDepthFlag = CODEC_4_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_PALETTIZED;
      break;
    case 8:
      codecDepthFlag = CODEC_8_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_PALETTIZED;
      break;
    case 16:
      codecDepthFlag = CODEC_16_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_RGB_5_6_5;
      break;
    case 24:
      codecDepthFlag = CODEC_24_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_RGB_24;
      break;
    case 32:
      codecDepthFlag = CODEC_24_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_RGB_24;
      break;
  };
  codecIniFileInfo.ulCapsFlags |= codecDepthFlag;

  codecModule = 0;
  codecSetup->sendCodecMessage = mmioLoadCODECProc(&codecIniFileInfo, &codecModule,
    MMIO_MATCHFOURCC | MMIO_MATCHCOMPRESSTYPE | MMIO_MATCHCAPSFLAGS);
  if (!codecSetup->sendCodecMessage) throw(Unsupported("Unsupported codec."));

  codecSetup->colorFormat = codecIniFileInfo.fccPreferredFormat;

  memset(&codecOpen, '\0', sizeof(codecOpen));
  codecOpen.pControlHdr = (PVOID)(mmioFOURCC('q','k','f','l'));
  codecOpen.pSrcHdr = &sourceHeader;
  sourceHeader.ulStructLen = sizeof(CODECVIDEOHEADER);
  sourceHeader.cx = imageDescription.width;
  sourceHeader.cy = imageDescription.height;
  sourceHeader.cPlanes = 1;
  sourceHeader.cBitCount = bitCount;
  sourceHeader.ulColorEncoding = (codecSetup->diveInstance_p ? MMIO_COMPRESSED : MMIO_PALETTIZED);
  sourceHeader.genpal.ulStartIndex = palette_p.ulStartIndex;
  sourceHeader.genpal.ulNumColors = palette_p.ulNumColors;
  sourceHeader.genpal.prgb2Entries = palette_p.prgb2Entries;

  codecOpen.pDstHdr = &destinationHeader;
  destinationHeader.ulStructLen = sizeof(CODECVIDEOHEADER);
  destinationHeader.cx = imageDescription.width;
  destinationHeader.cy = imageDescription.height;
  destinationHeader.cPlanes = 1;

  /* Override output color space if R565 is preferred by the CODEC */
  if (codecSetup->colorFormat == FOURCC_R565 ||
      codecSetup->colorFormat == FOURCC_R555 ||
      codecSetup->colorFormat == FOURCC_YUV9)
  {
    codecDepthFlag = CODEC_16_BIT_COLOR;
    destinationHeader.cBitCount = 16;
    destinationHeader.genpal.ulNumColors = 1 << 16;
    destinationHeader.ulColorEncoding = MMIO_RGB_5_6_5;
  }
  else
  {
    destinationHeader.cBitCount = bitCount;
  }

  codecOpen.ulFlags = codecDepthFlag | CODEC_DECOMPRESS | CODEC_ORIGIN_LOWERLEFT;
  codecOpen.ulFlags |= (codecIniFileInfo.ulCapsFlags &
    (CODEC_MULAPERTURE | CODEC_SELFHEAL | CODEC_WINDOW_CLIPPING | CODEC_PALETTE_TRANS));

  errorCode = codecSetup->sendCodecMessage(&codecSetup->codecHandle, MMIOM_CODEC_OPEN, (LONG)&codecOpen, 0);
  if (errorCode)
  {
    codecSetup->sendCodecMessage = 0;
    throw(Unsupported("Codec could not be opened."));
  }

  // Set up comressed and decompressed buffers
  codecSetup->compressedFrame = new CHAR[trackMap->maxSampleSize];

  codecSetup->rectl.xLeft = 0;
  codecSetup->rectl.yBottom = 0;
  codecSetup->rectl.xRight = imageDescription.width;
  codecSetup->rectl.yTop = imageDescription.height;
  codecSetup->bufferRectl = codecSetup->rectl;
  codecSetup->bufferRectl.yTop = (imageDescription.height + 3) >> 2 << 2;
  codecSetup->decompress.ulStructLen = sizeof(MMDECOMPRESS);
  codecSetup->decompress.ulFlags = MMIO_ORIGIN_LOWERLEFT;
  codecSetup->decompress.ulSrcBufLen = 0; // Set for each decompression
  codecSetup->decompress.pSrcBuf = codecSetup->compressedFrame; // Reset for each decompression
  codecSetup->decompress.ulDstBufLen = 0;
  codecSetup->decompress.pDstBuf = 0; // Set for each decompression
  codecSetup->decompress.pRunTimeInfo = &codecSetup->videoDecompress;
  codecSetup->videoDecompress.ulStructLen = sizeof(MMVIDEODECOMPRESS);
  codecSetup->videoDecompress.ulRectlCount = 1;
  codecSetup->videoDecompress.prectl = &codecSetup->rectl;
  codecSetup->videoDecompress.ulSkipLength = imageDescription.width;
  codecSetup->videoDecompress.ulDecodeLines = imageDescription.height;
  codecSetup->videoDecompress.genpalPhysical = palette_p;
  codecSetup->videoDecompress.genpalVideo = palette_p;
  codecSetup->videoDecompress.rectlSrc = codecSetup->rectl;
  codecSetup->videoDecompress.rectlDst = codecSetup->rectl;
  codecSetup->videoDecompress.ulDeltaCount = 0;
  codecSetup->videoDecompress.ulParm1 = codecSetup->colorFormat;
  codecSetup->videoDecompress.ulParm2 = 0;
  codecSetup->videoDecompress.ulParm3 = 0;
  codecSetup->videoDecompress.ulParm4 = 0;

  if (codecSetup->diveInstance_p)
  {
    //Allocate a buffer for the codec's output
    errorCode = DiveAllocImageBuffer(codecSetup->diveInstance_p, &codecSetup->decompressedFrame, codecSetup->colorFormat,
                                     imageDescription.width, codecSetup->bufferRectl.yTop, 0, 0);
    if (errorCode) throw(Unsupported("Image buffer could not be allocated."));

    if (codecSetup->colorFormat != FOURCC_BGR3)
    {
      codecSetup->setupBlitterForLoading_p.ulStructLen = sizeof(SETUP_BLITTER);
      codecSetup->setupBlitterForLoading_p.fInvert = TRUE;
      codecSetup->setupBlitterForLoading_p.fccSrcColorFormat = codecSetup->colorFormat;
      codecSetup->setupBlitterForLoading_p.ulSrcWidth = imageDescription.width;
      codecSetup->setupBlitterForLoading_p.ulSrcHeight = codecSetup->bufferRectl.yTop;
      codecSetup->setupBlitterForLoading_p.ulSrcPosX = 0;
      codecSetup->setupBlitterForLoading_p.ulSrcPosY = 0;
      codecSetup->setupBlitterForLoading_p.ulDitherType = 0;
      codecSetup->setupBlitterForLoading_p.fccDstColorFormat = FOURCC_BGR3;
      codecSetup->setupBlitterForLoading_p.ulDstWidth = imageDescription.width;
      codecSetup->setupBlitterForLoading_p.ulDstHeight = codecSetup->bufferRectl.yTop;
      codecSetup->setupBlitterForLoading_p.lDstPosX = 0;
      codecSetup->setupBlitterForLoading_p.lDstPosY = 0;
      codecSetup->setupBlitterForLoading_p.lScreenPosX = 0;
      codecSetup->setupBlitterForLoading_p.lScreenPosY = 0;
      codecSetup->setupBlitterForLoading_p.ulNumDstRects = 1;
      codecSetup->setupBlitterForLoading_p.pVisDstRects = &codecSetup->bufferRectl;

      errorCode = DiveSetupBlitter(codecSetup->diveInstance_p, &codecSetup->setupBlitterForLoading_p);
      if (errorCode) throw(Unsupported("Blitter could not be set up."));

      //Allocate a buffer for output when converted to our standard BGR3 format
      errorCode = DiveAllocImageBuffer(codecSetup->diveInstance_p, &codecSetup->convertedFrame, FOURCC_BGR3,
                                       imageDescription.width, codecSetup->bufferRectl.yTop, 0, 0);
      if (errorCode) throw(Unsupported("Image buffer could not be allocated."));
    }
  }
  else
    codecSetup->bufferRectl.xRight = (imageDescription.width + 3) >> 2 << 2;
}

/************************************************************************/
/* SET WINDOW                                                           */
/************************************************************************/
void QFWorldRenderer::setWindow(IWindow& window)
{
  window_p = &window;
}

/************************************************************************/
/* VISIBLE REGION ENABLED                                               */
/************************************************************************/
void QFWorldRenderer::visibleRegionEnabled(Boolean regionChanged)
{
  HPS hps;
  HRGN hrgn;
  IPoint absoluteWindowPosition;
  RECTL rcls[50];
  RGNRECT rgnCtl;
  LONG errorCode;

  hps = WinGetPS(window_p->handle());
  if (hps)
  {
    hrgn = GpiCreateRegion(hps, 0L, NULL);
    if (hrgn)
    {
      WinQueryVisibleRegion(window_p->handle(), hrgn);
      rgnCtl.ircStart     = 0;
      rgnCtl.crc          = 50;
      rgnCtl.ulDirection  = 1;

      if (GpiQueryRegionRects(hps, hrgn, NULL, &rgnCtl, rcls))
      {
        absoluteWindowPosition = IWindow::mapPoint(window_p->position(), window_p->parent()->handle(), HWND_DESKTOP);

        /* Tell DIVE about the new settings */
        setupBlitter_p.ulSrcWidth = movieSize().width();
        setupBlitter_p.ulSrcHeight = movieSize().height();
        setupBlitter_p.ulDstWidth = window_p->size().width();
        setupBlitter_p.ulDstHeight = window_p->size().height();
        setupBlitter_p.lScreenPosX = absoluteWindowPosition.x();
        setupBlitter_p.lScreenPosY = absoluteWindowPosition.y();
        setupBlitter_p.ulNumDstRects = rgnCtl.crcReturned;
        setupBlitter_p.pVisDstRects = rcls;

        errorCode = DiveSetupBlitter(dive, &setupBlitter_p);
      }
      GpiDestroyRegion(hps, hrgn);
    }
    WinReleasePS(hps);
  }
  if (regionChanged)
    refresh();
}

/************************************************************************/
/* VISIBLE REGION DISABLED                                              */
/************************************************************************/
void QFWorldRenderer::visibleRegionDisabled()
{
  LONG errorCode;

  errorCode = DiveSetupBlitter(dive, 0);
}

/************************************************************************/
/* SET VIEW                                                             */
/************************************************************************/
Boolean QFWorldRenderer::setView(float newPan, float newTilt, float newFov)
{
  Boolean result = false;

  // Calculate new fov value
  if (newFov > maxFov)
    fov = maxFov;
  else
    if (newFov < minFov)
      fov = minFov;
    else
      fov = newFov;

  float halfFov = fov / 2.0;

  // Calculate new pan value
  if (minPan == .0 && maxPan == 360.0)
  {
    for (;newPan >= 360.0; newPan -= 360.0);
    for (;newPan < .0; newPan += 360.0);
    pan = newPan;
  }
  else
  {
    float panHalfFov = halfFov * panFovAdjustment;
    if (newPan + panHalfFov  > maxPan)
    {
      pan = maxPan - panHalfFov;
      result = true;
    }
    else
      if (newPan - panHalfFov < minPan)
      {
        pan = minPan + panHalfFov;
        result = true;
      }
      else
        pan = newPan;
   }

  // Calculate new tilt value
  if (newTilt + halfFov > maxTilt)
    tilt = maxTilt - halfFov;
  else
    if (newTilt - halfFov < minTilt)
      tilt = minTilt + halfFov;
    else
      tilt = newTilt;

  return result;
}

/************************************************************************/
/* UPDATE ENABLED                                                       */
/************************************************************************/
Boolean QFWorldRenderer::updateEnabled()
{
  return updateEnabled_p;
}

/************************************************************************/
/* CURRENT PAN                                                          */
/************************************************************************/
float QFWorldRenderer::currentPan()
{
  return pan;
}

/************************************************************************/
/* CURRENT TILT                                                         */
/************************************************************************/
float QFWorldRenderer::currentTilt()
{
  return tilt;
}

/************************************************************************/
/* CURRENT FOV                                                          */
/************************************************************************/
float QFWorldRenderer::currentFov()
{
  return fov;
}

/************************************************************************/
/* PAN STEP                                                             */
/************************************************************************/
float QFWorldRenderer::panStep(InputDevice, float stepFov)
{
  return 6.0;
}

/************************************************************************/
/* TILT STEP                                                            */
/************************************************************************/
float QFWorldRenderer::tiltStep(InputDevice, float stepFov)
{
  return 6.0;
}

/************************************************************************/
/* FOV STEP                                                             */
/************************************************************************/
float QFWorldRenderer::fovStep(InputDevice)
{
  return 1.0;
}

/************************************************************************/
/* HOT SPOT AT                                                          */
/************************************************************************/
long QFWorldRenderer::hotSpotAt(IPoint point)
{
  return 0;
}

/************************************************************************/
/* HOT SPOT CLICKED                                                     */
/************************************************************************/
void QFWorldRenderer::hotSpotClicked(long hotSpotID)
{
}

/************************************************************************/
/* DESTRUCTOR                                                           */
/************************************************************************/
QFWorldRenderer::~QFWorldRenderer()
{
  delete videoSetup;

  if (diveForLoading)
    DiveClose(diveForLoading);

  if (dive)
    DiveClose(dive);

  mmioClose(movieFile_p, 0);
}

