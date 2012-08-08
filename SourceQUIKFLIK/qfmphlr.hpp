#ifndef QFMPHLR_HPP
#define QFMPHLR_HPP

#include "qfphlr.hpp"

#include <iscrlhdr.hpp>
#include <ikeyhdr.hpp>
#include <ipainhdr.hpp>
#include <icmdhdr.hpp>
#include <imoushdr.hpp>
#include <isizehdr.hpp>
#include <imenuhdr.hpp>

class QFMotionPlayer;

class QFMotionPlayerHandler : public QFPlayerHandler,
                              public IScrollHandler,
                              public IKeyboardHandler,
                              public IPaintHandler,
                              public ICommandHandler,
                              public IMouseHandler,
                              public IMenuHandler,
                              public IResizeHandler
{
public:
  // Constructor
  QFMotionPlayerHandler(QFMotionPlayer &qfMotionPlayer);

  // Helper
  Boolean scroll(IEvent &event);

  // Custom
  virtual Boolean gotHeader(IEvent &event);
  virtual Boolean open(IEvent &event);
  virtual Boolean load(IEvent &event);
  virtual Boolean cue(IEvent &event);
  virtual Boolean play(IEvent &event);
  virtual Boolean pause(IEvent &event);
  virtual Boolean seek(IEvent &event);
  virtual Boolean edit(IEvent &event);
  virtual Boolean destroy(IEvent &event);

  // Painting
  virtual Boolean paintWindow(IPaintEvent &event);

  // Menu
  virtual Boolean makePopUpMenu(IMenuEvent &event);

  // Resize
  virtual Boolean windowResize(IResizeEvent &event);

  // Commands
  virtual Boolean command(ICommandEvent &event);

  // Mouse
  virtual Boolean mouseClicked(IMouseClickEvent &event);

  // Scrolling
  virtual Boolean lineUp(IScrollEvent &event);
  virtual Boolean lineDown(IScrollEvent &event);
  virtual Boolean lineLeft(IScrollEvent &event);
  virtual Boolean lineRight(IScrollEvent &event);
  virtual Boolean pageUp(IScrollEvent &event);
  virtual Boolean pageDown(IScrollEvent &event);
  virtual Boolean pageLeft(IScrollEvent &event);
  virtual Boolean pageRight(IScrollEvent &event);
  virtual Boolean scrollBoxTrack(IScrollEvent &event);
  virtual Boolean virtualKeyPress(IKeyboardEvent &event);

private:
  QFMotionPlayer &qfmp;
};

#endif

