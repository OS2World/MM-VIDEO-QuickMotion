#include "qmmusic.h"

#include "musifmt.hpp"
#include "moov.hpp"

LONG MusicFormat::identifyFile(PSZ fileName, HMMIO fileHandle)
{
  HMMIO originalFileHandle = fileHandle;
  ULONG ulTempFlags = MMIO_READ | MMIO_DENYWRITE | MMIO_NOIDENTIFY;
  LONG returnCode;

  if (!fileName && !fileHandle)
    return (MMIO_ERROR);

  if (!fileHandle)
    if (!(fileHandle = mmioOpen((PSZ)fileName, NULL, ulTempFlags)))
      return (MMIO_ERROR);

  MovieAtom movieAtom;
  ULONG movieSize;
  returnCode = loadMovie(movieAtom, movieSize, fileHandle);

  if (!originalFileHandle)  /* Don't close handle if provided to us      */
    mmioClose(fileHandle, 0);

  if (returnCode == MMIO_SUCCESS) /* This is a QuickTime file */
    if (movieAtom.isMusicOnly())
      return (MMIO_SUCCESS);
    else
      return (MMIO_ERROR);
  else
    return (returnCode);
}

