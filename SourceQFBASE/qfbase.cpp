#include "qfwrend.hpp"

#include <iprofile.hpp>

/************************************************************************/
/* MINSTALL installation routine                                        */
/************************************************************************/
ULONG APIENTRY install(HWND hwndOwnerHandle, PSZ pszSourcePath, PSZ pszTargetDrive,
                       PSZ pszMyParms, HWND hwndMinstallHandle, PSZ pszResponseFile)
{
  APIRET returnCode;

  // This routine may be linked to the previous release of PROCL.LIB
  // Be careful when adding new calls into the OCL.

  //Install plugin
  IString quickFlickPlugin = IString(pszSourcePath) + "\\NPQKFL.DLL";
  IProfile profile(IProfile::userProfile());
  profile.setDefaultApplicationName("Netscape");
  IProfile::Cursor profileCursor(profile, profile.defaultApplicationName());
  profileCursor.setToLast();
  if (profileCursor.isValid())
  {
    IString pluginPath = profile.elementWithKey(profile.applicationOrKeyAt(profileCursor));
    pluginPath += "\\PLUGINS";
    returnCode = DosCopy(quickFlickPlugin, pluginPath, DCPY_EXISTING);
  }

  //Install PracticeMooV
  if (WinRegisterObjectClass("PracticeMooV", "PRMOOV"))
    WinReplaceObjectClass("MMMOV", "PracticeMooV", TRUE);

  return 0;
}

void EXPENTRY QUIKFLIK(QFWorldRenderer *worldRenderer)
{
#ifndef DEMO_VERSION
  worldRenderer->minPan = worldRenderer->minPanReg;
  worldRenderer->maxPan = worldRenderer->maxPanReg;
  worldRenderer->maxFov = worldRenderer->maxFovReg;
#endif
}
