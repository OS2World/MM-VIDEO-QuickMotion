#define INCL_WINWORKPLACE
#include <os2.h>

#include <string.h>
#include <iostream.h>

void main(int argc, char* argv[])
{
  BOOL rc;

  if (argc == 2 && !strcmp(argv[1], "-uninstall"))
  {
    cout << "\nUninstalling Practice MooV file class..." << endl;

    if (!WinReplaceObjectClass("MMMOV", "PracticeMooV", FALSE))
      cout << "ERROR unreplacing MMMOV.  Was it previously installed?" << endl;
    if (!WinDeregisterObjectClass("PracticeMooV"))
      cout << "ERROR deregistering PracticeMooV.  Was it previously installed?" << endl;

    cout << "\nPractice MooV file class uninstall complete. You must shut down\nand reboot for the changes to take effect." << endl;
  }
  else
  {
    cout << "\nInstalling Practice MooV file class..." << endl;

    if (!WinRegisterObjectClass("PracticeMooV", "PRMOOV"))
      cout << "ERROR registering PracticeMooV.  Is PRMOOV.DLL in a directory in your LIBPATH?" << endl;
    if (!WinReplaceObjectClass("MMMOV", "PracticeMooV", TRUE))
      cout << "ERROR replacing MMMOV.  Is PracticeMooV installed?" << endl;

    cout << "\nPractice MooV file class installation complete. You must shut down\nand reboot for the changes to take effect." << endl;
  }
}
