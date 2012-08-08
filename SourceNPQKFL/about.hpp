#include <ireslib.hpp>
#include <imcelcv.hpp>
#include <icmdhdr.hpp>
#include <ibmpctl.hpp>
#include <ipushbut.hpp>

class QuickFlick;

class AboutPluginsPanel : public IWindow,
                          private ICommandHandler
{
public:
  AboutPluginsPanel(IWindowHandle parentOwner, QuickFlick* quickFlick);
  ~AboutPluginsPanel();

private:
  virtual Boolean command(ICommandEvent &event);

  QuickFlick*                 quickFlick_p;

  IMultiCellCanvas            multiCellCanvas;
  ICanvas                     logoCanvas;
  IBitmapControl              logo;
  IPushButton                 properties;
};

