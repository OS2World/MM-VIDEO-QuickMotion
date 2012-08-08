#include "moviefrm.hpp" //MovieFrame
#include "moov.hpp" //MovieAtom
#include "qf.h"

#include <ithread.hpp>
#include <idmhndlr.hpp>
#include <istparse.hpp>
#include <imsgbox.hpp>
#include <isubmenu.hpp>

#include <stdio.h>

#define EFFECT_CP_HEIGHT (quickFlick && quickFlick->controller() ? CONTROL_PANEL_HEIGHT : 0)

MovieFrame::MovieFrame()
           : IFrameWindow(),
             systemMenu(this),
             canvas(FID_CLIENT, this, this, IRectangle(),IStaticText::defaultStyle() | IWindow::clipChildren),
             grabbable(true),
             stretchable(true),
             title(this),
             fileDialog_p(0),
             quickFlick(0), movieWindow(0), logo(0)
{
  setIcon(QF_ID);
  setClient(&canvas);

  titleBarSize = frameRectFor(IRectangle()).size();
  titleBarSize.setWidth(titleBarSize.width() - 2 * borderWidth());
  titleBarSize.setHeight(titleBarSize.height() - 2 * borderHeight());

/* Set up system and popup menus */
  popUpMenu = new IPopUpMenu(WND_POPUP, &canvas);
  IMenuItem windowSubmenu(MI_WINDOW);
  windowSubmenu.setIndex(0).setText(popUpMenu->menuItem(MI_WINDOWFORPOPUP).text());
  systemMenu.addItem(windowSubmenu, ISystemMenu::idPulldown).addSubmenu(MI_WINDOW);
  IMenu::Cursor systemCursor(systemMenu, ISystemMenu::idPulldown);
  for (systemCursor.setToNext(); systemCursor.isValid(); systemCursor.setToNext())
  {
    IMenuItem item = systemMenu.elementAt(systemCursor);
    systemMenu.deleteAt(systemCursor);
    item.setIndex();
    systemMenu.addItem(item, MI_WINDOW);
    popUpMenu->addItem(item, MI_WINDOWFORPOPUP);
  }
  IMenu::Cursor popUpCursor(*popUpMenu);
  popUpCursor.setToLast();
  for (popUpCursor.setToPrevious(); popUpCursor.isValid(); popUpCursor.setToPrevious())
  {
    IMenuItem item = popUpMenu->elementAt(popUpCursor);
    item.setIndex(0);
    systemMenu.addItem(item, ISystemMenu::idPulldown);
  }

  screenSize.setWidth(WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN));
  usableScreenSize.setWidth(screenSize.width() - 2 * (HORIZ_MARGIN + borderWidth()));
  screenSize.setHeight(WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN));
  usableScreenSize.setHeight(screenSize.height() - titleBarSize.height() - CONTROL_PANEL_HEIGHT -
                             2 * (VERT_MARGIN + borderHeight()));

  imageCenter = nextShellRect().center();

  MovieFrameCustomHandler::handleEventsFor(this);
  IMenuHandler::handleEventsFor(&canvas);
  IMenuHandler::handleEventsFor(&systemMenu);
  ICommandHandler::handleEventsFor(this);
  ICommandHandler::handleEventsFor(&canvas);
  IMouseHandler::handleEventsFor(&canvas);
  IFrameHandler::handleEventsFor(this);

  IDMHandler::enableDropOn(&canvas);
  canvas.setItemProvider(this);

  // Default window appearance
  title.setObjectText("QuickFlick");
  title.setViewText("");
  addToWindowList();
  ISize defaultSize = ISize(400, 300);
  moveSizeToClient(IRectangle(defaultSize).centeredAt(imageCenter));
  canvas.moveSizeTo(IRectangle(IPoint(borderWidth(), borderHeight()), defaultSize));
}

Boolean MovieFrame::makePopUpMenu(IMenuEvent& event)
{
  setFocus();
  IPoint mousePosition =
    IWindow::mapPoint(event.mousePosition(), event.controlWindow()->handle(), canvas.handle());
  popUpMenu->IPopUpMenu::show(mousePosition);

  return (true);
}


