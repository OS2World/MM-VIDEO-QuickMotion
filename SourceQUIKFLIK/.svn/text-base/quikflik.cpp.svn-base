#include "quikflik.hpp"
#include "qfmotion.hpp"
#include "qfworld.hpp"
#include "qfcontrol.hpp"
#include "propdiag.hpp"
#include "moov.hpp"

#include <ithread.hpp>
#include <ireslib.hpp>
#include <icolor.hpp>
#include <ibmpctl.hpp>
#include <immexcpt.hpp>
#include <idate.hpp>
#include <ifont.hpp>
#include <string.h>

#ifdef TRACE
#include <fstream.h>
ofstream *debugOutput;
#endif

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
HMODULE moduleHandle;
IThread *serviceThread;
HEV serviceThreadStart;
Boolean serviceThreadStarted;

IResourceLibrary *QuickFlick::resourceLibrary_p;
IObjectWindow *QuickFlick::serviceThreadControlWindow_p;
IProfile QuickFlick::profile = IProfile(IProfile::userProfile());
IString QuickFlick::saveAsPath_p = IString();
long QuickFlick::nonEmbeddedDisplay_p = 0;
Boolean QuickFlick::cueDuringPrepare_p = false;
IKeySortedSet<ImageTypeEntry, IString> QuickFlick::imageTypes;

/************************************************************************/
/* Helper functions                                                     */
/************************************************************************/
unsigned long now()
{
  DATETIME now;
  DosGetDateTime(&now);
  return (((now.day * 24 + now.hours) * 60 + now.minutes) * 60 + now.seconds) * 100 + now.hundredths;
}

#ifdef TRACE
void stampTime()
{
  DATETIME now;
  DosGetDateTime(&now);
  *debugOutput << (USHORT)(now.hours) << ':' << (USHORT)(now.minutes) << ':' <<
                  (USHORT)(now.seconds) << '.' << (USHORT)(now.hundredths) << '\t';
  *debugOutput << IThread::current().id() << '\t';
}
#endif

/************************************************************************/
/* IMAGE TYPES TABLE KEY FUNCTION                                       */
/************************************************************************/
IString const& key(ImageTypeEntry const& element)
{
  return element.imageType;
}

/************************************************************************/
/* ServiceHandler                                                       */
/************************************************************************/

class ServiceHandler : public IHandler
{
public:
  virtual Boolean dispatchHandlerEvent(IEvent &event)
  {
    switch (event.eventId())
    {
      case WM_CUSTOM_CREATE:
        return create(event);
      case WM_CUSTOM_DESTROY:
        return destroy(event);
      default:
        return (false);
    }
  }
  Boolean create(IEvent &event)
  {
    IObjectWindow *serviceThreadWindow_p = new IObjectWindow();
    event.setResult(serviceThreadWindow_p);
    return true;
  }
  Boolean destroy(IEvent &event)
  {
    IObjectWindow *serviceThreadWindow_p = (IObjectWindow*)(event.parameter1().asLong());
    delete serviceThreadWindow_p;
    return true;
  }
};

/************************************************************************/
/* service:                                                             */
/************************************************************************/
void _Optlink service(void *p)
{
#ifdef TRACE
  stampTime();
  *debugOutput << "Service thread started:" << endl;
#endif

//  IThread::current().setPriority(IApplication::timeCritical, 15);

  IObjectWindow *serviceThreadControlWindow = new IObjectWindow;
  ServiceHandler serviceHandler;

  *(IObjectWindow**)p = serviceThreadControlWindow;
  serviceHandler.handleEventsFor(serviceThreadControlWindow);
  serviceThreadStarted = true;
  DosPostEventSem(serviceThreadStart);

  while (true)
  {
    try
    {
      IThread::current().processMsgs();
      break;
    }
    catch (IException exception)
    {
#ifdef TRACE
  stampTime();
  *debugOutput << "Service thread EXCEPTION: " << exception.errorId() << ' ' << exception.text() << endl;
#endif
    }
  }

  serviceHandler.stopHandlingEventsFor(serviceThreadControlWindow);
  delete serviceThreadControlWindow;

#ifdef TRACE
  stampTime();
  *debugOutput << "Service thread ended:" << endl;
#endif
}

