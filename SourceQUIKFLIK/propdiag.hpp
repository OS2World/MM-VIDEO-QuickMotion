#include <iframe.hpp>
#include <icmdhdr.hpp>
#include <iradiobt.hpp>
#include <icheckbx.hpp>
#include <iprofile.hpp>

class IResourceLibrary;

class PropertiesDialog : public IFrameWindow,
                         private ICommandHandler
{
public:
  static IResourceLibrary     *resourceLibrary;

  PropertiesDialog(IWindow *owner, IProfile &profile);
  ~PropertiesDialog();

private:
  virtual Boolean command(ICommandEvent &event);

  IRadioButton                center, fill, detatch;
  ICheckBox                   cueDuringPrepare;

  IProfile                    &profile_p;

  Boolean                     cueDuringPrepareValue;
  long                        nonEmbeddedDisplayValue;
};

