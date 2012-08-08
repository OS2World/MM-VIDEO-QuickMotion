#ifndef MFCSTHLR_HPP
#define MFCSTHLR_HPP

#include <irect.hpp>    // IRectangle
#include <ihandler.hpp> // IHandler

#define INCL_WIN
#define INCL_WINTRACKRECT
#include <os2.h>

class TrackEvent : public IEvent
{
public:
  TrackEvent(IEvent &event);
  unsigned short flags();
};

class MinMaxEvent : public IEvent
{
public:
  MinMaxEvent(IEvent& event);
  unsigned long flags();
  void setRectangle(IRectangle& rectangle);
};

class MovieFrameCustomHandler : public IHandler
{
public:
  virtual Boolean dispatchHandlerEvent(IEvent &event);
  virtual Boolean track(TrackEvent &event) = 0;
  virtual Boolean minMax(MinMaxEvent& event) = 0;
  virtual Boolean setMovie(IEvent &event) = 0;
};

#endif
