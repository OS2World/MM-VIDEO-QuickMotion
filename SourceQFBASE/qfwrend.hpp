#ifndef QFWREND_HPP
#define QFWREND_HPP

#include "quikflik.h"

#include <dive.h>
#include <fourcc.h>
#include <ivbase.hpp>
#include <ipoint.hpp>
#include <istring.hpp>
#include <iexcept.hpp>

class QFWorldPlayer;
class MovieAtom;
class TrackAtom;
class SampleTableAtom;
class IWindow;
class IThread;

struct Unsupported : public IException
{
  Unsupported(const char *errorText) : IException(IString("QuickFlick Error: ") + errorText) {};
};

class CodecSetup
{
public:
  CodecSetup(HDIVE diveInstance);
  ~CodecSetup();

  HDIVE             diveInstance_p;
  ULONG             nonDiveScanLineBytes;
  ULONG             codecHandle;
  PCODECPROC        sendCodecMessage;
  MMDECOMPRESS      decompress;
  MMVIDEODECOMPRESS videoDecompress;
  RECTL             rectl, bufferRectl;
  ULONG             bufferHeight;
  PCHAR             compressedFrame;
  ULONG             decompressedFrame;
  ULONG             convertedFrame;
  SETUP_BLITTER     setupBlitterForLoading_p;
  FOURCC            colorFormat;
};

class QFWorldRenderer : public IVBase
{
public:
  enum InputDevice {mouse, keyboard, timer};

  QFWorldRenderer(QFWorldPlayer& qfWorldPlayer, MovieAtom& movieAtom, HMMIO movieFile);

  virtual ISize    movieSize() = 0;
  virtual void     setWindow(IWindow& window);
  virtual void     start() = 0;
  virtual void     load() = 0;
  virtual void     update() = 0;
  virtual void     refresh() = 0;
  virtual void     captureFrame(IString saveAsFileName, FOURCC ioProc) = 0;
  virtual void     copyFrame() = 0;
  virtual long     hotSpotAt(IPoint point);
  virtual void     hotSpotClicked(long hotSpotID);
  virtual Boolean  setView(float newPan, float newTilt, float newFov);
  virtual Boolean  updateEnabled();
  virtual float    currentPan();
  virtual float    currentTilt();
  virtual float    currentFov();
  virtual float    panStep(InputDevice inputDevice, float stepFov = .0);
  virtual float    tiltStep(InputDevice inputDevice, float stepFov = .0);
  virtual float    fovStep(InputDevice inputDevice);

  virtual void visibleRegionEnabled(Boolean regionChanged);
  virtual void visibleRegionDisabled();

  virtual ~QFWorldRenderer();

protected:
  void setCodec(SampleTableAtom* trackMap, CodecSetup* codecSetup);

  QFWorldPlayer&  qfwp;
  MovieAtom&      movieAtom_p;
  HMMIO           movieFile_p;
  IWindow*        window_p;

  TrackAtom*       panoramaTrack;
  SampleTableAtom* videoMap;

  HDIVE             diveForLoading, dive;
  CodecSetup*       videoSetup;

  float             pan, tilt, fov;

  float             minPan, maxPan;
  float             minTilt, maxTilt;
  float             minFov, maxFov;
  float             panFovAdjustment;

  float             defaultPan_p, defaultTilt_p, defaultFov_p;
  float             minPanReg, maxPanReg, maxFovReg;

  IThread           *loadThread;
  Boolean           updateEnabled_p, loading;

private:
  Boolean           usingPaletteManager_p;
  RGB2              paletteEntries[MAX_PALETTE];
  GENPAL            palette_p;
  HPAL              paletteHandle_p;

  CODECINIFILEINFO  codecIniFileInfo;
  CODECOPEN         codecOpen;
  CODECVIDEOHEADER  sourceHeader;
  CODECVIDEOHEADER  destinationHeader;
  HMODULE           codecModule;

  SETUP_BLITTER     setupBlitter_p;

  friend void EXPENTRY QUIKFLIK(QFWorldRenderer *worldRenderer);
};

#endif

