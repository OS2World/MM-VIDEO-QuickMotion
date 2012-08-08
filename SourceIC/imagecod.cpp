#include <limits.h>
#include <string.h>
#include <fourcc.h>

#include "imagecod.hpp"

LONG ImageCodec::dispatchMessage(USHORT usMsg, LONG lParam1, LONG lParam2)
{
  switch (usMsg)
  {
    case MMIOM_CODEC_QUERYNAMELENGTH:
      return queryNameLength((PLONG)lParam1);

    case MMIOM_CODEC_QUERYNAME:
      return queryName((PSZ)lParam1, (PLONG)lParam2);

    case MMIOM_CODEC_OPEN:
      return open((PCODECOPEN)lParam1);

    case MMIOM_CODEC_DECOMPRESS:
      return decompress((PMMDECOMPRESS)lParam1);

//    case MMIOM_CODEC_COMPRESS:
//      return compress((PMMCOMPRESS)lParam1);

    case MMIOM_CODEC_CLOSE:
      return close();

    default:
      return defaultResponse(usMsg, lParam1, lParam2);
  }
}

/************************************************************
 * QUERY NAME LENGTH
 ************************************************************/
LONG ImageCodec::queryNameLength(PLONG nameLength)
{

  if (GetFormatStringLength(CODEC_ID, nameLength))
    return (MMIO_ERROR);

  return (MMIO_SUCCESS);
}

/************************************************************
 * QUERY NAME
 ************************************************************/
LONG ImageCodec::queryName(PSZ name, PLONG nameLength)
{
  LONG lBytesCopied;

  lBytesCopied = GetFormatString(CODEC_ID, name, *nameLength);

  return (lBytesCopied);
}

/************************************************************
 * OPEN
 ************************************************************/
LONG ImageCodec::open(PCODECOPEN codecOpen)
{
  codecOpen_p = codecOpen;

  ULONG imageBytesPerLine = width() * depth() >> 3;
  bytesPerLine = ((imageBytesPerLine + 3) >> 2) << 2;

  return (MMIO_SUCCESS);
}

/************************************************************
 * CLOSE
 ************************************************************/
LONG ImageCodec::close()
{
  DiveFreeImageBuffer(diveHandle, diveSourceBuffer);
  return (DiveClose(diveHandle));
}

/************************************************************
 * DEFAULT RESPONSE
 ************************************************************/
LONG ImageCodec::defaultResponse(USHORT usMsg, LONG lParam1, LONG lParam2)
{
  return (MMIO_ERROR);
}

/************************************************************
 * SETUP SOURCE
 ************************************************************/
LONG ImageCodec::setupSource(PBYTE sourceBuffer)
{
  LONG returnCode;

  diveHandle = 0;
  returnCode = DiveOpen(&diveHandle, TRUE, 0);
  if (returnCode)
    return (returnCode);

  sourceColorSpace = FOURCC_BGR3;

  imageRectangle.xLeft = imageRectangle.yBottom = 0;
  imageRectangle.xRight = width();
  imageRectangle.yTop = height();

  setupBlitter.ulStructLen = sizeof(SETUP_BLITTER);
  setupBlitter.fInvert = FALSE;
  setupBlitter.fccSrcColorFormat = sourceColorSpace;
  setupBlitter.ulSrcWidth = setupBlitter.ulDstWidth = width();
  setupBlitter.ulSrcHeight = setupBlitter.ulDstHeight = height();
  setupBlitter.ulSrcPosX = setupBlitter.lDstPosX = 0;
  setupBlitter.ulSrcPosY = setupBlitter.lDstPosY = 0;
  setupBlitter.ulDitherType = 0;
  setupBlitter.fccDstColorFormat = FOURCC_SCRN;
  setupBlitter.lScreenPosX = setupBlitter.lScreenPosY = 0;
  setupBlitter.ulNumDstRects = 1;
  setupBlitter.pVisDstRects = &imageRectangle;

  returnCode = DiveSetupBlitter(diveHandle, &setupBlitter);

  diveSourceBuffer = 0;
  returnCode = DiveAllocImageBuffer(diveHandle, &diveSourceBuffer, sourceColorSpace,
    width(), height(), bytesPerLine, sourceBuffer);

  return (returnCode);
}

/************************************************************
 * COPY TO DESTINATION
 ************************************************************/
LONG ImageCodec::copyToDestination(PBYTE destinationBuffer)
{
  ULONG diveDestinationBuffer = 0;
  ULONG returnCode;

  returnCode = DiveAllocImageBuffer(diveHandle, &diveDestinationBuffer, FOURCC_SCRN,
    width(), height(), 0, destinationBuffer);
  returnCode = DiveBlitImage(diveHandle, diveSourceBuffer, diveDestinationBuffer);
  returnCode = DiveFreeImageBuffer(diveHandle, diveDestinationBuffer);

  return (returnCode);
}