/************************************************************************/
/* exitFunction                                                         */
/************************************************************************/
static void _System exitFunction(ULONG reason)
{
#ifdef TRACE
  stampTime();
  *debugOutput << "QuickFlick unloaded:" << endl;
  debugOutput->close();
#endif

  __ctordtorTerm();
  DosExitList(EXLST_EXIT, exitFunction);
}

/************************************************************************/
/* _DLL_InitTerm:                                                       */
/************************************************************************/
ULONG _System _DLL_InitTerm (ULONG hModHandle, ULONG fTerm)
{
  ULONG rc;

  moduleHandle = hModHandle;

  if (!fTerm)
  {
    DosExitList(0x0000FF00 | EXLST_ADD, exitFunction);
    __ctordtorInit();

#ifdef TRACE
  debugOutput = new ofstream("quikflik.msg", ios::out | ios::trunc);
  stampTime();
  *debugOutput << "QuickFlick loaded:" << endl;
#endif
  }

  return (1L);  /* Success */
}

/************************************************************************/
/* CONSTRUCTOR                                                          */
/************************************************************************/
QuickFlick::QuickFlick()
{
   // Initialize instance variables
   getHeaderThread = 0;
   disabled_p = true;
   streamStopped_p = streamDone_p = false;
   fileName_p = saveAsFileName = IString();
   loop_p = autoplay_p = false;

   scale_p = QF_SCALE_DEFAULT;
   window_p = 0;
   createdWin = false;
   setMovieWindowHandle_p = 0;

   // Sample enabling condition: if (IDate::today() < IDate(IDate::March, 15, 1998))
   // Next statement unconditionally enables for now
   disabled_p = false;

   // Initialize user interface components
   player_p = 0;
   controller_p = 0;
   movieAtom_p = 0;
   movieFile_p = 0;
   saveAsDialog = 0;
   saveAsDialogSettings.setSaveAsDialog();
   saveAsDialogSettings.setTitle("Save As");
   captureFrameSettings.setSaveAsDialog();
   captureFrameSettings.setTitle("Save Frame As");
   captureFrameSettings.setFileName(saveAsPath_p);
   IKeySortedSet<ImageTypeEntry, IString>::Cursor* cursor = imageTypes.newCursor();
   for (cursor->setToFirst(); cursor->isValid(); cursor->setToNext())
     captureFrameSettings.addFileType(imageTypes.elementAt(*cursor).imageType);
   delete cursor;
   captureFrameSettings.setInitialFileType(imageTypes.elementAtPosition(1).imageType);

   // Make sure service thread is started
   if (!serviceThreadStarted)
    DosWaitEventSem(serviceThreadStart, SEM_INDEFINITE_WAIT);
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
void QuickFlick::initialize()
{
  resourceLibrary_p = new IDynamicLinkLibrary(moduleHandle);
  PropertiesDialog::resourceLibrary = resourceLibrary_p;
  QFController::resourceLibrary = resourceLibrary_p;
  QFController::playBitmapHandle = IGBitmap(IResourceId(PLAY_BITMAP_ID, *QFController::resourceLibrary)).handle();
  QFController::pauseBitmapHandle = IGBitmap(IResourceId(PAUSE_BITMAP_ID, *QFController::resourceLibrary)).handle();

  serviceThreadStarted = false;
  DosCreateEventSem(NULL, &serviceThreadStart, 0, FALSE);
  serviceThread = new IThread(service, &QuickFlick::serviceThreadControlWindow_p);

  // Properties
  profile.setDefaultApplicationName("QuickFlick");
  try {saveAsPath_p = profile.elementWithKey("saveAsPath");} catch (...) {};
  try {nonEmbeddedDisplay_p = profile.integerWithKey("nonEmbeddedDisplay");} catch (...) {};
  try {cueDuringPrepare_p = profile.integerWithKey("cueDuringPrepare");} catch (...) {};

  QFMotionPlayer::initialize();
  QFWorldPlayer::initialize();

  // Get list of writable image ioProcs for frame capture
  CHAR          szBuffer[256];
  LONG          lNumIOProcs = 0;
  MMFORMATINFO  mmFormatInfo = {0};
  PMMFORMATINFO pmmFormatInfoArray;
  ULONG         ulReturnCode;
  LONG          lFormatsRead;
  LONG          index;
  LONG          lBytesRead;

  mmFormatInfo.ulMediaType |= MMIO_MEDIATYPE_IMAGE;
  ulReturnCode = mmioQueryFormatCount(&mmFormatInfo, &lNumIOProcs, 0, 0);
  if (lNumIOProcs)
  {
    pmmFormatInfoArray = new MMFORMATINFO[lNumIOProcs];
    ulReturnCode = mmioGetFormats(&mmFormatInfo, lNumIOProcs, pmmFormatInfoArray, &lFormatsRead, 0, 0);
    if(ulReturnCode == MMIO_SUCCESS)
    {
      for (index = 0; index < lFormatsRead; index++)
      {
        if (pmmFormatInfoArray[index].ulFlags & MMIO_CANWRITETRANSLATED)
        {
          ImageTypeEntry entry;
          mmioGetFormatName(&pmmFormatInfoArray[index], szBuffer, &lBytesRead, 0L, 0L);
          *(szBuffer + lBytesRead) = '\0';
          entry.imageType = IString(szBuffer)
                          + IString(" (.") + IString(pmmFormatInfoArray[index].szDefaultFormatExt) + ")";
          entry.ioProc = pmmFormatInfoArray[index].fccIOProc;
          imageTypes.add(entry);
        }
      }
    }
    delete pmmFormatInfoArray;
  }
}

/************************************************************************/
/* SET LOOP                                                             */
/************************************************************************/
void QuickFlick::setLoop(Boolean loop)
{
  loop_p = loop;
}

/************************************************************************/
/* WILL LOOP                                                            */
/************************************************************************/
Boolean QuickFlick::willLoop()
{
  return loop_p;
}

/************************************************************************/
/* SET AUTOPLAY                                                         */
/************************************************************************/
void QuickFlick::setAutoplay(Boolean autoplay)
{
  autoplay_p = autoplay;
}

/************************************************************************/
/* WILL AUTOPLAY                                                        */
/************************************************************************/
Boolean QuickFlick::willAutoplay()
{
  return autoplay_p;
}

/************************************************************************/
/* SET SCALE                                                            */
/************************************************************************/
void QuickFlick::setScale(float scale)
{
  scale_p = (scale == QF_SCALE_DEFAULT ? -nonEmbeddedDisplay_p : scale);
}

/************************************************************************/
/* SCALE                                                                */
/************************************************************************/
float QuickFlick::scale()
{
  return scale_p;
}

/************************************************************************/
/* SET MOVIE                                                            */
/************************************************************************/
void QuickFlick::setMovie(IString fileName)
{
  newFileName = fileName;
  fileName_p = IString();
  getHeaderThread = new IThread(new IThreadMemberFn<QuickFlick>(*this, QuickFlick::getHeader), false);
}

/************************************************************************/
/* FILE NAME                                                            */
/************************************************************************/
IString QuickFlick::fileName()
{
  return fileName_p;
}

/************************************************************************/
/* PLAYER                                                               */
/************************************************************************/
QFPlayer* QuickFlick::player()
{
  return player_p;
}

/************************************************************************/
/* PLAYER CREATED                                                       */
/************************************************************************/
void QuickFlick::playerCreated()
{
 // Base class implementation does nothing
}

/************************************************************************/
/* DISABLED                                                             */
/************************************************************************/
Boolean QuickFlick::disabled()
{
  return disabled_p;
}

/************************************************************************/
/* SET STREAM STOPPED                                                   */
/************************************************************************/
void QuickFlick::setStreamStopped(Boolean streamStopped)
{
  streamStopped_p = streamStopped;
}

/************************************************************************/
/* STREAM STOPPED                                                       */
/************************************************************************/
Boolean QuickFlick::streamStopped()
{
  return streamStopped_p;
}

/************************************************************************/
/* SET STREAM DONE                                                      */
/************************************************************************/
void QuickFlick::setStreamDone(Boolean streamDone)
{
  streamDone_p = streamDone;
}

/************************************************************************/
/* STREAM DONE                                                          */
/************************************************************************/
Boolean QuickFlick::streamDone()
{
  return streamDone_p;
}

/************************************************************************/
/* MAIN THREAD WINDOW                                                   */
/************************************************************************/
IObjectWindow& QuickFlick::mainThreadWindow()
{
  return mainThreadWindow_p;
}

/************************************************************************/
/* CREATE CONTROLLER                                                    */
/************************************************************************/
void QuickFlick::createController(float armPercent)
{
  QFController::State state = QFController::disabled;
  float fullPercent = .0;

  if (controller_p)
  {
    state = controller_p->state();
    fullPercent = controller_p->fullPercent();
  }

  delete controller_p;
  controller_p = new QFController(window_p, state, armPercent, fullPercent);
}

/************************************************************************/
/* CONTROLLER                                                           */
/************************************************************************/
QFController* QuickFlick::controller()
{
  return controller_p;
}

/************************************************************************/
/* DELETE CONTROLLER                                                    */
/************************************************************************/
void QuickFlick::deleteController()
{
  if (controller_p)
  {
    delete controller_p;
    controller_p = 0;
  }
}

/************************************************************************/
/* MOVIE ATOM                                                           */
/************************************************************************/
MovieAtom* QuickFlick::movieAtom()
{
  return movieAtom_p;
}

/************************************************************************/
/* MOVIE FILE                                                           */
/************************************************************************/
HMMIO QuickFlick::movieFile()
{
  return movieFile_p;
}

/************************************************************************/
/* SET WINDOW                                                           */
/************************************************************************/
void QuickFlick::setWindow(IWindowHandle windowHandle)
{
  if (windowHandle.isValid())
  {
#ifdef TRACE
  stampTime();
  *debugOutput << "->Set window:" << '\t' << this << '\t' << windowHandle.asString().d2x() << '\t' << endl;
#endif
    window_p = IWindow::windowWithHandle(windowHandle);
    if (!window_p)
    {
      window_p = new IWindow(windowHandle);
      createdWin = true;
    }
  }
}

/************************************************************************/
/* WINDOW                                                               */
/************************************************************************/
IWindow* QuickFlick::window()
{
  return window_p;
}

/************************************************************************/
/* SET SET MOVIE WINDOW HANDLE ADDRESS                                  */
/************************************************************************/
void QuickFlick::setSetMovieWindowHandleAddress(void (*setMovieWindowHandle)(IWindowHandle movieWindowHandle))
{
  setMovieWindowHandle_p = setMovieWindowHandle;
}

/************************************************************************/
/* SET MOVIE WINDOW HANDLE ADDRESS                                      */
/************************************************************************/
void (*QuickFlick::setMovieWindowHandleAddress())(IWindowHandle movieWindowHandle)
{
  return setMovieWindowHandle_p;
}

/************************************************************************/
/* LOGO WINDOW                                                          */
/************************************************************************/
IWindowHandle QuickFlick::logoWindowHandle()
{
  if (controller_p)
    return controller_p->logoWindowHandle();
  else
    return IWindowHandle();
}

/************************************************************************/
/* MOVIE SIZE                                                           */
/************************************************************************/
ISize QuickFlick::movieSize()
{
  if (player_p)
    return player_p->movieSize();
  else
    return ISize();
}

/************************************************************************/
/* PATH                                                                 */
/************************************************************************/
IString QuickFlick::path()
{
  return saveAsPath_p;
}

/************************************************************************/
/* SET PATH                                                             */
/************************************************************************/
void QuickFlick::setPath(char* path)
{
  saveAsPath_p = path;
  profile.addOrReplaceElementWithKey("saveAsPath", path);
}

/************************************************************************/
/* NON-EMBEDDED DISPLAY                                                 */
/************************************************************************/
long QuickFlick::nonEmbeddedDisplay()
{
  return nonEmbeddedDisplay_p;
}

/************************************************************************/
/* CUE DURING PREPARE                                                   */
/************************************************************************/
Boolean QuickFlick::cueDuringPrepare()
{
  return cueDuringPrepare_p;
}

/************************************************************************/
/* GET HEADER                                                           */
/************************************************************************/
void QuickFlick::getHeader()
{
#ifdef TRACE
  stampTime();
  *debugOutput << "Get header:" << '\t' << this << endl;
#endif

  // Open the movie through the plugin ioproc
  MMIOINFO movieFileInfo = {0};
  if (mmioDetermineSSIOProc(newFileName, NULL, &movieFileInfo.fccIOProc, NULL))
    movieFileInfo.fccIOProc = FOURCC_DOS;
  if (movieFileInfo.fccIOProc != mmioFOURCC('N', 'S', 'C', 'P'))
    streamDone_p = true;
  movieFile_p = mmioOpen(newFileName, &movieFileInfo, movieFileInfo.ulFlags);

  // Attempt to read the QuickTime header
  movieAtom_p = new MovieAtom();
  ULONG movieBytes;
  LONG returnCode;
  returnCode = loadMovie(*movieAtom_p, movieBytes, movieFile_p);

  // Set up player, if it's a supported QuickTime movie
  if (returnCode == MMIO_SUCCESS)
  {
    /* Virtual Reality */
    if (movieAtom_p->isWorld())
      player_p = new QFWorldPlayer(*this);
    else
    /* Regular Movie */
    {
      Boolean isMusicOnly = movieAtom_p->isMusicOnly();
      Boolean isSoundOnly = movieAtom_p->isSoundOnly();
      if (isMusicOnly || isSoundOnly || movieAtom_p->hasVideo())
      {
        // Close this view of the movie
        mmioClose(movieFile_p, 0);
        player_p = new QFMotionPlayer(*this, isMusicOnly, isSoundOnly);
      }
    }
    if (player_p)
    {
      fileName_p = newFileName;
      unsigned long lastDelimiterIndex = fileName_p.lastIndexOfAnyOf("\\+");
      saveAsFileName=saveAsPath_p+fileName_p.subString(lastDelimiterIndex+1,fileName_p.length()-lastDelimiterIndex);
      saveAsDialogSettings.setFileName(saveAsFileName);
      mainThreadWindow_p.postEvent(WM_CUSTOM_GOT_HEADER);
      return;
    }
  }

  setStreamStopped();
}

/************************************************************************/
/* RESOURCE LIBRARY                                                     */
/************************************************************************/
IResourceLibrary& QuickFlick::resourceLibrary()
{
  return *resourceLibrary_p;
}

/************************************************************************/
/* SERVICE THREAD CONTROL WINDOW                                        */
/************************************************************************/
IObjectWindow* QuickFlick::serviceThreadControlWindow()
{
  return serviceThreadControlWindow_p;
}

/************************************************************************/
/* SHOW PROPERTIES DIALOG                                               */
/************************************************************************/
void QuickFlick::showPropertiesDialog(IWindow* owner)
{
  PropertiesDialog* propertiesDialog = new PropertiesDialog(owner, profile);
  propertiesDialog->setDestroyOnClose(false);
  propertiesDialog->showModally();
  if (propertiesDialog->result() == DID_OK)
  {
    try {saveAsPath_p = profile.elementWithKey("saveAsPath");} catch (...) {};
    try {nonEmbeddedDisplay_p = profile.integerWithKey("nonEmbeddedDisplay");} catch (...) {};
    try {cueDuringPrepare_p = profile.integerWithKey("cueDuringPrepare");} catch (...) {};
  }
  delete propertiesDialog;
  propertiesDialog = 0;
}

/************************************************************************/
/* SHOW SAVE AS DIALOG                                                  */
/************************************************************************/
IString QuickFlick::showSaveAsDialog(IWindow* owner)
{
  if (saveAsDialog)
    delete saveAsDialog;

  saveAsDialog = new IFileDialog(0, owner, saveAsDialogSettings);

  if (saveAsDialog->pressedOK())
  {
#ifdef TRACE
  stampTime();
  *debugOutput << "Save:" << '\t' << this << '\t' << saveAsDialog->fileName() << endl;
#endif

    saveAsFileName = saveAsDialog->fileName();

    saveAsDialogSettings.setFileName(saveAsFileName); // In case user saves the same movie again

    // Update saveAsPath_p property
    unsigned long lastDelimiterIndex = saveAsFileName.lastIndexOf('\\');
    saveAsPath_p = saveAsFileName.subString(1, lastDelimiterIndex);
    profile.addOrReplaceElementWithKey("saveAsPath", saveAsPath_p);

    return saveAsFileName;
  }
  else
    return IString();
}

/************************************************************************/
/* SHOW CAPTURE FRAME DIALOG                                            */
/************************************************************************/
IString QuickFlick::showCaptureFrameDialog(IWindow* owner, FOURCC& ioProc)
{
  if (saveAsDialog)
    delete saveAsDialog;

  ioProc = mmioFOURCC(' ', ' ', ' ', ' ');

  saveAsDialog = new IFileDialog(0, owner, captureFrameSettings, IFileDialog::filter);

  if (saveAsDialog->pressedOK())
  {
#ifdef TRACE
  stampTime();
  *debugOutput << "Save:" << '\t' << this << '\t' << saveAsDialog->fileName() << endl;
#endif

    saveAsFileName = saveAsDialog->fileName();

    captureFrameSettings.setFileName(saveAsFileName); // In case user captures another frame

    IString saveAsEAType = saveAsDialog->saveAsEAType();
    if (saveAsEAType.length())
    {
      ioProc = imageTypes.elementWithKey(saveAsEAType).ioProc;
      if (!saveAsFileName.indexOf('.'))
      {
        unsigned long dotPos = saveAsEAType.lastIndexOf('.');
        saveAsFileName += saveAsEAType.subString(dotPos, saveAsEAType.lastIndexOf(')') - dotPos);
      }
    }
    return saveAsFileName;
  }
  else
    return IString();
}

/************************************************************************/
/* HOT SPOT URL                                                         */
/************************************************************************/
IString QuickFlick::hotSpotURL(long hotSpotID)
{
  return IString();
}

/************************************************************************/
/* LINK TO                                                              */
/************************************************************************/
void QuickFlick::linkTo(IString url)
{
}

/************************************************************************/
/* STATUS                                                               */
/************************************************************************/
void QuickFlick::status(IString text)
{
}

/************************************************************************/
/* DESTRUCTOR                                                           */
/************************************************************************/
QuickFlick::~QuickFlick()
{
  delete player_p;
  delete controller_p;
  delete movieAtom_p;
  delete saveAsDialog;

  if (window_p && createdWin)
    delete window_p;

  if (getHeaderThread && getHeaderThread->isStarted())
    IThread::current().waitFor(*getHeaderThread);
}

/************************************************************************/
/* SHUTDOWN                                                             */
/************************************************************************/
void QuickFlick::shutdown()
{
  serviceThread->stopProcessingMsgs();
  DosCloseEventSem(serviceThreadStart);
  IThread::current().waitFor(*serviceThread);
  delete serviceThread;

  delete QuickFlick::resourceLibrary_p;

  QFMotionPlayer::shutdown();
}

