#include "npqkfl.h"
#include "qfplugin.hpp"
#include "qfcontrol.hpp"
#include "qfplayer.hpp"

#include <igraphbt.hpp>
#include <islider.hpp>

#ifdef TRACE_PLUGIN
#include <fstream.h>
extern ofstream *debugOutput;
extern void stampTime();
#endif

/************************************************************************/
/* Constants                                                            */
/************************************************************************/
const int32 STREAMBUFSIZE = 0X0FFFFFFF;   // Accept a really big buffer

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
Boolean QuickFlickPlugin::mainThreadPriorityBoosted = false;
QuickFlickPlugin *QuickFlickPlugin::hangingInstance = 0;

/************************************************************************/
/* Helper functions                                                     */
/************************************************************************/
unsigned long now()
{
  DATETIME now;
  DosGetDateTime(&now);
  return (((now.day * 24 + now.hours) * 60 + now.minutes) * 60 + now.seconds) * 100 + now.hundredths;
}

// Our reference-counted window for properly destructing the viewport C++ object
RefCountedWindow::RefCountedWindow(const IWindowHandle& handle)
                : IWindow(handle)
{
}

/************************************************************************/
/* HOT SPOT TABLE KEY FUNCTION                                          */
/************************************************************************/
long const& key(HotSpotEntry const& element)
{
  return element.hotSpotID;
}

/************************************************************************/
/* NETSCAPE CONVERSATION CONSTRUCTOR                                    */
/************************************************************************/
NetscapeConversation::NetscapeConversation(IWindowHandle windowHandle)
                    : IDDEClientConversation(false)
{
}

/************************************************************************/
/* NETSCAPE CONVERSATION HANDLE INITIATE ACK                            */
/************************************************************************/
void NetscapeConversation::handleInitiateAck(const IEvent& initiateAckEvent)
{
  PID pid;
  TID tid;

  //Dont accept conversation from any other instance of Netscape
  WinQueryWindowProcess(initiateAckEvent.parameter1().asUnsignedLong(), &pid, &tid);
  if (pid != IApplication::currentPID())
  {
    PDDEINIT pddei = (_DDEINIT*)(void*)(initiateAckEvent.parameter2());
    *pddei->pszAppName = '\0';
  }
  IDDEClientConversation::handleInitiateAck(initiateAckEvent);
}

/************************************************************************/
/* NETSCAPE CONVERSATION DATA FUNCTION                                  */
/************************************************************************/
Boolean NetscapeConversation::data(IDDEDataEvent& event)
{
  return true;
}

/************************************************************************/
/* NETSCAPE CONVERSATION DESTRUCTOR                                     */
/************************************************************************/
NetscapeConversation::~NetscapeConversation()
{
}

/************************************************************************/
/* PAINT PLUGIN WINDOW                                                  */
/************************************************************************/
Boolean PluginWindowPaintHandler::paintWindow(IPaintEvent& event)
{
  event.clearBackground(IColor(IColor::paleGray));
  return true;
}

/************************************************************************/
/* CONSTRUCTOR                                                          */
/************************************************************************/
QuickFlickPlugin::QuickFlickPlugin(NPMIMEType pluginType, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved, NPP instance)
{
   LONG returnCode;

   pluginIOInfo.currentSize = 0;
   pluginIOInfo.currentPosition = 0;
   pluginIOInfo.movieFileAccess = 0;
   pluginIOInfo.dataReceived = 0;
   pluginIOInfo.waitForData = TRUE;
#ifdef TRACE_PLUGIN
   pluginIOInfo.debugOutput = debugOutput;
#endif
   whenStreamStarted = length_p = 0;
   mimeType = IString(pluginType);
   url_p = volume_p = href_p = target_p = pan_p = tilt_p = fov_p = node_p = correction_p = IString();
   mode_p = mode;
   pSavedInstanceData = saved;
   instance_p = instance;
   controllerTag = true;
   aboutPlugins = false;

   aboutPluginsPanel = 0;
   netscapeConversation = 0;

   setScale((mode == NP_EMBED ? QF_SCALE_TOFIT : QF_SCALE_DEFAULT));

   // Process embed tag parametrs
   Boolean hidden = false;
   int i;
   for (i=0; i < argc; i++)
   {
     IString keyword = IString(argn[i]);
     keyword = keyword.lowerCase();
     IString value = IString(argv[i]);
     IString valueLower = value.lowerCase();
     if (keyword == "hidden")
       hidden = (valueLower != "false");
     else if (keyword == "loop")
       setLoop(valueLower == "true" || valueLower == "palindrome");
     else if (keyword == "autoplay")
       setAutoplay(valueLower == "true");
     else if (keyword == "controller")
       controllerTag = (valueLower != "false");
     else if (keyword == "aboutplugins")
       aboutPlugins = (valueLower == "true");
     else if (keyword.subString(1,7) == "hotspot")
     {
       HotSpotEntry hotSpotEntry;
       hotSpotEntry.hotSpotID = keyword.subString(8, keyword.length() - 7).asInt();
       hotSpotEntry.url = value;
       hotSpots.add(hotSpotEntry);
     }
   }

   if (aboutPlugins)
   {
     if (hangingInstance)
     {
       hangingInstance->aboutPluginsPanel->hide();
       hangingInstance->aboutPluginsPanel->setParent(IWindow::desktopWindow());
     }
     hangingInstance = this;
   }

   if (hidden || aboutPlugins)
     controllerTag = false;

   // Create mutex semaphore for controlling access to private movie file
   returnCode = DosCreateMutexSem(NULL, &pluginIOInfo.movieFileAccess, 0, FALSE);

   // Create event semaphore for signaling arrival of new data to the IOProc read function
   returnCode = DosCreateEventSem(NULL, &pluginIOInfo.dataReceived, 0, FALSE);
}

