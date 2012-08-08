#include "moovproc.h"
#include "moviefmt.hpp"

#include <string.h>

LONG MovieFileFormat::dispatchMessage(USHORT usMsg, LONG lParam1, LONG lParam2)
{
  switch (usMsg)
  {
    case MMIOM_GETFORMATINFO:
      return getFormatInfo((PMMFORMATINFO)lParam1);

    case MMIOM_GETFORMATNAME:
      return getFormatName((PSZ)lParam1, lParam2);

    case MMIOM_IDENTIFYFILE:
      return identifyFile((PSZ)lParam1, (HMMIO)lParam2);

    default:
      return defaultResponse();
  }
}

LONG MovieFileFormat::getFormatInfo(PMMFORMATINFO pMMFormatInfo)
{
  pMMFormatInfo->ulStructLen = sizeof(MMFORMATINFO);
  pMMFormatInfo->fccIOProc = FOURCC_IOPROC;
  pMMFormatInfo->ulIOProcType = MMIO_IOPROC_FILEFORMAT;
  pMMFormatInfo->ulMediaType = MMIO_MEDIATYPE_MOVIE;

  pMMFormatInfo->ulFlags = MMIO_CANMULTITRACKREADUNTRANSLATED |
                           MMIO_CANTRACKREADUNTRANSLATED      |
                           MMIO_CANSEEKUNTRANSLATED           |
                           MMIO_CANTRACKSEEKUNTRANSLATED;

  strcpy ((PSZ) pMMFormatInfo->szDefaultFormatExt, pszFormatExt);
  if (GetNLSData(&pMMFormatInfo->ulCodePage, &pMMFormatInfo->ulLanguage))
    return (MMIO_ERROR);

  if (GetFormatStringLength(FOURCC_IOPROC, &(pMMFormatInfo->lNameLength)))
    return (MMIO_ERROR);

  return (MMIO_SUCCESS);
}

LONG MovieFileFormat::getFormatName(PSZ formatName, LONG formatNameSize)
{
  LONG lBytesCopied;

  lBytesCopied = GetFormatString(FOURCC_IOPROC, formatName, formatNameSize);

  return (lBytesCopied);
}

LONG MovieFileFormat::defaultResponse()
{
  return (MMIOERR_UNSUPPORTED_MESSAGE);
}

