#include <islidhdr.hpp>
#include <iedithdr.hpp>

class IResourceLibrary;
class IBitmapControl;
class IGraphicPushButton;
class ISlider;

class QFController : public IEditHandler,
                     public ISliderDrawHandler
{
public:
  enum State {disabled, playing, paused};

  static IResourceLibrary     *resourceLibrary;
  static IBitmapHandle        playBitmapHandle, pauseBitmapHandle;

  // Constructor
  QFController(IWindow* parent, State state, float armPercent, float fullPercent);

  // Member functions
  IWindowHandle         logoWindowHandle();

  void                  enablePlay();
  void                  enablePause();
  void                  disableControls();
  State                 state();
  float                 fullPercent();

  void                  setArmPercent(float armPercent);
  void          _Export setFullPercent(float fullPercent);
  void          _Export setDetentPercent(float detentPercent);
  void                  removeDetent();

  void          _Export refresh();

  // Slider drawing
  virtual Boolean drawBackground(IDrawItemEvent &event);
  virtual Boolean drawRibbonStrip(IDrawItemEvent &event);
  virtual Boolean drawShaft(IDrawItemEvent &event);

  // Edit (slider arm position changed)
  virtual Boolean edit(IControlEvent &event);

  // Destructor
  virtual ~QFController();

private:
  IBitmapControl              *logo;
  IGraphicPushButton          *playButton;
  ISlider                     *slider;
  unsigned long               detent, lastFullPixels;
  float                       fullPercent_p;
  State                       state_p;
};

