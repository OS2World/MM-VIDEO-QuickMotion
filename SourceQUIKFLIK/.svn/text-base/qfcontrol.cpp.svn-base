#include "quikflik.h"
#include "qfcontrol.hpp"

#include <ireslib.hpp>
#include <ibmpctl.hpp>
#include <igraphbt.hpp>
#include <islider.hpp>

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
IResourceLibrary *QFController::resourceLibrary;
IBitmapHandle QFController::playBitmapHandle, QFController::pauseBitmapHandle;

/************************************************************************/
/* CONSTRUCTOR                                                          */
/************************************************************************/
QFController::QFController(IWindow* parent, State state, float armPercent, float fullPercent)
{
  logo = 0;
  playButton = 0;
  slider = 0;
  detent = lastFullPixels = 0;
  fullPercent_p = 0.0;
  state_p = state;
  ISize size = parent->size();

  ISize buttonSize = ISize(CONTROL_PANEL_HEIGHT, CONTROL_PANEL_HEIGHT);
  long bottom = (size.height() >= CONTROL_PANEL_HEIGHT ? 0 : (size.height() - CONTROL_PANEL_HEIGHT) / 2);
  IPoint newControlPoint;

  // Create play button
  newControlPoint = IPoint(0, bottom);
  playButton = new IGraphicPushButton(PLAY_BUTTON_ID, parent, parent,
                                      (state_p == playing ? pauseBitmapHandle : playBitmapHandle),
                                      IRectangle(newControlPoint, buttonSize),
                                      IGraphicPushButton::classDefaultStyle | IPushButton::noPointerFocus);
  playButton->enable(state_p != disabled);

  // Create slider
  if (size.width() > CONTROL_PANEL_HEIGHT + 16)
  {
    newControlPoint = IPoint(CONTROL_PANEL_HEIGHT, bottom);
    unsigned long sliderWidth = size.width() - CONTROL_PANEL_HEIGHT;
    if (size.width() >= CONTROL_PANEL_HEIGHT + 84)
      sliderWidth -= CONTROL_PANEL_HEIGHT; // Allow room for logo

    if (size.width() >= CONTROL_PANEL_HEIGHT + 64) // Room for buttons
      slider = new ISlider(SLIDER_ID, parent, parent,
                       IRectangle(newControlPoint, ISize(sliderWidth, CONTROL_PANEL_HEIGHT)),
                       2, 0,
                       ISlider::defaultStyle() & ~(ISlider::buttonsLeft) | ISlider::buttonsRight);
    else // No room for buttons
      slider = new ISlider(SLIDER_ID, parent, parent,
                       IRectangle(newControlPoint, ISize(sliderWidth, CONTROL_PANEL_HEIGHT)),
                       2, 0,
                       ISlider::defaultStyle() & ~(ISlider::buttonsLeft));

    slider->enable(state_p == paused);
    slider->setTicks(IProgressIndicator::scale1, slider->armRange()); // Make every pixel a "tick"
    slider->enableRibbonStrip();
    slider->enableDrawItem();
    setArmPercent(armPercent);
    fullPercent_p = fullPercent;
    ISliderDrawHandler::handleEventsFor(slider);
    IEditHandler::handleEventsFor(slider);
  }

  // Create logo
  if (size.width() >= CONTROL_PANEL_HEIGHT + 84)
  {
    newControlPoint = IPoint(size.width() - CONTROL_PANEL_HEIGHT, bottom);
    logo = new IBitmapControl(LOGO_ID, parent, parent,
                              IResourceId(QMLOGO_BITMAP_ID, *resourceLibrary),
                              IRectangle(newControlPoint, buttonSize),
                              IBitmapControl::defaultStyle());
  }

  if (size.width() > CONTROL_PANEL_HEIGHT && size.width() <= CONTROL_PANEL_HEIGHT + 16)
  {
    newControlPoint = IPoint(CONTROL_PANEL_HEIGHT, bottom);
    logo = new IBitmapControl(LOGO_ID, parent, parent,
                              IResourceId(QMLOGO_BITMAP_ID, *resourceLibrary),
                              IRectangle(newControlPoint, buttonSize),
                              IBitmapControl::defaultStyle());
  }
}

/************************************************************************/
/* LOGO WINDOW HANDLE                                                   */
/************************************************************************/
IWindowHandle QFController::logoWindowHandle()
{
  if (logo)
    return logo->handle();
  else
    return IWindowHandle();
}

/************************************************************************/
/* ENABLE PLAY                                                          */
/************************************************************************/
void QFController::enablePlay()
{
  if (playButton)
  {
    playButton->enable();
    playButton->setGraphic(playBitmapHandle);
  }
  if (slider)
    slider->enable();
  state_p = paused;
}

/************************************************************************/
/* ENABLE PAUSE                                                         */
/************************************************************************/
void QFController::enablePause()
{
  if (playButton)
  {
    playButton->enable();
    playButton->setGraphic(pauseBitmapHandle);
  }
  if (slider)
    slider->disable();
  state_p = playing;
}

/************************************************************************/
/* DISABLE CONTROLS                                                     */
/************************************************************************/
void QFController::disableControls()
{
  if (playButton)
  {
    playButton->disable();
    playButton->setGraphic(playBitmapHandle);
  }
  if (slider)
    slider->disable();
  state_p = disabled;
}

/************************************************************************/
/* STATE                                                                */
/************************************************************************/
QFController::State QFController::state()
{
  return state_p;
}

