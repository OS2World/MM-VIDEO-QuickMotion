#include "qmsound.h"

#include <memory.h>
#include <fourcc.h>

#include "audiofil.hpp"

AudioFile::AudioFile()
         : fileInfo_p(0), storageSystemFileHandle_p(0)

#ifdef DEBUGGING_IOPROC
, debugOutput("audiofil.msg", ios::out | ios::trunc)
#endif

{
  memset(&audioHeader, '\0', sizeof(audioHeader));
}

void AudioFile::setFileInfo(PMMIOINFO fileInfo)
{
  fileInfo_p = fileInfo;
}

LONG AudioFile::dispatchMessage(USHORT usMsg, LONG lParam1, LONG lParam2)
{
#ifdef DEBUGGING_IOPROC
  debugOutput << "Message:\t" << usMsg << '\t' << lParam1 << '\t' << lParam2 << endl;
#endif

  switch (usMsg)
  {
    case MMIOM_IDENTIFYFILE:
      return identifyFile((PSZ)lParam1, (HMMIO)lParam2);

    case MMIOM_QUERYHEADERLENGTH:
      return queryHeaderLength();

    case MMIOM_OPEN:
      return open((PSZ)lParam1);

    case MMIOM_GETHEADER:
      return getHeader((PVOID)lParam1, lParam2);

    case MMIOM_SETHEADER:
      return setHeader((PVOID)lParam1, lParam2);

    case MMIOM_READ:
      return read((PSZ)lParam1, lParam2);

    case MMIOM_SEEK:
      return seek(lParam1, lParam2);

    case MMIOM_WRITE:
      return write((PSZ)lParam1, lParam2);

    case MMIOM_SEEKBYTIME:
      return seekByTime(lParam1, lParam2);

    case MMIOM_CLOSE:
      return close((USHORT)lParam1);

    default:
      return defaultResponse(usMsg, lParam1, lParam2);
  }
}

/************************************************************
 * QUERYHEADERLENGTH
 ************************************************************/
LONG AudioFile::queryHeaderLength()
{
  return (sizeof(MMAUDIOHEADER));
}

/************************************************************
 * OPEN
 ************************************************************/
LONG AudioFile::open(PSZ fileName)
{
  FOURCC fccStorageSystem;               /* SS I/O Proc FOURCC       */
  MMIOINFO  mmioinfoSS;                  /* I/O info block for SS ref*/

  if (fileInfo_p->ulFlags & (MMIO_WRITE | MMIO_READWRITE))
    return (MMIO_ERROR);

  /*********************************************************
  * Determine the storage system/child IOProc that actually
  *   obtains the data for us.   The compressed data may be
  *   in a memory (RAM) file, a component in a database or
  *   library (a Compound file), or a stand-alone disk file.
  *
  * While the Application uses this IOProc to obtain movie
  *   data, the IOProc must obtain its data from something that
  *   reads and writes to a storage media.  The exact storage
  *   media is immaterial - so long as the read and write
  *   operations generate data that LOOKS like it is part
  *   of a standard file.
  *********************************************************/
  if (!fileInfo_p->fccChildIOProc)
  {
    if (fileInfo_p->ulFlags & MMIO_CREATE)
      {
      if (mmioDetermineSSIOProc(fileName, fileInfo_p, &fccStorageSystem, NULL))
        fccStorageSystem = FOURCC_DOS; /* SS not recognized from file name */
      }
    else
      {
      if (mmioIdentifyStorageSystem(fileName, fileInfo_p, &fccStorageSystem))
        return (MMIO_ERROR);
      };
    if (!fccStorageSystem)
      return (MMIO_ERROR);
    else
      fileInfo_p->fccChildIOProc = fccStorageSystem;
  }

  /**************************************************************
   * Open the file
   **************************************************************/
  memmove(&mmioinfoSS, fileInfo_p, sizeof(MMIOINFO));
  mmioinfoSS.pIOProc = NULL;
  mmioinfoSS.fccIOProc = fileInfo_p->fccChildIOProc;
  mmioinfoSS.ulFlags |= MMIO_NOIDENTIFY; /* Avoid recursive loop */
  if (mmioinfoSS.ulFlags & MMIO_READWRITE)
    mmioinfoSS.ulFlags |= MMIO_READ; /* Set read mode if readwrite requested.  Never get here due to above. */
  mmioinfoSS.ulFlags &= ~MMIO_WRITE & ~MMIO_READWRITE; /* Force non-writable open */

  storageSystemFileHandle_p = mmioOpen(fileName, &mmioinfoSS, mmioinfoSS.ulFlags);

  /************************************************************
   * Check if a DELETE was requested - mmioOpen returns a 1,
   *    so we much check this separately
   ************************************************************/
  if (fileInfo_p->ulFlags & MMIO_DELETE)
  {
    if (!storageSystemFileHandle_p)
      {
        fileInfo_p->ulErrorRet = MMIOERR_DELETE_FAILED;
        return (MMIO_ERROR);
      }
    else
      return (MMIO_SUCCESS);
  };

  /************************************************************
   * Check the return code from the open call for an error.
   *    If not delete, then the open should have worked.
   ************************************************************/
  if (!storageSystemFileHandle_p)
      return (MMIO_ERROR);

  return (MMIO_SUCCESS);
}

/************************************************************
 * CLOSE
 ************************************************************/
LONG AudioFile::close(USHORT closeFlags)
{
  return mmioClose(storageSystemFileHandle_p, closeFlags);
}

/************************************************************
 * DEFAULT RESPONSE
 ************************************************************/
LONG AudioFile::defaultResponse(USHORT usMsg, LONG lParam1, LONG lParam2)
{
  LONG rc;

  if (storageSystemFileHandle)
  {
    rc = mmioSendMessage(storageSystemFileHandle_p, usMsg, lParam1, lParam2);
    if (rc)
      setError(mmioGetLastError(storageSystemFileHandle_p));
    return (rc);
  }
  else
  {
    setError(MMIOERR_UNSUPPORTED_MESSAGE);
    return (MMIOERR_UNSUPPORTED_MESSAGE);
  }
}

HMMIO AudioFile::storageSystemFileHandle()
{
  return storageSystemFileHandle_p;
}

VOID AudioFile::setError(ULONG error)
{
  fileInfo_p->ulErrorRet = error;
}

ULONG AudioFile::flags()
{
  return fileInfo_p->ulFlags;
}

/************************************************************
 * Destructor
 ************************************************************/
AudioFile::~AudioFile()
{
#ifdef DEBUGGING_IOPROC
  debugOutput.close();
#endif
}