/************************************************************
 * HELPER FUNCTIONS
 ************************************************************/
ULONG ImageCodec::width() const
{
  return ((PCODECVIDEOHEADER)(codecOpen_p->pDstHdr))->cx;
}

ULONG ImageCodec::height() const
{
  return ((PCODECVIDEOHEADER)(codecOpen_p->pDstHdr))->cy;
}

ULONG ImageCodec::depth() const
{
  return ((PCODECVIDEOHEADER)(codecOpen_p->pDstHdr))->cBitCount;
}

/************************************************************
 * STATIC FUNCTIONS
 ************************************************************/


/**************************************************************************
 *   GetFormatString                                                     **
 **************************************************************************
 *
 * ARGUMENTS:
 *
 *     lSearchId         - Table search key
 *     pszFormatString   - Address where to return string.
 *                         If null, then string is not returned.
 *     lBytes            - Number of bytes to copy.
 *
 * RETURN:
 *
 *     Returns 0 if string not found, or the number of characters (bytes)
 *     copied to the callers buffer.
 *     Note, returned string is not ASCII nul terminated
 *
 * DESCRIPTION:
 *
 *     This function will retrieve the format string for the specified
 *     IOProc from the resource file that contains the strings.
 *
 ***************************************************************************/
LONG ImageCodec::GetFormatString (LONG lSearchId,
                                  PSZ  pszFormatString,   /* Null, or dest address     */
                                  LONG lBytes)            /* Caller provided maximum   */
{
   PVOID   pResourceData;
   CHAR    *pTemp;
   LONG    lStringLen;      /* Length of format string  */
   LONG    lRetVal = 0;     /* Function return value    */
   SHORT   sSearchTemp;

   if(DosGetResource(moduleHandle, RT_RCDATA, NAME_TABLE, &pResourceData))
     return (MMIO_ERROR);

   lStringLen = 0;
   pTemp = (CHAR *)pResourceData;

   while (pTemp)
      {
      memmove(&sSearchTemp, pTemp, sizeof(SHORT));

      if (sSearchTemp == 0L)
         {
         break;  /* End of table, search item not found */
         }

      if (sSearchTemp == lSearchId)   /* Search item found?               */
         {
         pTemp += sizeof(SHORT);      /* Bypass numeric search id         */
         lStringLen = strlen(pTemp);  /* Determine length of found string */
         if (lStringLen >= lBytes)    /* Truncate, if necessary           */
            {
            if (lBytes > 0)
               lStringLen = lBytes;   /* Number of characters to return   */
            else
               {
               /* Callers buffer has zero size.  Cannot return any data   */
               lRetVal = 0;           /* Size of returned data            */
               break;                 /* Done searching                   */
               }
            }
         if (pszFormatString != NULL)
           memcpy(pszFormatString, pTemp, lStringLen); /* Copy string to caller */

         lRetVal = lStringLen;        /* Function return value            */
         break;                       /* We're done searching             */
         }

      pTemp += sizeof(SHORT);
      pTemp += (strlen(pTemp) + 1);   /* Advance to next search key       */
      }

   DosFreeResource(pResourceData);

   return (lRetVal);  /* Zero or strlen */
}


/**************************************************************************
 *   GetFormatStringLength                                               **
 **************************************************************************
 *
 * ARGUMENTS:
 *
 *     lSearchId         - Table search key
 *     plNameLength      - Address where to return string length.
 *
 * RETURN:
 *
 *     Length of the format string not including the terminating '\0'.
 *     That is, the same value as would be returned from strlen().
 *
 * DESCRIPTION:
 *
 *     This function will retrieve the length of the format string
 *     for the specified IOProc from the resource file that contains
 *     the strings.
 *
 ***************************************************************************/
LONG ImageCodec::GetFormatStringLength (LONG  lSearchId,
                                        PLONG plNameLength)
{
   LONG  lStringSize;
   LONG  lRetVal;

   lStringSize = GetFormatString(lSearchId, NULL, LONG_MAX);
   if (lStringSize > 0)             /* String found?                      */
      {
      *plNameLength = lStringSize;  /* yes, return strlen                 */
      lRetVal = 0;                  /* and indicate success to caller     */
      }
   else
      {
      *plNameLength = 0;            /* no, error.  Return zero for length */
      lRetVal = lStringSize;        /* and error code from called routine */
      }
   return (lRetVal);
}