Boolean MovieFrame::menuShowing(IMenuEvent& event, ISubmenu& submenu)
{
  if (event.menuItemId() == WND_POPUP || event.menuItemId() == ISystemMenu::idPulldown)
  {
    submenu.enableItem(MI_TRUESIZE, !isMinimized() && quickFlick &&
                       QuickFlick::nonEmbeddedDisplay() == QF_NONEMBED_TOFIT &&
                       quickFlick->movieSize().height());
    submenu.enableItem(MI_STRETCHABLE, quickFlick &&
                       QuickFlick::nonEmbeddedDisplay() == QF_NONEMBED_TOFIT &&
                       quickFlick->movieSize().height());
    submenu.checkItem(MI_STRETCHABLE, stretchable);
    submenu.enableItem(MI_GRABBABLE, !movieWindow || movieWindow &&
                       QuickFlick::nonEmbeddedDisplay() != QF_NONEMBED_TOFIT);
    submenu.checkItem(MI_GRABBABLE, grabbable);
    return (true);
  }

  if (event.menuItemId() == MI_WINDOWFORPOPUP)
  {
    submenu.enableItem(SC_RESTORE, (isMinimized() || isMaximized()));
    submenu.enableItem(SC_SIZE, !isMinimized());
    submenu.enableItem(SC_MINIMIZE, !isMinimized());
    submenu.enableItem(SC_MAXIMIZE, !isMaximized());
    return (true);
  }

  return (false);
}


Boolean MovieFrame::mouseClicked(IMouseClickEvent& event)
{
  if (grabbable &&
      event.mouseAction() == IMouseClickEvent::down && event.mouseButton() == IMouseClickEvent::button1)
    sendEvent(WM_TRACKFRAME, TF_MOVE);
  else
    setFocus();
  return true;
}



Boolean MovieFrame::command(ICommandEvent& event)
{
  IFileDialog::Settings *fileDialogSettings;
  IString path;
  unsigned long lastDelimiterIndex;
  ISize trueImageSize;
  IRectangle trueImageSizeRect;

  switch (event.commandId())
  {
    case MI_OPEN:
      fileDialogSettings = new IFileDialog::Settings();
      path = QuickFlick::path();
      path.reverse() >> '\\' >> path;
      path = path.reverse() + "\\*";
      fileDialogSettings->setFileName(path);
      fileDialog_p = new IFileDialog(IWindow::desktopWindow(), this,
                                     *fileDialogSettings, IFileDialog::defaultStyle());
      if (fileDialog_p->pressedOK())
      {
        imageCenter = IWindow::mapPoint(canvas.rect().center(), canvas.handle(), HWND_DESKTOP);
        // Update path property
        IString fileName = fileDialog_p->fileName();
        unsigned long lastDelimiterIndex = fileName.lastIndexOf('\\');
        QuickFlick::setPath(fileName.subString(1, lastDelimiterIndex));
        // Set movie
        setMovie(fileDialog_p->fileName());
      }
      delete fileDialogSettings;
      delete fileDialog_p;
      fileDialog_p = 0;
    break;

    case MI_PROPERTIES:
      QuickFlick::showPropertiesDialog(this);
      break;

    case MI_TRUESIZE:
      trueImageSize = quickFlick->movieSize();
      if (trueImageSize.width() < titleBarSize.width())
        trueImageSize.setHeight(trueImageSize.height() * titleBarSize.width() / trueImageSize.width());
      trueImageSize.setHeight(trueImageSize.height() + EFFECT_CP_HEIGHT);
      trueImageSizeRect = IRectangle(trueImageSize);
      trueImageSizeRect.centerAt(IWindow::mapPoint(canvas.rect().center(), handle(), HWND_DESKTOP));
      moveOntoScreen(trueImageSizeRect);
      moveSizeToClient(trueImageSizeRect);
      break;

    case MI_GRABBABLE:
      grabbable = !grabbable;
      break;

    case MI_STRETCHABLE:
      stretchable = !stretchable;
      break;

    case MI_CLOSE:
      close();
      break;

    default:
      break;
  }
  return (true);
}



