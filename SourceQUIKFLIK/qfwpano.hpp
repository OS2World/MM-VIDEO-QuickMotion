#ifndef QFWPANO_HPP
#define QFWPANO_HPP

#include "qfwrend.hpp"
#include "qfwpanof.hpp"

#include <ikeyhdr.hpp>

class PanoramaDescription;
class PanoramaMediaInfoAtom;
class PanoramaSample;

class QFPanoramaRenderer : public QFWorldRenderer,
                           public IKeyboardHandler
{
public:
  QFPanoramaRenderer(QFWorldPlayer& qfWorldPlayer, MovieAtom& movieAtom, HMMIO movieFile);

  virtual ISize movieSize();
  virtual void start();
  virtual void load();
  virtual void update();
  virtual void refresh();
  virtual void captureFrame(IString saveAsFileName, FOURCC ioProc);
  virtual void copyFrame();
  virtual long hotSpotAt(IPoint point);
  virtual void hotSpotClicked(long hotSpotID);
  virtual float panStep(QFWorldRenderer::InputDevice inputDevice, float stepFov = .0);
  virtual float tiltStep(QFWorldRenderer::InputDevice inputDevice, float stepFov = .0);
  virtual float fovStep(InputDevice inputDevice);

  virtual ~QFPanoramaRenderer();

protected:
  virtual Boolean characterKeyPress(IKeyboardEvent &event);

private:
  static Boolean trigTablesLoaded;
  static fixed secant[];

  void loadPlane(PBYTE planeBuffer, ULONG planeSize, ULONG planeLineBytes,
                 SampleTableAtom* trackMap, CodecSetup* codecSetup,
                 ULONG numFramesX, ULONG numFramesY);
  void loadDie(ULONG length, CodecSetup *codecSetup, ULONG planeLineBytes);

  ULONG panLineBytes, hotSpotLineBytes;
  RendererSetup* setup;
  CodecSetup* hotSpotSetup;

  PanoramaMediaInfoAtom* panoramaMediaInfo;
  PanoramaDescription* panoramaDescription;
  PanoramaSample* panoramaSample;
  SampleTableAtom *hotSpotMap, *panoramaMap;
  ULONG panoramaTimeScale, videoTimeScale, hotSpotTimeScale;

  PBYTE panorama;
  PBYTE hotSpots;
  ULONG frame;

  //For loading
  ULONG filePosition;
  PBYTE hotSpotLoadBuffer;
  PBYTE loadSpot;
  long  nodeID;
  float initPan, initTilt, initFov;

  float panScale;

  unsigned long *lastRender;
  float lastRenderedTilt, lastRenderedFov;

  long previousHotSpotID;
};

#endif

