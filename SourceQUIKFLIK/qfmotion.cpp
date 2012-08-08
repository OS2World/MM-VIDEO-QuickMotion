#include "qfmotion.hpp"
#include "quikflik.hpp"
#include "qfcontrol.hpp"

#include <ithread.hpp>
#include <ireslib.hpp>
#include <icolor.hpp>
#include <istattxt.hpp>
#include <ipopmenu.hpp>
#include <immsequ.hpp>
#include <immwave.hpp>
#include <immexcpt.hpp>
#include <idate.hpp>
#include <ifont.hpp>
#include <string.h>

#ifdef TRACE
#include <fstream.h>
extern ofstream *debugOutput;
extern void stampTime();
#endif

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
HMTX QFMotionPlayer::preparing;
ISequence<OldPlayer> QFMotionPlayer::oldPlayers;

/************************************************************************/
/* Classes                                                              */
/************************************************************************/

// MMDeviceData is private to IMMDevice.  Or at least it was supposed to be.
class IMMMessageWindow;
class IMMDeviceHandler;
class MMDeviceData : public IBase
{
public:
  MMDeviceData(IString anAlias, IString curDev)
  {
  }

  IMMMessageWindow       *aDeviceWindow;
  IMMDeviceHandler       *deviceHandler;
  Boolean                closeOnDestroy, sawFirstPassDevice;
  unsigned short         userParm;
  unsigned long          id, errorId;
  IString                alias, currentDevice;
  IMMDevice::OpenStatus  openStatus;
  static unsigned long   aliasCount;
};