Boolean MovieFrame::systemCommand(ICommandEvent& event)
{
  static Boolean closing = false;

  // System commands for the client window (whose events we are also handling)
  if (event.dispatchingWindow() != this)
  {
    if (event.commandId() == ISystemMenu::idClose)
    {
      if (closing) // Quit main message loop when frame notifies client of close
      {
        IThread::current().stopProcessingMsgs();
        return(false);
      }
      else
        closing = true;
    }
    sendEvent(event);
    return (true);
  }

  if (event.commandId() == ISystemMenu::idMinimize)
  {
    if (!isMaximized())
      clientRestoreRect = IRectangle(IWindow::mapPoint(canvas.rect().bottomLeft(), handle(), HWND_DESKTOP),
                                     canvas.size());
//    if (quickFlick)
//      setIcon(IPointerHandle());
  }
  else if (event.commandId() == ISystemMenu::idMaximize)
  {
    if (!isMinimized())
      clientRestoreRect = IRectangle(IWindow::mapPoint(canvas.rect().bottomLeft(), handle(), HWND_DESKTOP),
                                     canvas.size());
//    setIcon(QF_ID);
  }
//  else if (event.commandId() == ISystemMenu::idRestore)
//    setIcon(QF_ID);

  return (false);
}


Boolean MovieFrame::minMax(MinMaxEvent& event)
{
  if (event.controlWindow() == this)
  {
    if ((event.flags() & SWP_MAXIMIZE) && !stretchable)
    {
      ISize canvasSize = (isMinimized() ? clientRestoreRect.size() : canvas.size());
      canvasSize.setHeight(canvasSize.height() - EFFECT_CP_HEIGHT);
      if (canvasSize.height() < 0)
        canvasSize.setHeight(0);
      IRectangle maxRect(ISize(screenSize.width(), screenSize.height() - titleBarSize.height() - EFFECT_CP_HEIGHT));
      long impliedWidth = canvasSize.width() * maxRect.height()
                          / canvasSize.height();
      long impliedHeight = canvasSize.height() * maxRect.width()
                           / canvasSize.width();
      if (impliedWidth > maxRect.width())
        maxRect.sizeTo(ISize(maxRect.width(), impliedHeight));
      else
        maxRect.sizeTo(ISize(impliedWidth, maxRect.height()));
      maxRect.sizeTo(ISize(maxRect.width(), maxRect.height() + EFFECT_CP_HEIGHT));
      if (isMinimized())
      {
        IFrameWindow tempWindow;
        maxRect = tempWindow.frameRectFor(maxRect);
      }
      else
        maxRect = frameRectFor(maxRect);
      maxRect.centerAt(IWindow::desktopWindow()->rect().center());
      event.setRectangle(maxRect);
    }
    else if (event.flags() & SWP_RESTORE)
    {
      IFrameWindow tempWindow;
      IRectangle restRect = tempWindow.frameRectFor(clientRestoreRect);
      event.setRectangle(restRect);
    }
  }

  return (false);
}

MovieFrame::closed(IFrameEvent& event)
{
  hide();
  return (false);
}

