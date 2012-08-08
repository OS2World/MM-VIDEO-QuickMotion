#ifndef QFWORLD_HPP
#define QFWORLD_HPP

#include "quikflik.h"

#include <iobjwin.hpp>
#include <istattxt.hpp>
#include <iprofile.hpp>

#include "qfplayer.hpp"
#include "qfwphlr.hpp"

class IPopUpMenu;
class QFWorldRenderer;

class QFWorldPlayer : public QFPlayer
{
public:
  // Environment management
  static void initialize();
  static void shutdown();

  // Constructor
  QFWorldPlayer(QuickFlick& quickFlick);

  // QFWorldPlayer member functions
  virtual void                     setWindow(IWindow& window);
  virtual void                     setScreenText(const char *text);
  virtual ISize                    movieSize();
  virtual void             _Export start();
  virtual void             _Export finishStream();
  virtual void             _Export handleScrollingFor(IWindow& viewPort);
  virtual void             _Export stopHandlingScrollingFor(IWindow& viewPort);

  // Destructor
  virtual                          ~QFWorldPlayer();

private:
  static IPointerHandle            openHand, pullHand, spinArrows, spinHand, zoom, hotHand;

  // Custom handler functions
  void gotHeader();
  void saveAs(IString fileName);
  Boolean destroy(IEvent &event);

  // Resize
  Boolean windowResize(IResizeEvent &event);

  // Commands
  Boolean command(ICommandEvent &event);

  QFWorldPlayerHandler              handler;
  QFWorldRenderer                   *worldRenderer;
  IWindow                           *window_p;
  IStaticText                       *screen, *movieWindow;
  IPopUpMenu                        *menu;

friend class QFWorldPlayerHandler;
};

#endif