/************************************************************************/
/* INITIALIZE                                                           */
/************************************************************************/
void QuickFlickPlugin::initialize()
{
  QuickFlick::initialize();
}

/************************************************************************/
/* SHUTDOWN                                                             */
/************************************************************************/
void QuickFlickPlugin::shutdown()
{
/*   if (hangingInstance)
   {
     if (hangingInstance->aboutPluginsPanel)
     {
       hangingInstance->aboutPluginsPanel->setAutoDestroyWindow();
       delete hangingInstance;
     }
     else
     {
       if (hangingInstance->window()->isValid())
         hangingInstance->window()->setAutoDestroyWindow();
       delete hangingInstance;
     }
   } */

  QuickFlick::shutdown();
}

/************************************************************************/
/* HOT SPOT URL                                                         */
/************************************************************************/
IString QuickFlickPlugin::hotSpotURL(long hotSpotID)
{
  IString url;
  try
  {
    url = hotSpots.elementWithKey(hotSpotID).url; //Exception if no such hotspot
    //Expand relative link
    if (!url.indexOf(':'))
    {
      if (url.indexOf('/') == 1)
        url = url_p.subString(1, url_p.indexOf("//")) + url;
      else
      {
        IString base = url_p;
        while (url.subString(1, 3) == "../")
        {
          url.remove(1, 3);
          base = base.subString(1, base.lastIndexOf('/') - 1);
        }
        if (url.subString(1, 2) == "./")
          url.remove(1, 2);
        url = base.subString(1, base.lastIndexOf('/')) + url;
      }
    }
  }
  catch (IException ie)
  {
    url = IString();
  }

  return url;
}

/************************************************************************/
/* LINK TO                                                              */
/************************************************************************/
void QuickFlickPlugin::linkTo(IString url)
{
  if (netscapeConversation->begin("NETSCAPE", "WWW_OpenURL"))
  {
    netscapeConversation->requestData(url + ",,0xFFFFFFFF,0x0");
    netscapeConversation->end();
  }
}

/************************************************************************/
/* FLY OVER HOT SPOT                                                    */
/************************************************************************/
void QuickFlickPlugin::status(IString text)
{
  NPN_Status(instance_p, text);
}

/************************************************************************/
/* PLAYER CREATED                                                       */
/************************************************************************/
void QuickFlickPlugin::playerCreated()
{
  player()->pWaitForData = &pluginIOInfo.waitForData;

  if (window())
  {
    // Handle scroll events for the ViewPort ancestor of the plugin window (currently grandparent)
    HWND parent = WinQueryWindow(window()->handle(), QW_PARENT);
    parent =      WinQueryWindow(parent, QW_PARENT);
    if (parent != IWindow::desktopWindow()->handle())
    {
      RefCountedWindow *tempViewPort = (RefCountedWindow*)(IWindow::windowWithHandle(IWindowHandle(parent)));
      if (tempViewPort)
        viewPort = tempViewPort;
      else
      {
        viewPort = new RefCountedWindow(IWindowHandle(parent));
        viewPort->removeRef(); // Previous statement leaves reference count at 2
      }
      player()->handleScrollingFor(*(RefCountedWindow*)viewPort);
    }
    pluginWindowPaintHandler.stopHandlingEventsFor(window());
  }
}

/************************************************************************/
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                           NETSCAPE API                               */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

