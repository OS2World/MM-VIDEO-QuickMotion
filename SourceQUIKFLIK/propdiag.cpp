#include "quikflik.h"
#include "propdiag.hpp"

#include <ireslib.hpp>
#include <ifont.hpp>
#include <icolor.hpp>

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
IResourceLibrary *PropertiesDialog::resourceLibrary;

/************************************************************************/
/* Class                                                                */
/************************************************************************/
PropertiesDialog::PropertiesDialog(IWindow *owner, IProfile &profile)
                : IFrameWindow(IResourceId(PROPERTIES_ID, *resourceLibrary), owner),
                  center(CENTER_ID, this),
                  fill(FILL_ID, this),
                  detatch(DETATCH_ID, this),
                  cueDuringPrepare(CUE_DURING_PREP_ID, this),
                  profile_p(profile)
{
  cueDuringPrepareValue = false;
  nonEmbeddedDisplayValue = 0;

  ICommandHandler::handleEventsFor(this);

//  setBackgroundColor(IColor::paleGray);
//  setFont(IFont("Helv", 8));

  // Cue during prepare
  try {cueDuringPrepareValue = profile_p.integerWithKey("cueDuringPrepare");} catch (...) {};
  cueDuringPrepare.select(cueDuringPrepareValue);

  // Non-Embedded Movies
  try {nonEmbeddedDisplayValue = profile_p.integerWithKey("nonEmbeddedDisplay");} catch (...) {};
  center.select(nonEmbeddedDisplayValue == QF_NONEMBED_CENTER);
  fill.select(nonEmbeddedDisplayValue == QF_NONEMBED_TOFIT);
  detatch.select(nonEmbeddedDisplayValue == QF_NONEMBED_DETATCH);
}

Boolean PropertiesDialog::command(ICommandEvent &event)
{
  switch (event.commandId())
  {
    case DID_OK:
      cueDuringPrepareValue = cueDuringPrepare.isSelected();
      profile_p.addOrReplaceElementWithKey("cueDuringPrepare", cueDuringPrepareValue);
      nonEmbeddedDisplayValue = (center.isSelected() ?
        QF_NONEMBED_CENTER : (fill.isSelected() ? QF_NONEMBED_TOFIT : QF_NONEMBED_DETATCH));
      profile_p.addOrReplaceElementWithKey("nonEmbeddedDisplay", nonEmbeddedDisplayValue);
      dismiss(DID_OK);
      break;

    default:
      dismiss(DID_CANCEL);
      break;
  }
  return 0;
}

PropertiesDialog::~PropertiesDialog()
{
  ICommandHandler::stopHandlingEventsFor(this);
}

