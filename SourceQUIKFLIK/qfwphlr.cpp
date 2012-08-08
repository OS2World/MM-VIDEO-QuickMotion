#include "quikflik.h"

#include <istattxt.hpp>
#include <ipopmenu.hpp>
#include <ithread.hpp>
#include <math.h>

#include "qfwphlr.hpp"
#include "qfworld.hpp"
#include "qfwrend.hpp"
#include "quikflik.hpp"

#ifdef TRACE
#include <fstream.h>
extern ofstream *debugOutput;
extern void stampTime();
#endif

/************************************************************************/
/* Constructor                                                          */
/************************************************************************/
QFWorldPlayerHandler::QFWorldPlayerHandler(QFWorldPlayer &qfWorldPlayer)
                 : qfwp(qfWorldPlayer)
{
  currentAction = pendingAction = none;
  spinning = false;
  spinBase = 0;
}

/************************************************************************/
/* SCROLL                                                               */
/************************************************************************/
Boolean QFWorldPlayerHandler::scroll(IEvent &event)
{
  if (qfwp.movieWindow)
  {
    qfwp.movieWindow->sendEvent(WM_VRNDISABLED);
    event.dispatchingWindow()->dispatchRemainingHandlers(event);
    qfwp.movieWindow->sendEvent(WM_VRNENABLED, true);
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
Boolean QFWorldPlayerHandler::gotHeader(IEvent &event)
{
  qfwp.gotHeader();
  return true;
}

/************************************************************************/
/* VISIBLE REGION ENABLED                                               */
/************************************************************************/
Boolean QFWorldPlayerHandler::visibleRegionEnabled(IEvent &event)
{
  if (qfwp.worldRenderer)
    qfwp.worldRenderer->visibleRegionEnabled(true);
  return true;
}

/************************************************************************/
/* VISIBLE REGION DISABLED                                              */
/************************************************************************/
Boolean QFWorldPlayerHandler::visibleRegionDisabled(IEvent &event)
{
  if (qfwp.worldRenderer)
    qfwp.worldRenderer->visibleRegionDisabled();
  return true;
}

/************************************************************************/
/* DESTROY                                                              */
/************************************************************************/
Boolean QFWorldPlayerHandler::destroy(IEvent &event)
{
  return qfwp.destroy(event);
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
/* MAKE POPUP MENU                                                      */
/************************************************************************/
Boolean QFWorldPlayerHandler::makePopUpMenu(IMenuEvent &event)
{
  if (event.controlWindow() == qfwp.movieWindow)
  {
    IPoint mousePosition = IWindow::mapPoint(event.mousePosition(), event.controlWindow()->handle(), qfwp.window_p->handle());
    qfwp.menu->show(mousePosition);
    return true;
  }
  else if (event.controlWindow() == qfwp.screen)
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
Boolean QFWorldPlayerHandler::windowResize(IResizeEvent &event)
{
  return qfwp.windowResize(event);
}

/************************************************************************/
/* PAINT                                                                */
/************************************************************************/
Boolean QFWorldPlayerHandler::paintWindow(IPaintEvent &event)
{
  if (qfwp.worldRenderer)
    qfwp.worldRenderer->refresh();
  return true; // Do not let the plugin window paint.
}

/************************************************************************/
/* COMMAND                                                              */
/************************************************************************/
Boolean QFWorldPlayerHandler::command(ICommandEvent &event)
{
  return qfwp.command(event);
}

/************************************************************************/
/* RESET ACTION START                                                   */
/************************************************************************/
void QFWorldPlayerHandler::resetActionStart(const IPoint& startPoint)
{
  actionStartPoint = startPoint;
  startPan = qfwp.worldRenderer->currentPan();
  startTilt = qfwp.worldRenderer->currentTilt();
  startFov = qfwp.worldRenderer->currentFov();
}


/************************************************************************/
/* MOUSE CLICKED                                                        */
/************************************************************************/
Boolean QFWorldPlayerHandler::mouseClicked(IMouseClickEvent &event)
{
  if (!qfwp.worldRenderer->updateEnabled())
  {
    if (event.mouseAction() == IMouseClickEvent::down)
      qfwp.movieWindow->setFocus();
    return false;
  }

  if (event.mouseButton() == IMouseClickEvent::button1)
  {
    if (event.mouseAction() == IMouseClickEvent::down)
    {
      if (currentAction == none)
      {
        qfwp.movieWindow->setFocus();
        qfwp.movieWindow->capturePointer();
      }

      pendingAction = currentAction;

      if (!event.isShiftKeyDown() || spinning)
      {
        currentAction = settingSpin;
        if (event.isShiftKeyDown())
          WinSetPointer(IWindow::desktopWindow()->handle(), QFWorldPlayer::spinArrows);
        else
          WinSetPointer(IWindow::desktopWindow()->handle(), QFWorldPlayer::spinHand);
      }
      else
      {
        currentAction = moving;
        WinSetPointer(IWindow::desktopWindow()->handle(), QFWorldPlayer::pullHand);
      }

      resetActionStart(event.mousePosition());
      return true;
    }
    else /* not mousedown */
    {
      if (event.mouseAction() == IMouseClickEvent::up)
      {
        if (spinning)
        {
          if (event.isShiftKeyDown())
            spinBase = spinDelta;
          else
          {
            spinTimer.stop();
            spinBase = 0;
            spinning = false;
            IThread::current().setPriority(restoreClass, restoreLevel);
          }
        }
        if (currentAction == moving || currentAction == settingSpin)
        {
          currentAction = pendingAction;
          resetActionStart(event.mousePosition());
        }
        if (currentAction == none)
          qfwp.movieWindow->releasePointer();
        pendingAction = none;
        return true;
      }
      else
      {
        if (event.mouseAction() == IMouseClickEvent::click)
        {
          if (currentAction == none)
          {
            long hotSpot = qfwp.worldRenderer->hotSpotAt(event.mousePosition());
            if (hotSpot)
            {
              qfwp.worldRenderer->hotSpotClicked(hotSpot);
              return true;
            }
          }
        }
      }
    }
  }
  else /* not button 1 */
  {
    if (event.mouseButton() == IMouseClickEvent::button2)
    {
      if (event.mouseAction() == IMouseClickEvent::down)
      {
        if (currentAction == none)
        {
          qfwp.movieWindow->setFocus();
          qfwp.movieWindow->capturePointer();
        }
        pendingAction = currentAction;
        currentAction = zooming;
        WinSetPointer(IWindow::desktopWindow()->handle(), QFWorldPlayer::zoom);
        resetActionStart(event.mousePosition());
        return true;
      }
      else /* not mousedown */
      {
        if (event.mouseAction() == IMouseClickEvent::up)
        {
          if (currentAction == zooming)
          {
            currentAction = pendingAction;
            resetActionStart(event.mousePosition());
          }
          if (currentAction == none)
            qfwp.movieWindow->releasePointer();
          pendingAction = none;
          return true;
        }
      }
    }
  }

  return false;
}

/************************************************************************/
/* MOUSE MOVED                                                          */
/************************************************************************/
Boolean QFWorldPlayerHandler::mouseMoved(IMouseEvent &event)
{
  if (currentAction == none)
    return false;

  IPoint delta = event.mousePosition() - actionStartPoint;

  if (currentAction == moving)
  {
    float newPan = startPan + qfwp.worldRenderer->panStep(QFWorldRenderer::mouse, startFov) * delta.x();
    float newTilt = startTilt + qfwp.worldRenderer->tiltStep(QFWorldRenderer::mouse, startFov) * delta.y();
    qfwp.worldRenderer->setView(newPan, newTilt, qfwp.worldRenderer->currentFov());
    spinTimerExpired();
  } else
    if (currentAction == zooming)
    {
      float newFov = startFov / pow(qfwp.worldRenderer->fovStep(QFWorldRenderer::mouse), delta.y());
      qfwp.worldRenderer->setView(qfwp.worldRenderer->currentPan(), qfwp.worldRenderer->currentTilt(), newFov);
      spinTimerExpired();
    } else
      if (currentAction == settingSpin)
      {
        float newTilt = startTilt + qfwp.worldRenderer->tiltStep(QFWorldRenderer::mouse, startFov) * delta.y();
        qfwp.worldRenderer->setView(qfwp.worldRenderer->currentPan(), newTilt, qfwp.worldRenderer->currentFov());
        if (!spinning)
        {
          restoreClass = IThread::current().priorityClass();
          restoreLevel = IThread::current().priorityLevel();
          IThread::current().setPriority(IApplication::idleTime, 17);
          spinTimer.start(new ITimerMemberFn0<QFWorldPlayerHandler>(*this, QFWorldPlayerHandler::spinTimerExpired));
          lastSpinExpireTime = WinGetCurrentTime(IThread::current().anchorBlock());
          spinInterval = 3600000.0;
          spinning = true;
        }
        spinDelta = spinBase + delta.x();
        if (spinDelta)
        {
          float panStep = qfwp.worldRenderer->panStep(QFWorldRenderer::timer);
          spinInterval = (panStep >= 0 ? 2000.0 / spinDelta : -2000.0 / spinDelta); //Minimum rate is 2 seconds / scene pel
          spinTimer.setInterval(fabs(spinInterval / panStep));
        }
        spinTimerExpired();
      }

  return true;
}

/************************************************************************/
/* SPIN TIMER EXPIRED                                                   */
/************************************************************************/
void QFWorldPlayerHandler::spinTimerExpired()
{
  if (spinning)
  {
    ULONG currentTime = WinGetCurrentTime(IThread::current().anchorBlock());
    float newPan = qfwp.worldRenderer->currentPan() + (currentTime - lastSpinExpireTime) / spinInterval;
    lastSpinExpireTime = currentTime;
    if (qfwp.worldRenderer->setView(newPan, qfwp.worldRenderer->currentTilt(), qfwp.worldRenderer->currentFov()))
    {
      spinTimer.stop();
      spinBase = 0;
      spinning = false;
      IThread::current().setPriority(restoreClass, restoreLevel);
    }
  }
  qfwp.worldRenderer->update();
}

/************************************************************************/
/* MOUSE POINTER CHANGE                                                 */
/************************************************************************/
Boolean QFWorldPlayerHandler::mousePointerChange(IMousePointerEvent &event)
{
  if (!qfwp.worldRenderer->updateEnabled())
  {
    event.setMousePointer(ISystemPointerHandle(ISystemPointerHandle::wait));
    return true;
  }

  POINTL ptlPoint;

  Boolean shiftKeyDown = WinGetKeyState(HWND_DESKTOP, VK_SHIFT) & 0x8000;
  WinQueryPointerPos(HWND_DESKTOP, &ptlPoint);
  WinMapWindowPoints(HWND_DESKTOP, qfwp.movieWindow->handle(), &ptlPoint, 1);

  if (currentAction == none)
    if (qfwp.worldRenderer->hotSpotAt(ptlPoint))
      event.setMousePointer(QFWorldPlayer::hotHand);
    else
      if (shiftKeyDown)
        if (spinning)
          event.setMousePointer(QFWorldPlayer::spinArrows);
        else
          event.setMousePointer(QFWorldPlayer::openHand);
      else
        event.setMousePointer(QFWorldPlayer::spinHand);
  else
    if (currentAction == settingSpin)
      if (shiftKeyDown)
        event.setMousePointer(QFWorldPlayer::spinArrows);
      else
        event.setMousePointer(QFWorldPlayer::spinHand);
    else
      if (currentAction == zooming)
        event.setMousePointer(QFWorldPlayer::zoom);
      else
        event.setMousePointer(QFWorldPlayer::pullHand);

  return true;
}


/************************************************************************/
/* scroll functions                                                     */
/************************************************************************/
Boolean QFWorldPlayerHandler::lineUp(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFWorldPlayerHandler::lineDown(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFWorldPlayerHandler::lineLeft(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFWorldPlayerHandler::lineRight(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFWorldPlayerHandler::pageUp(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFWorldPlayerHandler::pageDown(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFWorldPlayerHandler::pageLeft(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFWorldPlayerHandler::pageRight(IScrollEvent &event)
{
  return scroll(event);
}
Boolean QFWorldPlayerHandler::scrollBoxTrack(IScrollEvent &event)
{
  return scroll(event);
}

Boolean QFWorldPlayerHandler::characterKeyPress(IKeyboardEvent &event)
{
  if (event.dispatchingWindow() == qfwp.movieWindow)
  {
    switch (event.character())
    {
      case 'l':
      case 'L':
        qfwp.worldRenderer->setView(qfwp.worldRenderer->currentPan() - 90.0,
                                    qfwp.worldRenderer->currentTilt(),
                                    qfwp.worldRenderer->currentFov());
        spinTimerExpired();
        return true;

      case 'r':
      case'R':
        qfwp.worldRenderer->setView(qfwp.worldRenderer->currentPan() + 90.0,
                                    qfwp.worldRenderer->currentTilt(),
                                    qfwp.worldRenderer->currentFov());
        spinTimerExpired();
        return true;

      case 'a':
      case 'A':
        qfwp.worldRenderer->setView(qfwp.worldRenderer->currentPan() + 180.0,
                                    qfwp.worldRenderer->currentTilt(),
                                    qfwp.worldRenderer->currentFov());
        spinTimerExpired();
        return true;
    }
  }

  return false;
}

Boolean QFWorldPlayerHandler::virtualKeyPress(IKeyboardEvent &event)
{
  if (event.dispatchingWindow() == qfwp.movieWindow)
  {
    switch (event.virtualKey())
    {
      case IKeyboardEvent::left:
        qfwp.worldRenderer->QFWorldRenderer::setView(qfwp.worldRenderer->currentPan() -
                                    qfwp.worldRenderer->panStep(QFWorldRenderer::keyboard),
                                    qfwp.worldRenderer->currentTilt(),
                                    qfwp.worldRenderer->currentFov());
        spinTimerExpired();
        return true;

        case IKeyboardEvent::right:
        qfwp.worldRenderer->QFWorldRenderer::setView(qfwp.worldRenderer->currentPan() +
                                    qfwp.worldRenderer->panStep(QFWorldRenderer::keyboard),
                                    qfwp.worldRenderer->currentTilt(),
                                    qfwp.worldRenderer->currentFov());
        spinTimerExpired();
        return true;

      case IKeyboardEvent::up:
        qfwp.worldRenderer->QFWorldRenderer::setView(qfwp.worldRenderer->currentPan(),
                                    qfwp.worldRenderer->currentTilt() +
                                    qfwp.worldRenderer->tiltStep(QFWorldRenderer::keyboard),
                                    qfwp.worldRenderer->currentFov());
        spinTimerExpired();
        return true;

      case IKeyboardEvent::down:
        qfwp.worldRenderer->QFWorldRenderer::setView(qfwp.worldRenderer->currentPan(),
                                    qfwp.worldRenderer->currentTilt() -
                                    qfwp.worldRenderer->tiltStep(QFWorldRenderer::keyboard),
                                    qfwp.worldRenderer->currentFov());
        spinTimerExpired();
        return true;

      case IKeyboardEvent::insert:
        if (event.isCtrlDown())
        {
          qfwp.worldRenderer->copyFrame();
          return true;
        }

      default:
        return false;
    }
  }

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

Boolean QFWorldPlayerHandler::key(IKeyboardEvent &event)
{
  POINTL ptlPoint;
  HWND hwndFound;

  if (event.parameter2().number2() == VK_SHIFT && qfwp.worldRenderer->updateEnabled())
  {
    //Proceed only if pointer is over movie window.
    WinQueryPointerPos(HWND_DESKTOP, &ptlPoint);
    WinMapWindowPoints(HWND_DESKTOP, qfwp.movieWindow->handle(), &ptlPoint, 1);
    if (currentAction != none ||
        WinWindowFromPoint(qfwp.movieWindow->handle(), &ptlPoint, FALSE) != NULLHANDLE)
      if (spinning)
        if (event.isUpTransition())
          WinSetPointer(IWindow::desktopWindow()->handle(), QFWorldPlayer::spinHand);
        else
          WinSetPointer(IWindow::desktopWindow()->handle(), QFWorldPlayer::spinArrows);
    else
      if (currentAction == none)
           if (event.isUpTransition())
            WinSetPointer(IWindow::desktopWindow()->handle(), QFWorldPlayer::spinHand);
          else
            WinSetPointer(IWindow::desktopWindow()->handle(), QFWorldPlayer::openHand);
  }

  return false;
}

