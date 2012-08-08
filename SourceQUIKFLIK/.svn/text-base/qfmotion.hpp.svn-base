#ifndef QFMOTION_HPP
#define QFMOTION_HPP

#include "quikflik.h"
#include "IMMDEV.HPP" // Local copy of this which makes us a friend (so we can fix its bugs)

#include <iwindow.hpp>
#include <igbitmap.hpp>
#include <inotifev.hpp>
#include <immdigvd.hpp>
#include <iobservr.hpp>
#include <iobjwin.hpp>
#include <iprofile.hpp>
#include <iseq.h>

#include "qfplayer.hpp"
#include "qfmphlr.hpp"

class IStaticText;
class IThread;
class IBitmapControl;
class IGraphicPushButton;
class ISlider;
class IPopUpMenu;

struct OldPlayer
{
  IMMDigitalVideo *moviePlayer;
  Boolean isDeletable;
};

class QFMotionPlayer: public QFPlayer, public IObserver
{
public:
  // Environment management
  static void initialize();
  static void shutdown();

  // Constructor
  QFMotionPlayer(QuickFlick& quickFlick, Boolean isMusicOnly, Boolean isSoundOnly);

  // QFMotionPlayer member functions
  virtual void                     setWindow(IWindow& window);
  virtual void                     setScreenText(const char *text);
  virtual ISize                    movieSize();
  virtual void             _Export start();
  virtual void             _Export finishStream();
  virtual void             _Export handleScrollingFor(IWindow& viewPort);
  virtual void             _Export stopHandlingScrollingFor(IWindow& viewPort);

  // Destructor
  virtual                          ~QFMotionPlayer();

private:

  // Private member functions
  void prepare();
  void handlePlayCommand();

  // Observer
  virtual QFMotionPlayer& _Export dispatchNotificationEvent(const INotificationEvent&);

  // Custom handler functions
  void gotHeader();
  void open();
  void load();
  void cue();
  void play();
  void pause();
  void seek(long seekTime);
  Boolean destroy(IEvent &event);

  // Resize
  Boolean windowResize(IResizeEvent &event);

  // Commands
  Boolean command(ICommandEvent &event);

  // Data members
  static HMTX                 preparing;
  static ISequence<OldPlayer> oldPlayers;

  QFMotionPlayerHandler             handler;
  IThread                           *prepareThread;
  IWindow                           *window_p;
  IMMFileMedia                      *moviePlayer;
  Boolean                           isMusicOnly_p, isSoundOnly_p;
  Boolean                           stoppedBeforeEnd, isPositionTracking, atEnd, isPlaying;
  Boolean                           firstPassDevice;
  ISequence<OldPlayer>::Cursor      *oldPlayerCursor;

  IStaticText                       *screen, *movieWindow;
  IPopUpMenu                        *menu;

friend class QFMotionPlayerHandler;
};

#endif

