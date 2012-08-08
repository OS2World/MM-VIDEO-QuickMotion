#define INCL_32
#define INCL_GPI
#include <os2.h>

#define INCL_MMIOOS2
#define INCL_MMIO_CODEC
#include <os2me.h>

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
HMODULE moduleHandle;

/************************************************************************/
/* _DLL_InitTerm:                                                       */
/************************************************************************/
ULONG _System _DLL_InitTerm (ULONG hModHandle, ULONG fTerm)
{
  return (1L);  /* Success */
}

/************************************************************************/
/* MINSTALL installation routine                                        */
/************************************************************************/
ULONG APIENTRY remove(HWND hwndOwnerHandle, PSZ pszSourcePath, PSZ pszTargetDrive,
                      PSZ pszMyParms, HWND hwndMinstallHandle, PSZ pszResponseFile)
{
  ULONG returnCode;

  CODECINIFILEINFO codecIniFileInfo;
  codecIniFileInfo.ulStructLen = sizeof(CODECINIFILEINFO);
  codecIniFileInfo.fcc = mmioFOURCC('M', 'o', 'o', 'V');

  do
    returnCode = mmioIniFileCODEC(&codecIniFileInfo, MMIO_REMOVEPROC);
  while (returnCode == MMIO_SUCCESS);

  MMINIFILEINFO iniFileInfo;

  iniFileInfo.fccIOProc = mmioFOURCC('M', 'o', 'o', 'V');
  mmioIniFileHandler(&iniFileInfo, MMIO_REMOVEPROC);

  iniFileInfo.fccIOProc = mmioFOURCC('S', 'M', 'o', 'V');
  mmioIniFileHandler(&iniFileInfo, MMIO_REMOVEPROC);

  iniFileInfo.fccIOProc = mmioFOURCC('N', 'S', 'C', 'P');
  mmioIniFileHandler(&iniFileInfo, MMIO_REMOVEPROC);

  return 0;
}

