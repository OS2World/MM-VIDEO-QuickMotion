#include "rawcodec.h"

#include <string.h>

#include "rawclass.hpp"

/************************************************************
 * OPEN
 ************************************************************/
LONG RawCodec::open(PCODECOPEN codecOpen)
{
  LONG returnCode;

  returnCode = ImageCodec::open(codecOpen);
  if (returnCode != MMIO_SUCCESS)
    return (returnCode);

  return (MMIO_SUCCESS);
}

/************************************************************
 * DECOMPRESS
 ************************************************************/
LONG RawCodec::decompress(PMMDECOMPRESS mmDecompress)
{
  mmDecompress_p = mmDecompress;

  PCODECVIDEOHEADER sourceHeader = (PCODECVIDEOHEADER)codecOpen_p->pSrcHdr;

  PBYTE source = (PBYTE)mmDecompress_p->pSrcBuf;
  PBYTE destination = (PBYTE)mmDecompress_p->pDstBuf;
  int i, j; /* index variables */
  RGB2 color;
  BYTE index;
  BOOL firstNybble = TRUE;
  BYTE firstByte, secondByte;
  ULONG pad = (sourceHeader->cBitCount == 8 && sourceHeader->ulColorEncoding == MMIO_PALETTIZED ?
               (4 - ((sourceHeader->cx) % 4)) % 4 :
               (4 - ((sourceHeader->cx * 3) % 4)) % 4);

  /*********************************************************
   * Copy source buffer to destination buffer
   *********************************************************/
  switch (sourceHeader->cBitCount)
  {
    case 4:
       for (i = sourceHeader->cy; i; i--)
       {
         for (j = sourceHeader->cx; j; j--)
         {
           index = (firstNybble ? *source >> 4 : *source++ & 0x0F);
           color = sourceHeader->genpal.prgb2Entries[index];
           *destination++ = color.bBlue;
           *destination++ = color.bGreen;
           *destination++ = color.bRed;
           firstNybble = !firstNybble;
         }
         destination += pad; /* Position to next longint boundary */
       }
       break;
    case 8:
       if (sourceHeader->ulColorEncoding == MMIO_PALETTIZED)
         for (i = sourceHeader->cy; i; i--)
         {
           for (j = sourceHeader->cx; j; j--)
             *destination++ = *source++;
           destination += pad; /* Position to next longint boundary */
         }
       else
         for (i = sourceHeader->cy; i; i--)
         {
           for (j = sourceHeader->cx; j; j--)
           {
             color = sourceHeader->genpal.prgb2Entries[*source++];
             *destination++ = color.bBlue;
             *destination++ = color.bGreen;
             *destination++ = color.bRed;
           }
           destination += pad; /* Position to next longint boundary */
         }
       break;
    case 16:
       for (i = sourceHeader->cy; i; i--)
       {
         for (j = sourceHeader->cx; j; j--)
         {
           firstByte = *source++;
           secondByte = *source++;
           *destination++ = secondByte << 3; /*Blue */
           *destination++ = ((firstByte & 0x03) << 6) | ((secondByte & 0xE0) >> 2); /*Green*/
           *destination++ = (firstByte & 0x7C) << 1; /* Red */
         }
         destination += pad; /* Position to next longint boundary */
         if (sourceHeader->cx & 0x00000001) /* Skip odd input byte pair */
           source += 2;
       }
       break;
    case 24:
       for (i = sourceHeader->cy; i; i--)
       {
         for (j = sourceHeader->cx; j; j--)
         {
           color.bRed = *source++;
           color.bGreen = *source++;
           *destination++ = *source++;
           *destination++ = color.bGreen;
           *destination++ = color.bRed;
         }
         destination += pad; /* Position to next longint boundary */
       }
       break;
     default:
       return (MMIO_ERROR);
  }

  return (MMIO_SUCCESS);
}

/************************************************************
 * CLOSE
 ************************************************************/
LONG RawCodec::close()
{
  return (ImageCodec::close());
}