void MovieFrame::reset()
{
  ISize imageSize = quickFlick->movieSize();

  /* Scale to fit on screen */
  if (imageSize.width() > usableScreenSize.width())
  {
    imageSize.setHeight(imageSize.height() * usableScreenSize.width() / imageSize.width());
    imageSize.setWidth(usableScreenSize.width());
  }
  else if (imageSize.width() < titleBarSize.width())
  {
    imageSize.setHeight(imageSize.height() * titleBarSize.width() / imageSize.width());
    imageSize.setWidth(titleBarSize.width());
  }

  if (imageSize.height() > usableScreenSize.height())
  {
    imageSize.setWidth(imageSize.width() * usableScreenSize.height() / imageSize.height());
    imageSize.setHeight(usableScreenSize.height());
  }

  imageSize.setHeight(imageSize.height() + EFFECT_CP_HEIGHT);

  /* Adjust position to fit on screen */
  IRectangle newRect = IRectangle(imageSize).centeredAt(imageCenter);
  moveOntoScreen(newRect);

  /* Position window */
  if (isMinimized())
  {
    clientRestoreRect = newRect;
    canvas.sizeTo(minimizeRect().size());
  }
  else
    if (isMaximized())
    {
      clientRestoreRect = newRect;
      restore();
    }
    else
    {
      moveSizeToClient(newRect);
      canvas.moveSizeTo(IRectangle(IPoint(borderWidth(), borderHeight()), imageSize));
    }

  showWindow();
  // refresh(IWindow::paintAll);
}



void MovieFrame::moveOntoScreen(IRectangle& rect)
{
  long hang;

  hang = screenSize.width() - (long)borderWidth() - HORIZ_MARGIN - rect.maxX();
  if (hang < 0)
    rect.moveBy(IPair(hang, 0));
  else
  {
    hang = (long)borderWidth() + HORIZ_MARGIN - rect.minX();
    if (hang > 0)
      rect.moveBy(IPair(hang, 0));
  }

  hang = screenSize.height() - (long)borderHeight() - VERT_MARGIN -
         titleBarSize.height() - rect.maxY();
  if (hang < 0)
    rect.moveBy(IPair(0, hang));
  else
  {
    hang = (long)borderHeight() + VERT_MARGIN - rect.minY();
    if (hang > 0)
      rect.moveBy(IPair(0,hang));
  }
}

Boolean MovieFrame::track(TrackEvent& event)
{
  TRACKINFO trackInfo, copyOfTrackInfo;
  BOOL rc;
  ISize workSize;
  IRectangle minClientRect, newClientRect;
  long impliedWidth, impliedHeight;
  IPoint absoluteCanvasPosition;

  if ((event.flags() & TF_MOVE) == TF_MOVE)
    return (false);

/* Allow the user to draw the tracking rectangle */
  rc = (BOOL)sendEvent(WM_QUERYTRACKINFO, event.flags(), &trackInfo);
  if (!rc)
    return (true);

  if (!stretchable)
  {
    workSize = canvas.size();
    workSize.setHeight(workSize.height() - EFFECT_CP_HEIGHT);
    if (workSize.height() < 0)
      workSize.setHeight(0);
    minClientRect = IRectangle(IPoint(), ISize(trackInfo.ptlMinTrackSize.x - 2 * borderWidth(), 0));
    minClientRect.sizeTo(ISize(minClientRect.width(),workSize.height() *
                               minClientRect.width() / workSize.width() + EFFECT_CP_HEIGHT));
    trackInfo.ptlMinTrackSize.x = frameRectFor(minClientRect).size().width();
    trackInfo.ptlMinTrackSize.y = frameRectFor(minClientRect).size().height();
  }

  rc = WinTrackRect(HWND_DESKTOP, NULLHANDLE, &trackInfo);
  if (!rc)
    return (true);

  copyOfTrackInfo = trackInfo;
  rc = (BOOL)sendEvent(WM_QUERYTRACKINFO, TF_VALIDATETRACKRECT, &copyOfTrackInfo);
  if (!rc)
    return (true);

  if (!stretchable)
  {
    workSize = canvas.size();
    workSize.setHeight(workSize.height() - EFFECT_CP_HEIGHT);
    if (workSize.height() < 0)
      workSize.setHeight(0);
    newClientRect = clientRectFor(trackInfo.rclTrack);
    impliedWidth = workSize.width() * newClientRect.height() / workSize.height();
    impliedHeight = workSize.height() * newClientRect.width() / workSize.width();

    if (impliedWidth > newClientRect.width())
      newClientRect.sizeTo(ISize(newClientRect.width(), impliedHeight + EFFECT_CP_HEIGHT));
    else
      newClientRect.sizeTo(ISize(impliedWidth, newClientRect.height() + EFFECT_CP_HEIGHT));

    absoluteCanvasPosition = IWindow::mapPoint(canvas.position(), handle(), HWND_DESKTOP);
    if (event.flags() & TF_LEFT)
      newClientRect.moveTo(IPoint(absoluteCanvasPosition.x() + canvas.size().width()
                                  - newClientRect.width(), newClientRect.minY()));
    if (event.flags() & TF_BOTTOM)
      newClientRect.moveTo(IPoint(newClientRect.minX(), absoluteCanvasPosition.y() +
                                  canvas.size().height() - newClientRect.height()));
    moveSizeToClient(newClientRect);
  }
  else
    moveSizeTo(trackInfo.rclTrack);

  setFocus();

  return (true);
}

