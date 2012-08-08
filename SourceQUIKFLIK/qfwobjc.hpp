#ifndef QFWOBJC_HPP
#define QFWOBJC_HPP

#include "qfwrend.hpp"

class QFObjectRenderer : public QFWorldRenderer
{
public:
  QFObjectRenderer(QFWorldPlayer& qfWorldPlayer, MovieAtom& movieAtom, HMMIO movieFile);

  virtual ISize movieSize();
  virtual void start();
  virtual void load();
  virtual void update();
  virtual void refresh();
  virtual void captureFrame(IString saveAsFileName, FOURCC ioProc);
  virtual void copyFrame();
  virtual float panStep(QFWorldRenderer::InputDevice inputDevice, float stepFov = .0);
  virtual float tiltStep(QFWorldRenderer::InputDevice inputDevice, float stepFov = .0);

  virtual ~QFObjectRenderer();

private:
  void prepareFrame(ULONG column, ULONG row, ULONG length);

  ULONG columns, rows;
  float degreesPerColumn, degreesPerRow;
  LONG currentColumn, currentRow;
  ULONG filePosition;

  // Prepared frame array in screen color space
  PULONG *frame;
  BYTE*** buffer;
};

#endif

