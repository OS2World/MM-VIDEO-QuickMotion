#include "quikflik.hpp"
#include "npqkfl.h"

#include "about.hpp"

#include <ifont.hpp>

/************************************************************************/
/* Class                                                                */
/************************************************************************/
AboutPluginsPanel::AboutPluginsPanel(IWindowHandle parentOwner, QuickFlick* quickFlick)
                 : IWindow(parentOwner),
                   multiCellCanvas(ABOUTPLUGINS_ID, this, this, IRectangle(this->size()), IWindow::noStyle),
                   logoCanvas(ABOUTPLUGINS_LOGO_CANVAS_ID, &multiCellCanvas, &multiCellCanvas),
                   logo(ABOUTPLUGINS_LOGO_ID, &logoCanvas, &logoCanvas, IResourceId(QFLOGO_BITMAP_ID, QuickFlick::resourceLibrary()),
                        IRectangle(), IBitmapControl::defaultStyle() | IBitmapControl::sizeToBitmap),
                   properties(ABOUTPLUGINS_PROPERTIES_ID, &multiCellCanvas, &multiCellCanvas),
                   quickFlick_p(quickFlick)
{
  ICommandHandler::handleEventsFor(&multiCellCanvas);

  setBackgroundColor(IColor::paleGray);
  setFont(IFont("Helv", 8));

  // Logo
  multiCellCanvas.setRowHeight(1, 24, false);
  multiCellCanvas.addToCell(&logoCanvas, 1, 1);

   // Empty column
  multiCellCanvas.setColumnWidth(2, 10, false);

  // Properties pushbutton
  properties.setText("Properties...");
  properties.enableDefault();
  multiCellCanvas.addToCell(&properties, 3, 1);

  multiCellCanvas.show();
}

Boolean AboutPluginsPanel::command(ICommandEvent &event)
{
  switch (event.commandId())
  {
    case ABOUTPLUGINS_PROPERTIES_ID:
      QuickFlick::showPropertiesDialog(this);
      return true;

    default:
      return false;
  }
}

AboutPluginsPanel::~AboutPluginsPanel()
{
  ICommandHandler::stopHandlingEventsFor(&multiCellCanvas);
}

