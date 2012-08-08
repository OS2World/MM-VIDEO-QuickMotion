#include "kpcdcode.h"

#include <string.h>

#include "kpcdclas.hpp"

/************************************************************
 * OPEN
 ************************************************************/
LONG KPCDCodec::open(PCODECOPEN codecOpen)
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
LONG KPCDCodec::decompress(PMMDECOMPRESS mmDecompress)
{
  mmDecompress_p = mmDecompress;

  PCODECVIDEOHEADER sourceHeader = (PCODECVIDEOHEADER)codecOpen_p->pSrcHdr;

  int imageWidth = width();
  int halfImageWidth = imageWidth >> 1;
  int imageHeight = height();
  int halfImageHeight = imageHeight >> 1;
  PBYTE source = (PBYTE)mmDecompress_p->pSrcBuf;
  PBYTE destinationY = (PBYTE)mmDecompress_p->pDstBuf;
  PBYTE destinationC1 = destinationY + imageWidth * imageHeight;
  PBYTE destinationC2 = destinationC1 + halfImageWidth * halfImageHeight;
  int i, j; /* index variables */

  /*********************************************************
   * Copy source buffer to destination buffer
   *********************************************************/
  for (i = halfImageHeight; i > 0; i--)
  {
    /* Y */
    for (j = 2; j > 0; j--)
    {
      memcpy(destinationY, source, imageWidth);
      destinationY += imageWidth;
      source += imageWidth;
    }

    /* C2 */
    memcpy(destinationC2, source, halfImageWidth);
    destinationC2 += halfImageWidth;
    source += halfImageWidth;

    /* C1 */
    memcpy(destinationC1, source, halfImageWidth);
    destinationC1 += halfImageWidth;
    source += halfImageWidth;
  }

  return (MMIO_SUCCESS);
}

/************************************************************
 * CLOSE
 ************************************************************/
LONG KPCDCodec::close()
{
  return (ImageCodec::close());
}

