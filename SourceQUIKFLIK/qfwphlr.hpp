#ifndef QFWPHLR_HPP
#define QFWPHLR_HPP

#include "qfphlr.hpp"

#include <iapp.hpp>
#include <iscrlhdr.hpp>
#include <ikeyhdr.hpp>
#include <icmdhdr.hpp>
#include <imoushdr.hpp>
#include <imenuhdr.hpp>
#include <isizehdr.hpp>
#include <ipainhdr.hpp>
#include <itimer.hpp>

class QFWorldPlayer;

class QFWorldPlayerHandler : public QFPlayerHandler,
                             public IScrollHandler,
                             public IKeyboardHandler,
                             public ICommandHandler,
                             public IMouseHandler,
                             public IMenuHandler,
                             public IResizeHandler,
                             public IPaintHandler
{
public:
  enum MouseAction {none, moving, zooming, settingSpin};

  // Constructor
  QFWorldPlayerHandler(QFWorldPlayer &qfWorldPlayer);

  // Helper
  Boolean scroll(IEvent &event);
  void resetActionStart(const IPoint& startPoint);
  void spinTimerExpired();

  // Custom
  virtual Boolean gotHeader(IEvent &event);
  virtual Boolean visibleRegionEnabled(IEvent &event);
  virtual Boolean visibleRegionDisabled(IEvent &event);
  virtual Boolean destroy(IEvent &event);

  // Menu
  virtual Boolean makePopUpMenu(IMenuEvent &event);

  // Resize
  virtual Boolean windowResize(IResizeEvent &event);

  // Paint
  virtual Boolean paintWindow(IPaintEvent &event);

  // Commands
  virtual Boolean command(ICommandEvent &event);

  // Mouse
  virtual Boolean mouseClicked(IMouseClickEvent &event);
  virtual Boolean mouseMoved(IMouseEvent &event);
  virtual Boolean mousePointerChange(IMousePointerEvent &event);

  // Scrolling
  virtual Boolean lineUp(IScrollEvent &event);
  virtual Boolean lineDown(IScrollEvent &event);
  virtual Boolean lineLeft(IScrollEvent &event);
  virtual Boolean lineRight(IScrollEvent &event);
  virtual Boolean pageUp(IScrollEvent &event);
  virtual Boolean pageDown(IScrollEvent &event);
  virtual Boolean pageLeft(IScrollEvent &event);
  virtual Boolean pageRight(IScrollEvent &event);
  virtual Boolean scrollBoxTrack(IScrollEvent &event);
  virtual Boolean characterKeyPress(IKeyboardEvent &event);
  virtual Boolean virtualKeyPress(IKeyboardEvent &event);
  virtual Boolean key(IKeyboardEvent &event);

  ITimer spinTimer;

private:
  QFWorldPlayer &qfwp;

  Boolean spinning;
  LONG spinBase, spinDelta;
  float spinInterval;
  ULONG lastSpinExpireTime;

  IApplication::PriorityClass restoreClass;
  unsigned restoreLevel;

  MouseAction currentAction, pendingAction;
  IPoint actionStartPoint;

  float startPan, startTilt, startFov;

};

#endif