/************************************************************************/
/* FULL PERCENT                                                         */
/************************************************************************/
float QFController::fullPercent()
{
  return fullPercent_p;
}

/************************************************************************/
/* SET ARM PERCENT                                                      */
/************************************************************************/
void QFController::setArmPercent(float armPercent)
{
  if (slider)
  {
    float armRange = slider->armRange() - 1;
    slider->moveArmToPixel(armRange * armPercent);
  }
}

/************************************************************************/
/* SET FULL PERCENT                                                     */
/************************************************************************/
void QFController::setFullPercent(float fullPercent)
{
  if (slider)
  {
    fullPercent_p = fullPercent;
    unsigned long fullPixels = fullPercent_p * (slider->shaftSize().width() - 6);
    if (fullPixels != lastFullPixels)
    {
      refresh();
      lastFullPixels = fullPixels;
    }
  }
}

/************************************************************************/
/* SET DETENT PERCENT                                                   */
/************************************************************************/
void QFController::setDetentPercent(float detentPercent)
{
  removeDetent();
  if (slider)
  {
    unsigned long detentPosition = detentPercent * (slider->armRange() - 1) + 5;
    if (detentPosition < slider->armRange() - 1)
      detent = slider->addDetent(detentPosition);
  }
}

/************************************************************************/
/* REMOVE DETENT                                                        */
/************************************************************************/
void QFController::removeDetent()
{
  if (detent)
  {
    // Drawing could be happening on another thread so be careful
    unsigned long tempDetent = detent;
    detent = 0;
    slider->removeDetent(tempDetent);
  }
}

/************************************************************************/
/* REFRESH                                                              */
/************************************************************************/
void QFController::refresh()
{
  if (playButton)
    playButton->refresh();

  if (slider)
    slider->refresh();

  if (logo)
    logo->refresh();
}

/************************************************************************/
/* DRAW BACKGROUND                                                      */
/************************************************************************/
Boolean QFController::drawBackground(IDrawItemEvent &event)
{
  RECTL rectl = event.itemRect().asRECTL();
  WinFillRect((HPS)event.itemPresSpaceHandle(), &rectl, CLR_PALEGRAY);

  if (detent)
  {
    RECTL detentRectl = rectl;
    detentRectl.xLeft = slider->tickPosition(0).x() + slider->detentPosition(detent);
    detentRectl.xRight = detentRectl.xLeft + 1;
    detentRectl.yBottom = detentRectl.yTop - 4;
    WinFillRect((HPS)event.itemPresSpaceHandle(), &detentRectl, CLR_BLACK);
  }

  return true;
}

/************************************************************************/
/* DRAW RIBBON STRIP                                                    */
/************************************************************************/
Boolean QFController::drawRibbonStrip(IDrawItemEvent &event)
{
  if (slider->armPixelOffset() == 0)
    return true;

  RECTL fullRectl = event.itemRect().asRECTL();
  RECTL emptyRectl = fullRectl;
  LONG shaftWidth = slider->shaftSize().width() - 6;
  LONG firstEmptyPixel = fullRectl.xLeft + fullPercent_p * shaftWidth;

  if (firstEmptyPixel < fullRectl.xRight)
  {
    fullRectl.xRight = emptyRectl.xLeft = firstEmptyPixel;
    WinFillRect((HPS)event.itemPresSpaceHandle(), &emptyRectl, CLR_PALEGRAY);
  }

  WinFillRect((HPS)event.itemPresSpaceHandle(), &fullRectl, CLR_DARKBLUE);

  return true;
}

/************************************************************************/
/* DRAW SHAFT                                                           */
/************************************************************************/
Boolean QFController::drawShaft(IDrawItemEvent &event)
{
  if (slider->armPixelOffset() >= slider->armRange() - 1)
    return true;

  RECTL emptyRectl = event.itemRect().asRECTL();
  RECTL fullRectl = emptyRectl;
  LONG shaftWidth = slider->shaftSize().width() - 6;
  LONG firstEmptyPixel = emptyRectl.xRight - shaftWidth + 1 + fullPercent_p * shaftWidth;

  if (firstEmptyPixel >= emptyRectl.xLeft)
  {
    fullRectl.xRight = emptyRectl.xLeft = firstEmptyPixel;
    WinFillRect((HPS)event.itemPresSpaceHandle(), &fullRectl, CLR_DARKBLUE);
  }

  WinFillRect((HPS)event.itemPresSpaceHandle(), &emptyRectl, CLR_PALEGRAY);

  return true;
}

/************************************************************************/
/* EDIT (slider arm position changed)                                   */
/************************************************************************/
Boolean QFController::edit(IControlEvent &event)
{
  if (slider->isEnabled())
    slider->parent()->postEvent(WM_CUSTOM_EDIT, slider->armPixelOffset(), slider->armRange() - 1);

  return false;
}

/************************************************************************/
/* DESTRUCTOR                                                           */
/************************************************************************/
QFController::~QFController()
{

  if (logo)
  {
    delete logo;
    logo = 0;
  }

  if (detent)
  {
    slider->removeDetent(detent);
    detent = 0;
  }

  if (slider)
  {
    ISliderDrawHandler::stopHandlingEventsFor(slider);
    IEditHandler::stopHandlingEventsFor(slider);
    delete slider;
    slider = 0;
  }

  if (playButton)
  {
    delete playButton;
    playButton = 0;
  }
}
