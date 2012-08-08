#include "qfplayer.hpp"
#include "quikflik.hpp"

QFPlayer::QFPlayer(QuickFlick& quickFlick)
        : qf(quickFlick)
{
  serviceThreadWindow = 0;
}

QFPlayer::~QFPlayer()
{
  qf.serviceThreadControlWindow()->postEvent(WM_CUSTOM_DESTROY, serviceThreadWindow);
}

