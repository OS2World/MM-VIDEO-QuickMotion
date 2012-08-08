#include "qf.h"
#include "mfcsthlr.hpp"

TrackEvent::TrackEvent(IEvent& event)
          : IEvent(event)
{
}

unsigned short TrackEvent::flags()
{
  return parameter1().number1();
}

MinMaxEvent::MinMaxEvent(IEvent& event)
          : IEvent(event)
{
}

unsigned long MinMaxEvent::flags()
{
  return *(unsigned long *)(char *)parameter1();
}

void MinMaxEvent::setRectangle(IRectangle& rectangle)
{
  SWP *swp = (SWP *)(char *)parameter1();
  swp->cy = rectangle.height();
  swp->cx = rectangle.width();
  swp->y = rectangle.minY();
  swp->x = rectangle.minX();
}

Boolean MovieFrameCustomHandler::dispatchHandlerEvent(IEvent &event)
{
  if (event.eventId() == WM_TRACKFRAME)
  {
    TrackEvent trackEvent(event);
    return track(trackEvent);
  }

  if (event.eventId() == WM_MINMAXFRAME)
  {
    MinMaxEvent minMaxEvent(event);
    return minMax(minMaxEvent);
  }

  if (event.eventId() == WM_CUSTOM_SETMOVIE)
    return setMovie(event);

  return (false);
}
