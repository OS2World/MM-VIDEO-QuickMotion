#ifndef QFPLAYER_HPP
#define QFPLAYER_HPP

#include "quikflik.h"

#include <iwindow.hpp>
#include <istring.hpp>

class QuickFlick;
class IObjectWindow;

class QFPlayer
{
public:
  QFPlayer(QuickFlick& quickFlick);
  virtual void                     setWindow(IWindow& window) = 0;
  virtual ISize                    movieSize() = 0;
  virtual void             _Export start() = 0;
  virtual void             _Export finishStream() = 0;
  virtual void             _Export handleScrollingFor(IWindow& viewPort) = 0;
  virtual void             _Export stopHandlingScrollingFor(IWindow& viewPort) = 0;

  virtual                          ~QFPlayer();

  HEV proceedWithPrepare;
  BOOL *pWaitForData;
  unsigned long howLongMovieWillPlay;
  Boolean filePrepared, autoStartPointReached, autoStartPerformed;

  QuickFlick& qf;
  IObjectWindow *serviceThreadWindow;
};

#endif

