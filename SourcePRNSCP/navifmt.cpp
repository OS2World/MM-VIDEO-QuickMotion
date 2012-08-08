#include "navifmt.hpp"

LONG NavigatorFormat::identifyFile(PSZ fileName, HMMIO fileHandle)
{
  FOURCC storageSystem;
  CHAR remainder[256];
  LONG returnCode;

  if (!fileName)
    return (MMIO_ERROR);

  returnCode = mmioDetermineSSIOProc(fileName, NULL, &storageSystem, remainder);
  if (returnCode != MMIO_SUCCESS)
    return (MMIO_ERROR);

  if (storageSystem == FOURCC_IOPROC)
    return (MMIO_SUCCESS);
  else
    return (MMIO_ERROR);
}

