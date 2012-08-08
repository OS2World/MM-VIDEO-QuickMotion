#include "quikflik.hpp"
#include "pluginio.hpp"
#include "about.hpp"

#include <irefcnt.hpp>
#include <iddeccnv.hpp>
#include <ipainhdr.hpp>

class RefCountedWindow : public IWindow, public IRefCounted
{
public:
  RefCountedWindow(const IWindowHandle& handle);
};

struct HotSpotEntry
{
  long hotSpotID;
  IString url;
};
long const& key(HotSpotEntry const& element);



class NetscapeConversation : public IDDEClientConversation
{
public:
  NetscapeConversation(IWindowHandle windowHandle);
  virtual void handleInitiateAck(const IEvent& initiateAckEvent);
  virtual Boolean data(IDDEDataEvent& event);
  ~NetscapeConversation();
};


class PluginWindowPaintHandler : public IPaintHandler
{
protected:
  virtual Boolean paintWindow(IPaintEvent& event);
};


class QuickFlickPlugin : public QuickFlick
{
public:
  static Boolean mainThreadPriorityBoosted;

  // Constructor
  QuickFlickPlugin(NPMIMEType pluginType, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved, NPP instance);

  static void initialize();
  static void shutdown();

  virtual IString hotSpotURL(long hotSpotID);
  virtual void linkTo(IString url);
  virtual void status(IString text);

  // Plugin API functions
  NPError setWindow(IWindowHandle newWindowHandle);
  NPError newStream(IString url, unsigned long length);
  int32 writeReady();
  int32 write(unsigned long length, PCHAR buffer);
  NPError destroyStream(NPError reason);

  // Destructor
  virtual ~QuickFlickPlugin();

private:

  static QuickFlickPlugin      *hangingInstance;

  HMMIO                        movieFile;
  NPSavedData*                 pSavedInstanceData;
  NPP                          instance_p;
  PluginIOInfo                 pluginIOInfo;
  unsigned long                whenStreamStarted, length_p;
  IString                      mimeType, url_p;
  IString                      volume_p, href_p, target_p, pan_p, tilt_p, fov_p, node_p, correction_p;
  uint16                       mode_p;
  Boolean                      controllerTag, aboutPlugins;

  AboutPluginsPanel            *aboutPluginsPanel;
  IReference<RefCountedWindow> viewPort;
  IKeySortedSet<HotSpotEntry, long> hotSpots;
  NetscapeConversation         *netscapeConversation;
  PluginWindowPaintHandler     pluginWindowPaintHandler;

protected:
  virtual void                 playerCreated();

};

