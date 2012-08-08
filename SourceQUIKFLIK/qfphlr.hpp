#ifndef QFPHLR_HPP
#define QFPHLR_HPP

#include <ihandler.hpp> // IHandler

class QFPlayerHandler : public IHandler
{
public:
  virtual Boolean dispatchHandlerEvent(IEvent &event);
  virtual Boolean gotHeader(IEvent &event) = 0;
  virtual Boolean open(IEvent &event);
  virtual Boolean load(IEvent &event);
  virtual Boolean cue(IEvent &event);
  virtual Boolean play(IEvent &event);
  virtual Boolean pause(IEvent &event);
  virtual Boolean seek(IEvent &event);
  virtual Boolean edit(IEvent &event);
  virtual Boolean visibleRegionEnabled(IEvent &event);
  virtual Boolean visibleRegionDisabled(IEvent &event);
  virtual Boolean destroy(IEvent &event) = 0;
};

#endif

