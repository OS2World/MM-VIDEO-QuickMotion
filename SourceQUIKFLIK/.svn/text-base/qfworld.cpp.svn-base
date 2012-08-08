#include "quikflik.hpp"
#include "qfworld.hpp"
#include "qfwobjc.hpp"
#include "qfwpano.hpp"
#include "moov.hpp"

#include <ireslib.hpp>
#include <icolor.hpp>
#include <istattxt.hpp>
#include <ipopmenu.hpp>
#include <idate.hpp>
#include <ifont.hpp>
#include <string.h>
#include <float.h>

#ifdef TRACE
#include <fstream.h>
extern ofstream *debugOutput;
extern void stampTime();
#endif

IPointerHandle QFWorldPlayer::openHand;
IPointerHandle QFWorldPlayer::pullHand;
IPointerHandle QFWorldPlayer::spinArrows;
IPointerHandle QFWorldPlayer::spinHand;
IPointerHandle QFWorldPlayer::zoom;
IPointerHandle QFWorldPlayer::hotHand;

/************************************************************************/
/* CONSTRUCTOR                                                          */
/************************************************************************/
QFWorldPlayer::QFWorldPlayer(QuickFlick& quickFlick)
             : QFPlayer(quickFlick), handler(*this)
{
   LONG returnCode;

   worldRenderer = 0;
   window_p = 0;
   filePrepared = autoStartPointReached = autoStartPerformed = false;
   howLongMovieWillPlay = 0;

   pWaitForData = 0;

   // Initialize user interface components
   screen = movieWindow = 0;
   menu = 0;

   proceedWithPrepare = NULLHANDLE;

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
void QFWorldPlayer::gotHeader()
{
#ifdef TRACE
  stampTime();
  *debugOutput << "Got header:" << '\t' << &qf << endl;
#endif

  qf.deleteController();

  if (qf.window() && !window_p)
    setWindow(*qf.window());

  if (qf.disabled())
    return;

  qf.playerCreated(); // Tell plugin we're here (on thread 1)

  if (qf.streamDone())
  {
    autoStartPointReached = true;
    finishStream();
  }

  /* Create the appropriate renderer.  Return without loading if an "unsupported" exception is thrown. */
  try
  {
    switch (qf.movieAtom()->userDataAtom->controllerTypeAtom->controllerFourcc)
    {
      case mmioFOURCC('s', 't', 'n', 'a'):
        worldRenderer = new QFObjectRenderer(*this, *qf.movieAtom(), qf.movieFile());
        break;

      case mmioFOURCC('S', 'T', 'p', 'n'):
        worldRenderer = new QFPanoramaRenderer(*this, *qf.movieAtom(), qf.movieFile());
        break;

      case mmioFOURCC('q', 't', 'v', 'r'):
        setScreenText("QuickFlick Error: QTVR 2.0 format not supported.");
        qf.setStreamStopped();
        return;

      default:
        setScreenText("QuickFlick Error: Unsupported controller type.");
        qf.setStreamStopped();
        return;
    }
  }
  catch (Unsupported unsupported)
  {
    setScreenText(unsupported.text());
    qf.setStreamStopped();
    return;
  }

  // Create movie window
  IRectangle movieRectangle = IRectangle(screen->rect().size());
  if (qf.scale() == QF_SCALE_CENTER || qf.scale() == QF_SCALE_DETATCH)
    movieRectangle = IRectangle(worldRenderer->movieSize()).centeredAt(movieRectangle.center());
  movieWindow = new IStaticText(MOVIE_WINDOW_ID, screen, screen, movieRectangle, IWindow::noStyle);
  handler.IMenuHandler::handleEventsFor(movieWindow);
  handler.QFPlayerHandler::handleEventsFor(movieWindow);
  handler.IMouseHandler::handleEventsFor(movieWindow);
  handler.IKeyboardHandler::handleEventsFor(movieWindow);
  if (qf.setMovieWindowHandleAddress())
    qf.setMovieWindowHandleAddress()(movieWindow->handle());
  worldRenderer->setWindow(*movieWindow);

  // Get a window from the module's service thread for use by this instance
  serviceThreadWindow = (IObjectWindow*)((QuickFlick::serviceThreadControlWindow()->sendEvent(WM_CUSTOM_CREATE, this)).asLong());
  handler.QFPlayerHandler::handleEventsFor(serviceThreadWindow);

  // Start renderer
  worldRenderer->start();
}

/************************************************************************/
/* DESTROY                                                              */
/************************************************************************/
Boolean QFWorldPlayer::destroy(IEvent &event)
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
void QFWorldPlayer::initialize()
{
  _fpreset();
  unsigned int oldstate = _control87(EM_UNDERFLOW, EM_UNDERFLOW);

  openHand = QuickFlick::resourceLibrary().loadPointer(OPENHAND_ID);
  pullHand = QuickFlick::resourceLibrary().loadPointer(PULLHAND_ID);
  spinArrows = QuickFlick::resourceLibrary().loadPointer(SPINARROWS_ID);
  spinHand = QuickFlick::resourceLibrary().loadPointer(SPINHAND_ID);
  zoom = QuickFlick::resourceLibrary().loadPointer(ZOOM_ID);
  hotHand = QuickFlick::resourceLibrary().loadPointer(HOTHAND_ID);
}

/************************************************************************/
/* SET WINDOW                                                           */
/************************************************************************/
void QFWorldPlayer::setWindow(IWindow& window)
{
  window_p = &window;

  handler.IResizeHandler::handleEventsFor(window_p);
  handler.IPaintHandler::handleEventsFor(window_p);
  handler.ICommandHandler::handleEventsFor(window_p);
  handler.QFPlayerHandler::handleEventsFor(window_p);
  menu = new IPopUpMenu(IResourceId(WORLD_POPUP_ID, QuickFlick::resourceLibrary()), window_p);
  menu->disableItem(MI_WORLD_SAVEAS);

  IRectangle screenRectangle = IRectangle(window_p->size());
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
void QFWorldPlayer::setScreenText(const char *text)
{
  if (screen)
    screen->setText(text);
}

/************************************************************************/
/* MOVIE SIZE                                                           */
/************************************************************************/
ISize QFWorldPlayer::movieSize()
{
  if (worldRenderer)
    return worldRenderer->movieSize();
  else
    return ISize();
}

/************************************************************************/
/* PERFORM AUTO START                                                   */
/************************************************************************/
void QFWorldPlayer::start()
{
  autoStartPerformed = true;
}

/************************************************************************/
/* FINISH STREAM                                                        */
/************************************************************************/
void QFWorldPlayer::finishStream()
{
  autoStartPointReached = true;

  if (menu)
    menu->enableItem(MI_WORLD_SAVEAS);

  if (!autoStartPerformed)
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
void QFWorldPlayer::handleScrollingFor(IWindow& viewPort)
{
  handler.IScrollHandler::handleEventsFor(&viewPort);
  handler.IKeyboardHandler::handleEventsFor(&viewPort);
}

/************************************************************************/
/* STOP HANDLING SCROLLING FOR                                          */
/************************************************************************/
void QFWorldPlayer::stopHandlingScrollingFor(IWindow& viewPort)
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
Boolean QFWorldPlayer::windowResize(IResizeEvent &event)
{
  IRectangle screenRectangle;
  if (screen)
  {
    screenRectangle = IRectangle(event.newSize());
    screen->moveSizeTo(screenRectangle);
    if (movieWindow)
    {
      IRectangle movieRectangle = IRectangle(screenRectangle.size());
      if (qf.scale() == QF_SCALE_CENTER)
        movieRectangle = IRectangle(worldRenderer->movieSize()).centeredAt(movieRectangle.center());
      movieWindow->moveSizeTo(movieRectangle);
    }
  }

  return false;
}

/************************************************************************/
/* SAVE AS                                                              */
/************************************************************************/
void QFWorldPlayer::saveAs(IString fileName)
{
  HMMIO outFile = mmioOpen(fileName, NULL, MMIO_CREATE | MMIO_WRITE);
  if (!outFile)
    return;

  char* buffer = new char[4096];
  long bytesRead;
  mmioSeek(qf.movieFile(), 0, SEEK_SET);

  do
  {
    bytesRead = mmioRead(qf.movieFile(), buffer, 4096);
    mmioWrite(outFile, buffer, bytesRead);
  } while (bytesRead == 4096);

  delete[] buffer;
  mmioClose(outFile, 0);
}

/************************************************************************/
/* COMMAND                                                              */
/************************************************************************/
Boolean QFWorldPlayer::command(ICommandEvent &event)
{
  IString saveAsFileName;
  FOURCC ioProc;

  switch (event.commandId())
  {
    case MI_WORLD_SAVEAS:
      saveAsFileName = qf.showSaveAsDialog(window_p);
      if (saveAsFileName.length())
        saveAs(saveAsFileName);
      return true;

    case MI_WORLD_CAPTUREFRAME:
      saveAsFileName = qf.showCaptureFrameDialog(window_p, ioProc);
      if (saveAsFileName.length())
        worldRenderer->captureFrame(saveAsFileName, ioProc);
      return true;

    case MI_WORLD_COPY:
      worldRenderer->copyFrame();
      return true;

    case MI_WORLD_LEFT90:
      worldRenderer->setView(worldRenderer->currentPan() - 90.0, worldRenderer->currentTilt(), worldRenderer->currentFov());
      worldRenderer->update();
      return true;

    case MI_WORLD_RIGHT90:
      worldRenderer->setView(worldRenderer->currentPan() + 90.0, worldRenderer->currentTilt(), worldRenderer->currentFov());
      worldRenderer->update();
      return true;

    case MI_WORLD_TURNAROUND180:
      worldRenderer->setView(worldRenderer->currentPan() + 180.0, worldRenderer->currentTilt(), worldRenderer->currentFov());
      worldRenderer->update();
      return true;

    default:
      return false;
  }
}


/************************************************************************/
/* DESTRUCTOR                                                           */
/************************************************************************/
QFWorldPlayer::~QFWorldPlayer()
{
  qf.setStreamStopped();

  if (handler.spinTimer.isStarted())
    handler.spinTimer.stop();

  handler.QFPlayerHandler::stopHandlingEventsFor(&qf.mainThreadWindow());

  if (window_p)
  {
    handler.IResizeHandler::stopHandlingEventsFor(window_p);
    handler.IPaintHandler::stopHandlingEventsFor(window_p);
    handler.ICommandHandler::stopHandlingEventsFor(window_p);
    handler.QFPlayerHandler::stopHandlingEventsFor(window_p);
  }


  if (serviceThreadWindow)
    handler.QFPlayerHandler::stopHandlingEventsFor(serviceThreadWindow);

  if (movieWindow)
  {
    handler.IMenuHandler::stopHandlingEventsFor(movieWindow);
    handler.QFPlayerHandler::stopHandlingEventsFor(movieWindow);
    handler.IMouseHandler::stopHandlingEventsFor(movieWindow);
    handler.IKeyboardHandler::stopHandlingEventsFor(movieWindow);
  }

  WinSetPointer(IWindow::desktopWindow()->handle(), ISystemPointerHandle::arrow);

  if (screen)
    handler.IMenuHandler::stopHandlingEventsFor(screen);

  delete worldRenderer;
  delete movieWindow;
  delete screen;
  delete menu;
}

/************************************************************************/
/* SHUTDOWN                                                             */
/************************************************************************/
void QFWorldPlayer::shutdown()
{
}