/************************************************************************/
/* CONSTRUCTOR                                                          */
/************************************************************************/
QFMotionPlayer::QFMotionPlayer(QuickFlick& quickFlick, Boolean isMusicOnly, Boolean isSoundOnly)
              : QFPlayer(quickFlick), handler(*this)
{
   LONG returnCode;

   // Initialize plugin instance variables
   isMusicOnly_p = isMusicOnly;
   isSoundOnly_p = isSoundOnly;
   prepareThread = 0;
   window_p = 0;
   moviePlayer = 0;
   filePrepared = autoStartPointReached = autoStartPerformed = stoppedBeforeEnd = false;
   isPositionTracking = isPlaying = false;
   howLongMovieWillPlay = 0;

   firstPassDevice = atEnd = true;
   oldPlayerCursor = 0;
   pWaitForData = 0;

   // Initialize user interface components
   screen = movieWindow = 0;
   menu = 0;

   // Create event semaphore for signaling that prepare thread may proceed after thread 1 actions
   returnCode = DosCreateEventSem(NULL, &proceedWithPrepare, 0, FALSE);

   handler.QFPlayerHandler::handleEventsFor(&qf.mainThreadWindow());
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
void QFMotionPlayer::gotHeader()
{
#ifdef TRACE
  stampTime();
  *debugOutput << "Got header:" << '\t' << &qf << endl;
#endif

  if (qf.window() && !window_p)
    setWindow(*qf.window());

  if (qf.disabled())
    return;

  if (moviePlayer) // In case we are retrying open in response to error
  {
    stopHandlingNotificationsFor(*moviePlayer);
    moviePlayer->disableNotification();
    delete moviePlayer;
  }
  else
    qf.playerCreated(); // Tell plugin we're here (on thread 1, first time only)

  // Create player and launch thread to prepare the movie
  if (isMusicOnly_p)
    moviePlayer = new IMMSequencer(false);
  else
    if (isSoundOnly_p)
      moviePlayer = new IMMWaveAudio(false);
    else
      moviePlayer = new IMMDigitalVideo(false);

  moviePlayer->enableNotification();
  handleNotificationsFor(*moviePlayer);

  if (qf.streamDone())
  {
    autoStartPointReached = true;
    finishStream();
    if (qf.controller())
      qf.controller()->setFullPercent(1.0);
  }

  prepareThread = new IThread(new IThreadMemberFn<QFMotionPlayer>(*this, QFMotionPlayer::prepare));
}

/************************************************************************/
/* OPEN                                                                 */
/************************************************************************/
void QFMotionPlayer::open()
{
#ifdef TRACE
  stampTime();
  *debugOutput << "Open:" << '\t' << &qf << endl;
#endif

  moviePlayer->open(IString(), true, IMMDevice::nowait);
}

/************************************************************************/
/* LOAD                                                                 */
/************************************************************************/
void QFMotionPlayer::load()
{
#ifdef TRACE
  stampTime();
  *debugOutput << "Load:" << '\t' << &qf << endl;
#endif

  try
  {
    moviePlayer->load(qf.fileName(), !isMusicOnly_p, IMMDevice::nowait);
  }
  catch (IException exception)
  {
#ifdef TRACE
  stampTime();
  *debugOutput << "Load EXCEPTION: " << exception.errorId() << ' ' << exception.text() << endl;
#endif
    qf.setStreamStopped();
    DosPostEventSem(proceedWithPrepare);
  }
}

/************************************************************************/
/* CUE                                                                  */
/************************************************************************/
void QFMotionPlayer::cue()
{
#ifdef TRACE
  stampTime();
  *debugOutput << "Cue:" << '\t' << &qf << endl;
#endif

  if (!isSoundOnly_p && !isMusicOnly_p)
    seek(0);

  if (movieWindow)
  {
    movieWindow->show();
    movieWindow->enable();
  }
  if (qf.controller())
    qf.controller()->enablePlay();

  if (pWaitForData)
    *pWaitForData = FALSE;

  if (QuickFlick::cueDuringPrepare())
    DosPostEventSem(proceedWithPrepare);
}

/************************************************************************/
/* PLAY                                                                 */
/************************************************************************/
void QFMotionPlayer::play()
{
  if (isPlaying)
    return;

  if (qf.controller() && !isPositionTracking)
  {
    isPositionTracking = true;
    moviePlayer->startPositionTracking(SLIDER_UPDATE_INTERVAL);
  }

  // Issue play from 0 if atEnd flag is set
  if (atEnd)
  {
    try
    {
#ifdef TRACE
  stampTime();
  *debugOutput << "Play(0):\t" << &qf << endl;
#endif
      moviePlayer->play(0);
    }
    catch(IException exception)
    {
#ifdef TRACE
  stampTime();
  *debugOutput << "Play EXCEPTION: " << exception.errorId() << ' ' << exception.text() << endl;
#endif
    }
  }

  // Issue play, repeating if atEnd, except in the case of wave audio
  if (!atEnd || !isSoundOnly_p)
  {
    for (int i=1; i<=2; i++)
    {
      try
      {
#ifdef TRACE
  stampTime();
  *debugOutput << "Play:\t" << &qf << endl;
#endif
        moviePlayer->play();
        break;
      }
      catch(IException exception) // Might get 5603 'stream not stopped'
      {
#ifdef TRACE
  stampTime();
  *debugOutput << "Play EXCEPTION: " << exception.errorId() << ' ' << exception.text() << endl;
#endif
      }
    }
  }

  if (screen && screen->textLength())
    setScreenText("");

  atEnd = stoppedBeforeEnd = false;
  isPlaying = true;
}

/************************************************************************/
/* PAUSE                                                                */
/************************************************************************/
void QFMotionPlayer::pause()
{
  if (isPositionTracking)
  {
    isPositionTracking = false;
    moviePlayer->stopPositionTracking();
    moviePlayer->deletePendingEvents(IMMDevice::positionChangeEvent);
  }

  moviePlayer->pause(IMMDevice::wait);

  isPlaying = false;

  if (qf.controller())
    qf.controller()->enablePlay();
}

/************************************************************************/
/* SEEK                                                                 */
/************************************************************************/
void QFMotionPlayer::seek(long seekTime)
{
#ifdef TRACE
  stampTime();
  *debugOutput << "Seek:" << '\t' << &qf << "\tPosition: " << seekTime << endl;
#endif

  if (isPositionTracking)
  {
    isPositionTracking = false;
    moviePlayer->stopPositionTracking();
    moviePlayer->deletePendingEvents(IMMDevice::positionChangeEvent);
  }

  atEnd = stoppedBeforeEnd = false;

  // The second one is going to break, so we may as well break it right now.  Sets up good future seek.
  for (int i=1; i<=2; i++)
  {
    try
    {
      moviePlayer->seek(seekTime, IMMDevice::wait);
    }
    catch(IException exception)
    {
#ifdef TRACE
  stampTime();
  *debugOutput << "Seek EXCEPTION: " << exception.errorId() << ' ' << exception.text() << endl;
#endif
    }
  }

  if (screen && screen->textLength())
    setScreenText("");
}

/************************************************************************/
/* DESTROY                                                              */
/************************************************************************/
Boolean QFMotionPlayer::destroy(IEvent &event)
{
  if (event.dispatchingWindow() == window_p)
  {
#ifdef TRACE
  stampTime();
  *debugOutput << "Plugin window destroyed!:" << '\t' << &qf << endl;
#endif
  }

  return false;
}

/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                     CUSTOM MEMBER FUNCTIONS                          */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/* INITIALIZE                                                           */
/************************************************************************/
void QFMotionPlayer::initialize()
{
  ULONG returnCode;

  returnCode = DosOpenMutexSem(SEM_PREPARING, &preparing);
  if (returnCode)
    DosCreateMutexSem(SEM_PREPARING, &preparing, DC_SEM_SHARED, FALSE);
}

/************************************************************************/
/* SET WINDOW                                                           */
/************************************************************************/
void QFMotionPlayer::setWindow(IWindow& window)
{
  window_p = &window;

  handler.IResizeHandler::handleEventsFor(window_p);
  handler.IPaintHandler::handleEventsFor(window_p);
  handler.ICommandHandler::handleEventsFor(window_p);
  handler.QFPlayerHandler::handleEventsFor(window_p);
  menu = new IPopUpMenu(IResourceId(MOTION_POPUP_ID, QuickFlick::resourceLibrary()), window_p);
  menu->disableItem(MI_MOTION_SAVEAS);

  IRectangle screenRectangle = (qf.controller() ?
                                IRectangle(IPoint(0, CONTROL_PANEL_HEIGHT), IPoint(window_p->size())) :
                                IRectangle(window_p->size()));
  screen = new IStaticText(SCREEN_ID, window_p, window_p, screenRectangle,
                           IWindow::noStyle | IWindow::clipChildren);
  screen->setFillColor(IColor::paleGray);
  screen->setBackgroundColor(IColor::paleGray);
  screen->enableFillBackground();
  screen->setAlignment(IStaticText::topLeftWrapped);
  screen->setFont(IFont("Helv", 8));
  if (qf.disabled())
    setScreenText("This version of QuickFlick has expired.\nTo upgrade:\n - Open Netscape\n - Choose Help->About Plug-ins->Practice");
  handler.IMenuHandler::handleEventsFor(screen);
  screen->show();
}

/************************************************************************/
/* SET SCREEN TEXT                                                      */
/************************************************************************/
void QFMotionPlayer::setScreenText(const char *text)
{
  if (screen)
    screen->setText(text);
}

/************************************************************************/
/* MOVIE SIZE                                                           */
/************************************************************************/
ISize QFMotionPlayer::movieSize()
{
  if (movieWindow)
    return ISize(((IMMDigitalVideo*)moviePlayer)->videoFileWidth(),
                 ((IMMDigitalVideo*)moviePlayer)->videoFileHeight());
  else
    return ISize();
}

/************************************************************************/
/* PREPARE                                                              */
/************************************************************************/
void QFMotionPlayer::prepare()
{
  // Get a window from the module's service thread for use by this plugin instance
  serviceThreadWindow = (IObjectWindow*)((QuickFlick::serviceThreadControlWindow()->sendEvent(WM_CUSTOM_CREATE, this)).asLong());
  handler.QFPlayerHandler::handleEventsFor(serviceThreadWindow);

  unsigned long postCount;

#ifdef TRACE
  stampTime();
  *debugOutput << "Open wait:" << '\t' << &qf << endl;
#endif

  setScreenText("Waiting for device...");

  DosRequestMutexSem(preparing, SEM_INDEFINITE_WAIT);

  if (qf.streamStopped())
  {
    DosReleaseMutexSem(preparing);
    return;
  }

  // Open (if movie player is already open, we are re-loading)
  if (!moviePlayer->isOpen())
  {
    serviceThreadWindow->postEvent(WM_CUSTOM_OPEN);
    DosWaitEventSem(proceedWithPrepare, SEM_INDEFINITE_WAIT);
    DosResetEventSem(proceedWithPrepare, &postCount);
    if (qf.streamStopped())
    {
      DosReleaseMutexSem(preparing);
      return;
    }
  }

  // Load
  setScreenText("Loading movie...");
  serviceThreadWindow->postEvent(WM_CUSTOM_LOAD);
  DosWaitEventSem(proceedWithPrepare, SEM_INDEFINITE_WAIT);
  DosResetEventSem(proceedWithPrepare, &postCount);
  if (qf.streamStopped())
  {
    DosReleaseMutexSem(preparing);
    return;
  }

  howLongMovieWillPlay = moviePlayer->length();

  setScreenText("Buffering movie data...");
  if (QuickFlick::cueDuringPrepare())
  {
    serviceThreadWindow->postEvent(WM_CUSTOM_CUE);
    DosWaitEventSem(proceedWithPrepare, SEM_INDEFINITE_WAIT);
    DosResetEventSem(proceedWithPrepare, &postCount);
    if (qf.streamStopped())
    {
      DosReleaseMutexSem(preparing);
      return;
    }
  }

  filePrepared = true;

  if (autoStartPointReached && !autoStartPerformed) // A destroyed stream could have flagged autostart
  {
#ifdef TRACE
  stampTime();
  *debugOutput << "Start issued from prepare:" << '\t' << &qf << endl;
#endif
    start();
  }

  DosReleaseMutexSem(preparing);
}

/************************************************************************/
/* PERFORM AUTO START                                                   */
/************************************************************************/
void QFMotionPlayer::start()
{
  autoStartPerformed = true;

  if (qf.willAutoplay())
  {
    if (movieWindow)
    {
      movieWindow->show();
      movieWindow->enable();
    }
    if (qf.controller())
      qf.controller()->enablePause();
    serviceThreadWindow->postEvent(WM_CUSTOM_PLAY);
  }
  else
    if (!QuickFlick::cueDuringPrepare())
        serviceThreadWindow->postEvent(WM_CUSTOM_CUE);

  if (qf.controller())
    qf.controller()->removeDetent();
}

/************************************************************************/
/* HANDLE PLAY COMMAND                                                  */
/************************************************************************/
void QFMotionPlayer::handlePlayCommand()
{
  if (qf.disabled())
    return;

  if (isPlaying)
  {
    if (moviePlayer->mode() == IMMDevice::notReady)
    {
#ifdef TRACE
  stampTime();
  *debugOutput << "Reacquire:" << '\t' << &qf << endl;
#endif
      moviePlayer->acquire();
    }
    else
    {
#ifdef TRACE
  stampTime();
  *debugOutput << "Manual pause:" << '\t' << &qf << endl;
#endif
      serviceThreadWindow->postEvent(WM_CUSTOM_PAUSE);
    }
  }
  else
  {
    if (qf.controller())
      qf.controller()->enablePause();
    if (stoppedBeforeEnd)
      serviceThreadWindow->postEvent(WM_CUSTOM_SEEK, moviePlayer->position());
#ifdef TRACE
  stampTime();
  *debugOutput << "Manual play:" << '\t' << &qf << endl;
#endif
    serviceThreadWindow->postEvent(WM_CUSTOM_PLAY);
  }
}

/************************************************************************/
/* FINISH STREAM                                                        */
/************************************************************************/
void QFMotionPlayer::finishStream()
{
  autoStartPointReached = true;

  if (menu)
    menu->enableItem(MI_MOTION_SAVEAS); // All data is here, so enable save as... feature

  if (filePrepared && !autoStartPerformed)
  {
#ifdef TRACE
  stampTime();
  *debugOutput << "Start issued from finish stream:" << '\t' << &qf << endl;
#endif

    start();
  }
}

/************************************************************************/
/* HANDLE SCROLLING FOR                                                 */
/************************************************************************/
void QFMotionPlayer::handleScrollingFor(IWindow& viewPort)
{
  handler.IScrollHandler::handleEventsFor(&viewPort);
  handler.IKeyboardHandler::handleEventsFor(&viewPort);
}

/************************************************************************/
/* STOP HANDLING SCROLLING FOR                                          */
/************************************************************************/
void QFMotionPlayer::stopHandlingScrollingFor(IWindow& viewPort)
{
  handler.IScrollHandler::stopHandlingEventsFor(&viewPort);
  handler.IKeyboardHandler::stopHandlingEventsFor(&viewPort);
}

/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                        OBSERVER FUNCTIONS                            */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/************************************************************************/
/* DISPATCH NOTIFICATION EVENT                                          */
/************************************************************************/
QFMotionPlayer& QFMotionPlayer::dispatchNotificationEvent(const INotificationEvent &event)
{
   if (event.notificationId() == IMMDevice::positionChangeId) // Position Change Notification
   {
     IMMPositionChangeEvent* positionChangeEvent = (IMMPositionChangeEvent*)(event.eventData().asLong());
     qf.controller()->setArmPercent((float)(unsigned long)positionChangeEvent->position() / howLongMovieWillPlay);
   }

   else // Pass Device Notification

   if (event.notificationId() == IMMDevice::passDeviceId)
   {
     IMMPassDeviceEvent* passDeviceEvent = (IMMPassDeviceEvent*)(event.eventData().asLong());

     // GainingUse
     if (passDeviceEvent->isGainingUse())
     {
       if (firstPassDevice)
       {
         firstPassDevice = false;
         if (!moviePlayer->deviceId()) // Sometimes the damn thing has no device ID even though we're here
         {
#ifdef TRACE
  stampTime();
  *debugOutput << "Opened with NO DEVICE ID:" << '\t' << &qf << endl;
#endif
           ((MMDeviceData*)(moviePlayer->fDeviceData))->id = passDeviceEvent->parameter1().lowNumber();
         }
         DosPostEventSem(proceedWithPrepare);
       }
     }
     // Losing use
     else
     {
     }
   } // Pass Device Notification

   else // Command Notification

   if (event.notificationId() == IMMDevice::commandNotifyId)
   {
     IMMNotifyEvent* notifyEvent = (IMMNotifyEvent*)(event.eventData().asUnsignedLong());

     if (notifyEvent->command() == IMMNotifyEvent::open) // Open Command
     {
       if (notifyEvent->errorId())
       {
#ifdef TRACE
  stampTime();
  *debugOutput << "Open ERROR:" << '\t' << &qf << '\t' << notifyEvent->errorId() << endl;
#endif
         qf.setStreamStopped();
         DosPostEventSem(proceedWithPrepare);
       }
     } // Open
     else
     if (notifyEvent->command() == IMMNotifyEvent::load) // Load Command
     {
       if (!(isMusicOnly_p || isSoundOnly_p))
       {
         // Make a movie window, if there's a screen to put it on and we need to put it there
         if (screen && !movieWindow && (qf.scale() != QF_SCALE_DETATCH))
         {
           IRectangle movieRectangle = IRectangle(screen->rect().size());
           if (qf.scale() == QF_SCALE_CENTER)
             movieRectangle = IRectangle(((IMMDigitalVideo*)moviePlayer)->videoFileWidth(),
                                         ((IMMDigitalVideo*)moviePlayer)->videoFileHeight()).
                                         centeredAt(movieRectangle.center());
           movieWindow = new IStaticText(MOVIE_WINDOW_ID, screen, screen, movieRectangle, IWindow::noStyle);
           movieWindow->disable();
           movieWindow->hide();
           handler.IMouseHandler::handleEventsFor(movieWindow);
           handler.IMenuHandler::handleEventsFor(movieWindow);
           if (qf.setMovieWindowHandleAddress())
             qf.setMovieWindowHandleAddress()(movieWindow->handle());
         }
         else if (qf.setMovieWindowHandleAddress())
           qf.setMovieWindowHandleAddress()(0);

#ifdef TRACE
  stampTime();
  *debugOutput << "Set digitalvideo window:" << '\t' << &qf << endl;
#endif
         // Tell the movie player where it can put it's movie
         if (movieWindow)
           ((IMMDigitalVideo*)moviePlayer)->setWindow(*movieWindow);

         // Create an entry on the old player stack.  Players must be closed in the reverse order that setWindow is called.
         // Many lives were lost to bring us this information.  Doing this even for detatched movies just in case.
         OldPlayer oldPlayerEntry;
         oldPlayerEntry.moviePlayer = (IMMDigitalVideo*)moviePlayer;
         oldPlayerEntry.isDeletable = false;
         oldPlayerCursor = oldPlayers.newCursor();
         oldPlayers.addAsLast(oldPlayerEntry, *oldPlayerCursor);
       } // !isMusicOnly_p
       else if (qf.setMovieWindowHandleAddress())
         qf.setMovieWindowHandleAddress()(0);

       DosPostEventSem(proceedWithPrepare);

     } // Load
     else
     if (notifyEvent->command() == IMMNotifyEvent::play) // Play Command
     {
       IMMNotifyEvent::SuccessCode successCode = notifyEvent->successCode();
#ifdef TRACE
  stampTime();
  *debugOutput << "Play notify:" << '\t' << &qf << '\t' << *notifyEvent << endl;
#endif

       if (successCode != IMMNotifyEvent::preempted)
         isPlaying = false;

       if (successCode == IMMNotifyEvent::successful)
       {
         atEnd = true;
         if (!qf.streamDone())
           stoppedBeforeEnd = true;
         if (qf.willLoop() && qf.streamDone())
           serviceThreadWindow->postEvent(WM_CUSTOM_PLAY);
         else
         {
           if (qf.controller())
           {
             if (qf.streamDone())
               qf.controller()->setArmPercent(1.0);
             qf.controller()->enablePlay();
           }
         }
       }  // Successful
     } // Play
   } // Command Notification

   return *this;
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
/* RESIZE                                                               */
/************************************************************************/
Boolean QFMotionPlayer::windowResize(IResizeEvent &event)
{
  IRectangle screenRectangle;
  if (screen)
  {
    screenRectangle = (qf.controller() ?
                       IRectangle(IPoint(0, CONTROL_PANEL_HEIGHT), IPoint(event.newSize())) :
                       IRectangle(event.newSize()));
    screen->moveSizeTo(screenRectangle);
    if (movieWindow)
    {
      IRectangle movieRectangle = IRectangle(screenRectangle.size());
      if (qf.scale() == QF_SCALE_CENTER && moviePlayer && howLongMovieWillPlay)
        movieRectangle = IRectangle(((IMMDigitalVideo*)moviePlayer)->videoFileWidth(),
                                    ((IMMDigitalVideo*)moviePlayer)->videoFileHeight()).
                                    centeredAt(movieRectangle.center());
      movieWindow->moveSizeTo(movieRectangle);
    }
  }

  if (qf.controller())
  {
    float armPercent = .0;
    if (filePrepared && !isPlaying && moviePlayer->position())
    {
      armPercent = moviePlayer->position();
      armPercent /= howLongMovieWillPlay;
    }
    qf.createController(armPercent);
  }

  return false;
}

/************************************************************************/
/* COMMAND                                                              */
/************************************************************************/
Boolean QFMotionPlayer::command(ICommandEvent &event)
{
  IString saveAsFileName;
  FOURCC ioProc;

  switch (event.commandId())
  {
    case PLAY_BUTTON_ID:
      handlePlayCommand();
      return true;

    case MI_MOTION_SAVEAS:
      saveAsFileName = qf.showSaveAsDialog(window_p);
      if (saveAsFileName.length())
      {
        if (movieWindow)
          movieWindow->disable();
        if (qf.controller())
          qf.controller()->disableControls();
        if (isPositionTracking)
        {
          isPositionTracking = false;
          moviePlayer->stopPositionTracking();
          moviePlayer->deletePendingEvents(IMMDevice::positionChangeEvent);
        }

        moviePlayer->stop();

        ((IMMDigitalVideo*)moviePlayer)->saveAs(saveAsFileName, IMMDevice::wait);

        autoStartPerformed = false;
        atEnd = true;
        if (qf.controller())
          qf.controller()->setArmPercent(.0);
        prepareThread = new IThread(new IThreadMemberFn<QFMotionPlayer>(*this, QFMotionPlayer::prepare));
      }
      return true;

    default:
      return false;
  }
}


/************************************************************************/
/* DESTRUCTOR                                                           */
/************************************************************************/
QFMotionPlayer::~QFMotionPlayer()
{
  qf.setStreamStopped();

  handler.QFPlayerHandler::stopHandlingEventsFor(&qf.mainThreadWindow());

  if (window_p)
  {
    handler.IPaintHandler::stopHandlingEventsFor(window_p);
    handler.IResizeHandler::stopHandlingEventsFor(window_p);
    handler.ICommandHandler::stopHandlingEventsFor(window_p);
    handler.QFPlayerHandler::stopHandlingEventsFor(window_p);
  }

  if (serviceThreadWindow)
    handler.QFPlayerHandler::stopHandlingEventsFor(serviceThreadWindow);

  if (moviePlayer)
  {
    if (isPositionTracking)
      moviePlayer->stopPositionTracking();
    stopHandlingNotificationsFor(*moviePlayer);
    moviePlayer->disableNotification();
    if (moviePlayer->isOpen() && moviePlayer->deviceId())
      moviePlayer->stop();

    if (!oldPlayerCursor) // No setWindow was done for this player
      delete moviePlayer;
    else
    {
      oldPlayers.elementAt(*oldPlayerCursor).isDeletable = true; // Flag this player as deletable
      delete oldPlayerCursor; // That was cursor's sole purpose
      while (!oldPlayers.isEmpty() && oldPlayers.lastElement().isDeletable) // Delete deletable players from the top of the stack
      {
        delete oldPlayers.lastElement().moviePlayer;
        oldPlayers.removeLast();
      }
    }
  }

  if (movieWindow)
  {
    handler.IMouseHandler::stopHandlingEventsFor(movieWindow);
    handler.IMenuHandler::stopHandlingEventsFor(movieWindow);
    delete movieWindow;
  }

  if (screen)
  {
    handler.IMenuHandler::stopHandlingEventsFor(screen);
    delete screen;
  }

  delete menu;

  // Close semaphores
  DosCloseEventSem(proceedWithPrepare);

  if (prepareThread && prepareThread->isStarted())
    IThread::current().waitFor(*prepareThread);
}

/************************************************************************/
/* SHUTDOWN                                                             */
/************************************************************************/
void QFMotionPlayer::shutdown()
{
  DosCloseMutexSem(preparing);
}

