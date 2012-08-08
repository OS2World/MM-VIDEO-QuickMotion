#include "quikflik.h"

#include <istattxt.hpp>
#include <islider.hpp>
#include <ipopmenu.hpp>

#include "qfmphlr.hpp"
#include "qfmotion.hpp"
#include "quikflik.hpp"


/************************************************************************/
/* Constructor                                                          */
/************************************************************************/
QFMotionPlayerHandler::QFMotionPlayerHandler(QFMotionPlayer &qfMotionPlayer)
                 : qfmp(qfMotionPlayer)
{
}

/************************************************************************/
/* SCROLL                                                               */
/************************************************************************/
Boolean QFMotionPlayerHandler::scroll(IEvent &event)
{
  if (qfmp.movieWindow)
  {
    qfmp.movieWindow->sendEvent(WM_VRNDISABLED);
    event.dispatchingWindow()->dispatchRemainingHandlers(event);
    qfmp.movieWindow->sendEvent(WM_VRNENABLED, true);
    return true;
  }
  else
   return false;
}

/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                     CUSTOM HANDLER FUNCTIONS                         */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/* GOT HEADER                                                           */
/************************************************************************/
Boolean QFMotionPlayerHandler::gotHeader(IEvent &event)
{
  qfmp.gotHeader();
  return true;
}

/************************************************************************/
/* OPEN                                                                 */
/************************************************************************/
Boolean QFMotionPlayerHandler::open(IEvent &event)
{
  qfmp.open();
  return true;
}

/************************************************************************/
/* LOAD                                                                 */
/************************************************************************/
Boolean QFMotionPlayerHandler::load(IEvent &event)
{
  qfmp.load();
  return true;
}

/************************************************************************/
/* CUE                                                                  */
/************************************************************************/
Boolean QFMotionPlayerHandler::cue(IEvent &event)
{
  qfmp.cue();
  return true;
}

/************************************************************************/
/* PLAY                                                                 */
/************************************************************************/
Boolean QFMotionPlayerHandler::play(IEvent &event)
{
  qfmp.play();
  return true;
}

/************************************************************************/
/* PAUSE                                                                */
/************************************************************************/
Boolean QFMotionPlayerHandler::pause(IEvent &event)
{
   qfmp.pause();
   return true;
}

/************************************************************************/
/* SEEK                                                                 */
/************************************************************************/
Boolean QFMotionPlayerHandler::seek(IEvent &event)
{
  qfmp.seek(event.parameter1().asUnsignedLong());
  return true;
}

/************************************************************************/
/* EDIT                                                                 */
/************************************************************************/
Boolean QFMotionPlayerHandler::edit(IEvent &event)
{
  float percent = event.parameter1().asUnsignedLong();
  unsigned long parm2 = event.parameter2().asUnsignedLong();
  percent = (parm2 ? percent / parm2 : .0);
  unsigned long seekTime = percent * qfmp.howLongMovieWillPlay;

  qfmp.serviceThreadWindow->postEvent(WM_CUSTOM_SEEK, seekTime);

  return true;
}

/************************************************************************/
/* DESTROY                                                              */
/************************************************************************/
Boolean QFMotionPlayerHandler::destroy(IEvent &event)
{
  return qfmp.destroy(event);
}

/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                        HANDLER FUNCTIONS                             */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/* PAINT                                                                */
/************************************************************************/
Boolean QFMotionPlayerHandler::paintWindow(IPaintEvent &event)
{
  if (qfmp.movieWindow)
  {
    qfmp.movieWindow->postEvent(WM_VRNDISABLED);
    qfmp.movieWindow->postEvent(WM_VRNENABLED);
  }
  return true; // Do not let the plugin window paint.
}

/************************************************************************/
/* MAKE POPUP MENU                                                      */
/************************************************************************/
Boolean QFMotionPlayerHandler::makePopUpMenu(IMenuEvent &event)
{
  if (event.controlWindow() == qfmp.movieWindow)
  {
    IPoint mousePosition = IWindow::mapPoint(event.mousePosition(), event.controlWindow()->handle(), qfmp.window_p->handle());
    qfmp.menu->show(mousePosition);
    return true;
  }
  else if (event.controlWindow() == qfmp.screen)
  {
    IWindowHandle parent = WinQueryWindow(event.controlWindow()->handle(), QW_PARENT);
    parent.sendEvent(event.eventId(), event.parameter1(), event.parameter2());
    return true;
  }

  return false;
}

/************************************************************************/
/* RESIZE                                                               */
/************************************************************************/
Boolean QFMotionPlayerHandler::windowResize(IResizeEvent &event)
{
  return qfmp.windowResize(event);
}

/************************************************************************/
/* COMMAND                                                              */
/************************************************************************/
Boolean QFMotionPlayerHandler::command(ICommandEvent &event)
{
  return qfmp.command(event);
}

/************************************************************************/
/* MOUSE CLICKED                                                        */
/************************************************************************/
Boolean QFMotionPlayerHandler::mouseClicked(IMouseClickEvent &event)
{
  if (event.mouseAction() == IMouseClickEvent::click &&
      event.mouseButton() == IMouseClickEvent::button1)
  {
    qfmp.handlePlayCommand();
  }

  return true;
}

/************************************************************************/
/* scroll functions                                                     */
/************************************************************************/
Boolean QFMotionPlayerHandler::lineUp(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFMotionPlayerHandler::lineDown(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFMotionPlayerHandler::lineLeft(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFMotionPlayerHandler::lineRight(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFMotionPlayerHandler::pageUp(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFMotionPlayerHandler::pageDown(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFMotionPlayerHandler::pageLeft(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFMotionPlayerHandler::pageRight(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFMotionPlayerHandler::scrollBoxTrack(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFMotionPlayerHandler::virtualKeyPress(IKeyboardEvent &event)
{
  switch (event.virtualKey())
  {
    case IKeyboardEvent::left:
    case IKeyboardEvent::right:
    case IKeyboardEvent::up:
    case IKeyboardEvent::down:
    case IKeyboardEvent::pageUp:
    case IKeyboardEvent::pageDown:
      return scroll(event);
  }
  return false;
}


