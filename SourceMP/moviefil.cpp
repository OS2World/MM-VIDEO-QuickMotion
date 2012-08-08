#include "moovproc.h"

#include <memory.h>
#include <fourcc.h>

#include "moviefil.hpp"

#ifdef DEBUGGING_IOPROC
  ULONG debugFrame = 0;
#endif


MovieFile::MovieFile()
         : movieFileInfo_p(0), storageSystemFileHandle_p(0),
           codecModule(0), sendCodecMessage(0), codecHandle(0)
{
  memset(&movieHeader, '\0', sizeof(movieHeader));
  memset(&trackInfoList, '\0', sizeof(trackInfoList));
  memset(&videoHeader, '\0', sizeof(videoHeader));
  memset(&palette, '\0', sizeof(palette));
  memset(&extendedBitmapHeader, '\0', sizeof(extendedBitmapHeader));
  memset(&audioHeader, '\0', sizeof(audioHeader));
  memset(&extendedInfo_p, '\0', sizeof(extendedInfo_p));
  memset(&codecAssoc, '\0', sizeof(codecAssoc));
  memset(&codecOpen, '\0', sizeof(codecOpen));
  memset(&codecIniFileInfo, '\0', sizeof(codecIniFileInfo));
  memset(&sourceHeader, '\0', sizeof(sourceHeader));
  memset(&destinationHeader, '\0', sizeof(destinationHeader));

  codecAssoc.pCodecOpen = &codecOpen;
  codecAssoc.pCODECIniFileInfo = &codecIniFileInfo;

  extendedInfo_p.ulStructLen = sizeof(MMEXTENDINFO);
  extendedInfo_p.ulBufSize = sizeof(MMEXTENDINFO);
  extendedInfo_p.pCODECAssoc = &codecAssoc;
}

void MovieFile::setMovieFileInfo(PMMIOINFO movieFileInfo)
{
  movieFileInfo_p = movieFileInfo;
}

