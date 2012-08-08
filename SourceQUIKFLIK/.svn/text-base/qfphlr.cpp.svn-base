#include "quikflik.h"
#include "qfphlr.hpp"

Boolean QFPlayerHandler::dispatchHandlerEvent(IEvent &event)
{
  switch (event.eventId())
  {
    case WM_CUSTOM_GOT_HEADER:
      return gotHeader(event);

    case WM_CUSTOM_OPEN:
      return open(event);

    case WM_CUSTOM_LOAD:
      return load(event);

    case WM_CUSTOM_CUE:
      return cue(event);

    case WM_CUSTOM_PLAY:
      return play(event);

    case WM_CUSTOM_PAUSE:
      return pause(event);

    case WM_CUSTOM_SEEK:
      return seek(event);

    case WM_CUSTOM_EDIT:
      return edit(event);

    case WM_VRNENABLED:
      return visibleRegionEnabled(event);

    case WM_VRNDISABLED:
      return visibleRegionDisabled(event);

    case WM_DESTROY:
      return destroy(event);

    default:
      return (false);
  }
}

Boolean QFPlayerHandler::open(IEvent &event)
{
  return false;
}

Boolean QFPlayerHandler::load(IEvent &event)
{
  return false;
}

Boolean QFPlayerHandler::cue(IEvent &event)
{
  return false;
}

Boolean QFPlayerHandler::play(IEvent &event)
{
  return false;
}

Boolean QFPlayerHandler::pause(IEvent &event)
{
  return false;
}

Boolean QFPlayerHandler::seek(IEvent &event)
{
  return false;
}

Boolean QFPlayerHandler::edit(IEvent &event)
{
  return false;
}

Boolean QFPlayerHandler::visibleRegionEnabled(IEvent &event)
{
  return false;
}

Boolean QFPlayerHandler::visibleRegionDisabled(IEvent &event)
{
  return false;
}