void MovieFrame::showErrorMessage()
{
    IMessageBox messageBox(this);
    messageBox.setTitle("QuickFlick Error");
//    IString message = IApplication::current().userResourceLibrary().loadString(diveImage_p.errorCode())
//                      + "\n\nrc = " + IString(diveImage_p.returnCode());
    messageBox.show(IString(), IMessageBox::catastrophic);
//    if (diveImage_p.errorCode() && diveImage_p.catastrophic())
//      close();
//    diveImage_p.resetErrorCode();
}


void MovieFrame::showInfoMessage()
{
}


Boolean MovieFrame::setMovie(IEvent& event)
{
  setMovie(newFileName);
  return true;
}


void MovieFrame::setMovie(IString fileName)
{
  if (!validFile(fileName))
    return;

  logo = 0;
  movieWindow = 0;
  delete quickFlick; quickFlick = 0;

  grabbable = stretchable = true;

  quickFlick = new QuickFlick();
  quickFlick->setScale(QF_SCALE_DEFAULT);
  quickFlick->setWindow(canvas.handle());
  quickFlick->createController();
  quickFlick->setSetMovieWindowHandleAddress(setMovieWindowHandle);
  quickFlick->setMovie(fileName);
}

void MovieFrame::showWindow()
{
  grabbable = stretchable = !movieWindow || QuickFlick::nonEmbeddedDisplay() != QF_NONEMBED_TOFIT;

  setFocus();
  show();
}

Boolean MovieFrame::validFile(IString fileName)
{
  MMIOINFO headerFileInfo = {0};
  headerFileInfo.fccIOProc = FOURCC_DOS;
  HMMIO headerFile;
  LONG returnCode;

  headerFile = mmioOpen(fileName, &headerFileInfo, headerFileInfo.ulFlags);

  // Attempt to read the QuickTime header
  MovieAtom movieAtom;
  ULONG movieSize;
  returnCode = loadMovie(movieAtom, movieSize, headerFile);

  // Close this view of the movie
  mmioClose(headerFile, 0);

  // Return true if it's a supported QuickTime movie
  if (returnCode == MMIO_SUCCESS && (movieAtom.isMusicOnly() || movieAtom.isSoundOnly() || movieAtom.hasVideo()))
    return (true);

  return (false);
}

Boolean MovieFrame::provideEnterSupport(IDMTargetEnterEvent& event)
{
  MMIOINFO headerFileInfo = {0};
  headerFileInfo.fccIOProc = FOURCC_DOS;
  HMMIO headerFile;
  LONG returnCode;

  IDMTargetOperation::Handle targetOp = IDMTargetOperation::targetOperation();

  if (targetOp->numberOfItems() == 1)
  {
    IDMItem::Handle targetItem = targetOp->item(1);

    if (validFile(targetItem->containerName() + targetItem->sourceName()))
      return (true);
  }

  event.setDropIndicator(IDM::neverOk);
  return (false);
}

Boolean MovieFrame::provideLeaveSupport(IDMTargetLeaveEvent& event)
{
  return (true);
}


MovieFrame::~MovieFrame()
{
//  delete movieWindow;
//  delete logo;
  delete quickFlick;
  delete popUpMenu;
  delete fileDialog_p;
}