////////////////////////////////////////////////////////////////////////////////
// SET WINDOW
////////////////////////////////////////////////////////////////////////////////
NPError QuickFlickPlugin::setWindow(IWindowHandle newWindowHandle)
{
  if (aboutPlugins)
  {
    if (!aboutPluginsPanel && newWindowHandle)
      aboutPluginsPanel = new AboutPluginsPanel(newWindowHandle, this);
  }
  else
    if (!window())
    {
      QuickFlick::setWindow(newWindowHandle);
      if (window())
      {
#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "Set window:" << '\t' << newWindowHandle << endl;
#endif
        netscapeConversation = new NetscapeConversation(newWindowHandle);
        pluginWindowPaintHandler.handleEventsFor(window());
        if (controllerTag && !controller())
        {
          controllerTag = false;
          createController();
        }
      }
    }

  return NPERR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// NEW STREAM
////////////////////////////////////////////////////////////////////////////////
NPError QuickFlickPlugin::newStream(IString url, unsigned long length)
{
//  if (disabled())
//    return NPERR_GENERIC_ERROR;

  url_p = url;
  length_p = length;

  // Open private movie file as a memory file for caching the movie to be played
  MMIOINFO movieFileInfo = {0};
  movieFileInfo.aulInfo[0] = 32768;
  movieFileInfo.cchBuffer = length_p;
  movieFileInfo.fccIOProc = FOURCC_MEM;
  movieFileInfo.ulFlags = MMIO_READWRITE | MMIO_CREATE;
  movieFileInfo.pExtraInfoStruct = &pluginIOInfo;

  movieFile = mmioOpen(NULL, &movieFileInfo, movieFileInfo.ulFlags);

  unsigned long lastDelimiterIndex = url_p.lastIndexOf('/');
  setMovie(IString(movieFile)+IString(".nscp+")+url_p.subString(lastDelimiterIndex+1,url_p.length()-lastDelimiterIndex));

  // Remember when stream started for future bit rate calculation
  whenStreamStarted = now();

  return NPERR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
// WRITE READY
////////////////////////////////////////////////////////////////////////////////
int32 QuickFlickPlugin::writeReady()
{
  return (STREAMBUFSIZE);   // Number of bytes ready to accept in write()
}

////////////////////////////////////////////////////////////////////////////////
// WRITE
////////////////////////////////////////////////////////////////////////////////
int32 QuickFlickPlugin::write(unsigned long length, PCHAR buffer)
{
  if (streamStopped()) // || disabled()
    return -1;

  ULONG position;
  ULONG bytesWritten;
  LONG returnCode;

  // Update private movie file
  returnCode = DosRequestMutexSem(pluginIOInfo.movieFileAccess, SEM_INDEFINITE_WAIT);
  position = mmioSeek(movieFile, pluginIOInfo.currentSize, SEEK_SET);
  bytesWritten = mmioWrite(movieFile, buffer, length);
  pluginIOInfo.currentSize += bytesWritten;
  returnCode = DosReleaseMutexSem(pluginIOInfo.movieFileAccess);

  // Notify observers that data has arrived
  returnCode = DosPostEventSem(pluginIOInfo.dataReceived);

  if (controller())
  {
    float fullPercent = (length_p ? (float)pluginIOInfo.currentSize / length_p : .0);
    controller()->setFullPercent(fullPercent);
  }

// Play the movie now, if there's enough data.
  if (player() && player()->howLongMovieWillPlay && !player()->autoStartPerformed)
  {
    float elapsedTime = 30.0 * (now() - whenStreamStarted);
    float movieBytesWeCouldHandle = pluginIOInfo.currentSize / elapsedTime * player()->howLongMovieWillPlay;
    if (movieBytesWeCouldHandle > length_p) movieBytesWeCouldHandle = length_p;
    unsigned long autoStartBytes = length_p - movieBytesWeCouldHandle;

    player()->autoStartPointReached = pluginIOInfo.currentSize >= autoStartBytes;

    if (player()->filePrepared && player()->autoStartPointReached)
    {
#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "Start issued from write (fast start):" << '\t' << this << endl;
#endif

      player()->start();
    }
    else // Autostart point not reached
    {
      if (controller() && autoStartBytes > 0 && autoStartBytes < length_p)
      {
        float autoStartPoint = (float)autoStartBytes / length_p;
        controller()->setDetentPercent(autoStartPoint);
      }
    }
  }

  return (bytesWritten);
}

////////////////////////////////////////////////////////////////////////////////
// DESTROY STREAM
////////////////////////////////////////////////////////////////////////////////
NPError QuickFlickPlugin::destroyStream(NPError reason)
{
  if (streamStopped()) // || disabled()
    return NPERR_NO_ERROR;

  if (reason != NPRES_DONE)
  {
    setStreamStopped();
    pluginIOInfo.waitForData = FALSE;
    DosPostEventSem(pluginIOInfo.dataReceived);
    if (player())
      DosPostEventSem(player()->proceedWithPrepare);
  }
  else // Normal completion
  {
    setStreamDone();
    if (player() && (pluginIOInfo.currentSize == length_p))
      player()->finishStream();
  }

  return NPERR_NO_ERROR;
}

/************************************************************************/
/* DESTRUCTOR                                                           */
/************************************************************************/
QuickFlickPlugin::~QuickFlickPlugin()
{
  pluginIOInfo.waitForData = FALSE;
  DosPostEventSem(pluginIOInfo.dataReceived);

  delete aboutPluginsPanel;
  delete netscapeConversation;
  if (window())
    pluginWindowPaintHandler.stopHandlingEventsFor(window());

  if (aboutPlugins)
    hangingInstance = 0;

  // Close memory file
  if (movieFile)
    mmioClose(movieFile, 0);

  if ((RefCountedWindow*)viewPort)
    player()->stopHandlingScrollingFor(*(RefCountedWindow*)viewPort);

  // Close semaphores
  DosCloseMutexSem(pluginIOInfo.movieFileAccess);
  DosCloseEventSem(pluginIOInfo.dataReceived);
}