LONG MovieFile::dispatchMessage(USHORT usMsg, LONG lParam1, LONG lParam2)
{
  switch (usMsg)
  {
    case MMIOM_IDENTIFYFILE:
      return identifyFile((PSZ)lParam1, (HMMIO)lParam2);

    case MMIOM_QUERYHEADERLENGTH:
      return queryHeaderLength();

    case MMIOM_OPEN:
      return open((PSZ)lParam1);

    case MMIOM_SET:
      return set((PMMEXTENDINFO)lParam1, lParam2);

    case MMIOM_GETHEADER:
      return getHeader((PVOID)lParam1, lParam2);

    case MMIOM_SETHEADER:
      return setHeader((PVOID)lParam1, lParam2);

    case MMIOM_READ:
      return read((PSZ)lParam1, lParam2);

    case MMIOM_MULTITRACKREAD:
      return multiTrackRead((PMMMULTITRACKREAD)lParam1);

    case MMIOM_DECOMPRESS:
       return decompress((PMMDECOMPRESS)lParam1);

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
LONG MovieFile::queryHeaderLength()
{
  return (sizeof(MMMOVIEHEADER) + sizeof(trackInfoList));
}

/************************************************************
 * OPEN
 ************************************************************/
LONG MovieFile::open(PSZ fileName)
{
  FOURCC fccStorageSystem;               /* SS I/O Proc FOURCC       */
  MMIOINFO  mmioinfoSS;                  /* I/O info block for SS ref*/

  if (movieFileInfo_p->ulFlags & (MMIO_WRITE | MMIO_READWRITE))
    return (MMIO_ERROR);

 // if (showExpiredMessage())
 //   return (MMIO_ERROR);

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
  if (!movieFileInfo_p->fccChildIOProc)
  {
    if (movieFileInfo_p->ulFlags & MMIO_CREATE)
      {
      if (mmioDetermineSSIOProc(fileName, movieFileInfo_p, &fccStorageSystem, NULL))
        fccStorageSystem = FOURCC_DOS; /* SS not recognized from file name */
      }
    else
      {
      if (mmioIdentifyStorageSystem(fileName, movieFileInfo_p, &fccStorageSystem))
        return (MMIO_ERROR);
      };
    if (!fccStorageSystem)
      return (MMIO_ERROR);
    else
      movieFileInfo_p->fccChildIOProc = fccStorageSystem;
  }

  /**************************************************************
   * Open the file
   **************************************************************/
  memmove(&mmioinfoSS, movieFileInfo_p, sizeof(MMIOINFO));
  mmioinfoSS.pIOProc = NULL;
  mmioinfoSS.fccIOProc = movieFileInfo_p->fccChildIOProc;
  mmioinfoSS.ulFlags |= MMIO_NOIDENTIFY; /* Avoid recursive loop */
  if (mmioinfoSS.ulFlags & MMIO_READWRITE)
    mmioinfoSS.ulFlags |= MMIO_READ; /* Set read mode if readwrite requested.  Never get here due to above. */
  mmioinfoSS.ulFlags &= ~MMIO_WRITE & ~MMIO_READWRITE; /* Force non-writable open */

  storageSystemFileHandle_p = mmioOpen(fileName, &mmioinfoSS, mmioinfoSS.ulFlags);

  /************************************************************
   * Check if a DELETE was requested - mmioOpen returns a 1,
   *    so we much check this separately
   ************************************************************/
  if (movieFileInfo_p->ulFlags & MMIO_DELETE)
  {
    if (!storageSystemFileHandle_p)
      {
        movieFileInfo_p->ulErrorRet = MMIOERR_DELETE_FAILED;
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
 * SET
 ************************************************************/
LONG MovieFile::set(PMMEXTENDINFO extendedInfo, LONG flags)
{
  const ULONG readModeFlags = MMIO_NORMAL_READ | MMIO_SCAN_READ | MMIO_REVERSE_READ;

  if (flags & MMIO_SET_EXTENDEDINFO)
  {
    if (extendedInfo->ulFlags & MMIO_TRACK)
      extendedInfo_p.ulTrackID = extendedInfo->ulTrackID;

    if (extendedInfo->ulFlags & readModeFlags)
    {
      extendedInfo_p.ulFlags &= ~readModeFlags;
      extendedInfo_p.ulFlags |= extendedInfo->ulFlags & readModeFlags;
    }
  }
  else if (flags & MMIO_QUERY_EXTENDEDINFO_BASE)
  {
    memcpy(extendedInfo, &extendedInfo_p, sizeof(MMEXTENDINFO));
  }
  else if (flags & MMIO_QUERY_EXTENDEDINFO_ALL)
  {
    extendedInfo->pCODECAssoc->pCodecOpen = extendedInfo_p.pCODECAssoc->pCodecOpen;
    extendedInfo->pCODECAssoc->pCODECIniFileInfo = extendedInfo_p.pCODECAssoc->pCODECIniFileInfo;
  }
  return (MMIO_SUCCESS);
}

/************************************************************
 * DECOMPRESS
 ************************************************************/
LONG MovieFile::decompress(PMMDECOMPRESS decompress)
{
#ifdef DEBUGGING_IOPROC
  debugFrame++;
#endif

  LONG rc = sendCodecMessage(&codecHandle, MMIOM_CODEC_DECOMPRESS, (LONG)decompress, 0);

  return (rc);
}

/************************************************************
 * CLOSE
 ************************************************************/
LONG MovieFile::close(USHORT closeFlags)
{
  if (sendCodecMessage)
  {
    LONG rc = sendCodecMessage(&codecHandle, MMIOM_CODEC_CLOSE, 0, 0);
    if (rc != MMIO_SUCCESS)
      return (rc);
  }

  return mmioClose(storageSystemFileHandle_p, closeFlags);
}

/************************************************************
 * DEFAULT RESPONSE
 ************************************************************/
LONG MovieFile::defaultResponse(USHORT usMsg, LONG lParam1, LONG lParam2)
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

/************************************************************
 * OPEN CODEC
 ************************************************************/
LONG MovieFile::openCodec()
{
  ULONG codecDepthFlag;

  codecIniFileInfo.ulStructLen = sizeof(codecIniFileInfo);
  codecIniFileInfo.fcc = FOURCC_IOPROC;
  codecIniFileInfo.ulCompressType = extendedBitmapHeader.BMPInfoHeader2.ulCompression;
  codecIniFileInfo.ulCapsFlags = CODEC_DECOMPRESS | CODEC_ORIGIN_LOWERLEFT;

  switch (extendedBitmapHeader.BMPInfoHeader2.cBitCount)
  {
    case 1:
      codecDepthFlag = CODEC_4_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_PALETTIZED;
      break;
    case 2:
      codecDepthFlag = CODEC_4_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_PALETTIZED;
      break;
    case 4:
      codecDepthFlag = CODEC_4_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_PALETTIZED;
      break;
    case 8:
      codecDepthFlag = CODEC_8_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_PALETTIZED;
      break;
    case 16:
      codecDepthFlag = CODEC_16_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_RGB_5_6_5;
      break;
    case 24:
      codecDepthFlag = CODEC_24_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_RGB_24;
      break;
    case 32:
      codecDepthFlag = CODEC_24_BIT_COLOR;
      destinationHeader.ulColorEncoding = MMIO_RGB_24;
      break;
  };
  codecIniFileInfo.ulCapsFlags |= codecDepthFlag;

  sendCodecMessage = mmioLoadCODECProc(&codecIniFileInfo, &codecModule,
    MMIO_MATCHFOURCC | MMIO_MATCHCOMPRESSTYPE | MMIO_MATCHCAPSFLAGS);

  if (!sendCodecMessage)
    return (MMIO_ERROR);

  codecOpen.pSrcHdr = &sourceHeader;
  sourceHeader.ulStructLen = sizeof(CODECVIDEOHEADER);
  sourceHeader.cx = extendedBitmapHeader.BMPInfoHeader2.cx;
  sourceHeader.cy = extendedBitmapHeader.BMPInfoHeader2.cy;
  sourceHeader.cPlanes = extendedBitmapHeader.BMPInfoHeader2.cPlanes;
  sourceHeader.cBitCount = extendedBitmapHeader.BMPInfoHeader2.cBitCount;
  sourceHeader.ulColorEncoding = MMIO_COMPRESSED;
  sourceHeader.genpal.ulStartIndex = videoHeader.genpalVideo.ulStartIndex;
  sourceHeader.genpal.ulNumColors = videoHeader.genpalVideo.ulNumColors;
  sourceHeader.genpal.prgb2Entries = videoHeader.genpalVideo.prgb2Entries;

  codecOpen.pDstHdr = &destinationHeader;
  destinationHeader.ulStructLen = sizeof(CODECVIDEOHEADER);
  destinationHeader.cx = extendedBitmapHeader.BMPInfoHeader2.cx;
  destinationHeader.cy = extendedBitmapHeader.BMPInfoHeader2.cy;
  destinationHeader.cPlanes = extendedBitmapHeader.BMPInfoHeader2.cPlanes;

  /* Override output color space if R565 is preferred by the CODEC (gets rt21 to work) */
  if (codecIniFileInfo.fccPreferredFormat == FOURCC_R565 ||
      codecIniFileInfo.fccPreferredFormat == FOURCC_R555 ||
      codecIniFileInfo.fccPreferredFormat == FOURCC_YUV9)
  {
    codecDepthFlag = CODEC_16_BIT_COLOR;
    destinationHeader.cBitCount = 16;
    destinationHeader.genpal.ulNumColors = 1 << 16;
    destinationHeader.ulColorEncoding = MMIO_RGB_5_6_5;
  }
  else
  {
    destinationHeader.cBitCount = extendedBitmapHeader.BMPInfoHeader2.cBitCount;
  }

  codecOpen.ulFlags = codecDepthFlag | CODEC_DECOMPRESS | CODEC_ORIGIN_LOWERLEFT;
  codecOpen.ulFlags |= (codecIniFileInfo.ulCapsFlags &
    (CODEC_MULAPERTURE | CODEC_SELFHEAL | CODEC_WINDOW_CLIPPING | CODEC_PALETTE_TRANS));

  LONG rc = sendCodecMessage(&codecHandle, MMIOM_CODEC_OPEN, (LONG)&codecOpen, 0);
  if (rc != MMIO_SUCCESS)
    return (rc);

  extendedInfo_p.ulNumCODECs = 1;

  return (MMIO_SUCCESS);
}

HMMIO MovieFile::storageSystemFileHandle()
{
  return storageSystemFileHandle_p;
}

VOID MovieFile::setError(ULONG error)
{
  movieFileInfo_p->ulErrorRet = error;
}

ULONG MovieFile::flags()
{
  return movieFileInfo_p->ulFlags;
}

FOURCC MovieFile::childIOProc()
{
  return movieFileInfo_p->fccChildIOProc;
}

ULONG MovieFile::extendedFlags()
{
  return extendedInfo_p.ulFlags;
}

ULONG MovieFile::track()
{
  return extendedInfo_p.ulTrackID;
}

/************************************************************
 * Destructor
 ************************************************************/
MovieFile::~MovieFile()
{
}

