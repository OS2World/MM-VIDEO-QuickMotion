#include "storefil.hpp"

void StorageSystemFile::setFileInfo(PMMIOINFO fileInfo)
{
  fileInfo_p = fileInfo;
}

StorageSystemFile::StorageSystemFile()
                 : identifier(FOURCC_IOPROC)
{
}

LONG StorageSystemFile::dispatchMessage(USHORT usMsg, LONG lParam1, LONG lParam2)
{
  switch (usMsg)
  {
    case MMIOM_IDENTIFYFILE:
      return identifyFile((PSZ)lParam1, (HMMIO)lParam2);

    case MMIOM_OPEN:
      return open((PSZ)lParam1);

    case MMIOM_READ:
      return read((PSZ)lParam1, lParam2);

    case MMIOM_WRITE:
      return write((PSZ)lParam1, lParam2);

    case MMIOM_SEEK:
      return seek(lParam1, lParam2);

    case MMIOM_CLOSE:
      return close((USHORT)lParam1);

    default:
      return defaultResponse(usMsg, lParam1, lParam2);
  }
}

/************************************************************
 * DEFAULT RESPONSE
 ************************************************************/
LONG StorageSystemFile::defaultResponse(USHORT usMsg, LONG lParam1, LONG lParam2)
{
  setError(MMIOERR_UNSUPPORTED_MESSAGE);
  return (MMIOERR_UNSUPPORTED_MESSAGE);
}

VOID StorageSystemFile::setError(ULONG error)
{
  fileInfo_p->ulErrorRet = error;
}

ULONG StorageSystemFile::flags()
{
  return fileInfo_p->ulFlags;
}

LONG StorageSystemFile::currentFilePosition()
{
  return fileInfo_p->lLogicalFilePos;
}
