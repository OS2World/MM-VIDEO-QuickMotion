#include "jpegcode.h"

#include <string.h>

#include "jfifcode.hpp"

/************************************************************
 * OPEN
 ************************************************************/
LONG JFIFCodec::open(PCODECOPEN codecOpen)
{
  LONG returnCode;

  returnCode = ImageCodec::open(codecOpen);
  if (returnCode != MMIO_SUCCESS)
    return (returnCode);

  /************************************************************
   * DECOMPRESS mode, set up JPEG decompression
   ************************************************************/
  /************************************************************
   * Set up JPEG error handling code
   ************************************************************/
  if (codecOpen_p->ulFlags & CODEC_DECOMPRESS)
    jpegDecompress.err = jpeg_std_error(&mmioErrorManager);
//  if (codecOpen_p->ulFlags & CODEC_COMPRESS)
//   jpegCompress.err = jpeg_std_error(&mmioErrorManager);
  mmioErrorManager.error_exit = handleJPEGError;

  try
  {
    /************************************************************
     * Create JPEG decompression/compression objects
     ************************************************************/
    if (codecOpen_p->ulFlags & CODEC_DECOMPRESS)
    {
      jpeg_create_decompress(&jpegDecompress);
      initMMIOSourceManager(&jpegDecompress, &mmDecompress_p);
    }
//    if (codecOpen_p->ulFlags & CODEC_COMPRESS)
//    {
//      jpeg_create_compress(&jpegCompress);
//      initMMIODestManager(&jpegCompress, &mmCompress_p);
//    }

    /*********************************************************
     * Allocate the whole-bitmap buffer
     * This must be done before jpeg_start_decompress, which
     * must be called to get the output colormap in case
     * we are quantizing colors
     *********************************************************/
//    bitmap = (*jpegDecompress.mem->alloc_sarray)
//             ((j_common_ptr) &jpegDecompress, JPOOL_PERMANENT, bytesPerLine, height());

    /*********************************************************
     * Call ImageCodec method to set up this source for color
     * conversion
     *********************************************************/
//    returnCode = setupSource((PBYTE)(bitmap[0]));

    /*********************************************************
     * Allocate a working buffer for input
     *********************************************************/
    workLines = (*jpegDecompress.mem->alloc_sarray)
                ((j_common_ptr) &jpegDecompress, JPOOL_PERMANENT, bytesPerLine, height());

    rgbBytesPerLine = ((width() * 3 + 3) >> 2) << 2;
  }
  catch(int)
  {
    return (MMIO_ERROR);
  }

  return (MMIO_SUCCESS);
}

/************************************************************
 * DECOMPRESS
 ************************************************************/
LONG JFIFCodec::decompress(PMMDECOMPRESS mmDecompress)
{
  LONG col; /* Index variable */
  LONG linesRead, bytesRead;
  LONG returnCode;
  PGENPAL palette = 0;

  mmDecompress_p = mmDecompress;

  try
  {
    /************************************************************
     * Read JPEG header
     ************************************************************/
    jpeg_read_header(&jpegDecompress, TRUE);

    /************************************************************
     * Set decompression parms
     ************************************************************/
    jpegDecompress.dct_method = JDCT_FLOAT;

    /*********************************************************
     * Start JPEG decompression
     *********************************************************/
    jpeg_start_decompress(&jpegDecompress);

    /************************************************************
     * For quantized color output, write the colormap
     ************************************************************/
    if (jpegDecompress.quantize_colors)
    {
      palette = (PGENPAL)&(((PMMVIDEODECOMPRESS)(mmDecompress->pRunTimeInfo))->genpalVideo);
      palette->ulStartIndex = 0;
      palette->ulNumColors = jpegDecompress.actual_number_of_colors;

      for (col = 0; col < palette->ulNumColors; col++)
      {
        palette->prgb2Entries[col].bBlue = jpegDecompress.colormap[0][col];
        palette->prgb2Entries[col].bGreen = jpegDecompress.colormap[1][col];
        palette->prgb2Entries[col].bRed = jpegDecompress.colormap[2][col];
        palette->prgb2Entries[col].fcOptions = 0;
      };
    };

    /*********************************************************
     * Read in the entire decompressed image
     *********************************************************/
    PBYTE outPosition = (PBYTE)mmDecompress->pDstBuf;
    for (ULONG line = 0; jpegDecompress.output_scanline < jpegDecompress.output_height; line++)
    {
      linesRead = jpeg_read_scanlines(&jpegDecompress, workLines, jpegDecompress.output_height);
      if (jpegDecompress.out_color_space == JCS_GRAYSCALE)
      {
        for (linesRead--; linesRead >= 0; linesRead--)
        {
          PBYTE outPositionThisLine = outPosition;
          for (LONG j = 0; j < jpegDecompress.output_width; j++)
          {
            BYTE colorComponent = *(workLines[linesRead] + j);
            *outPositionThisLine++ = colorComponent;
            *outPositionThisLine++ = colorComponent;
            *outPositionThisLine++ = colorComponent;
          }
          outPosition += rgbBytesPerLine;
        }
      }
      else
      {
      bytesRead = bytesPerLine * linesRead;
      memcpy(outPosition, workLines[0], bytesRead);
      outPosition += bytesRead;
      }
    };  /* Done with image */

    /*********************************************************
     * Convert BGR data to the screen color space
     *********************************************************/
//    returnCode = copyToDestination((PBYTE)(mmDecompress->pDstBuf));

    /*********************************************************
     * Call JPEG cleanup method
     *********************************************************/
    jpeg_finish_decompress(&jpegDecompress);
  }
  catch(int)
  {
    close();
    return (MMIO_ERROR);
  }

  return (MMIO_SUCCESS);
}

/************************************************************
 * COMPRESS
 ************************************************************/
//LONG JFIFCodec::compress(PMMCOMPRESS mmCompress)
//{
//  mmCompress_p = mmCompress;

 /************************************************************
 * SET HEADER
 ************************************************************/
//  PMMIMAGEHEADER pMMImgHdr;
//  USHORT bitCount;
//  ULONG inputBytesPerLine;

  /************************************************************
   * Only allow this function if we are in WRITE mode
   * And only if we have not already set the header
   ************************************************************/
//  if (!(codecOpen_p->ulFlags & CODEC_COMPRESS))

//    return (MMIO_ERROR);

  /************************************************************
   * Header is translated.
   ************************************************************/
  /************************************************************
   * Create local pointer media specific structure.
   ************************************************************/

  /************************************************************
   * Check for validity of header contents supplied
   ************************************************************
   *  --  Length must be that of the standard header
   *  --  NO Compression
   *      1 plane
   *      24, 8, 4 or 1 bpp
   ************************************************************/
//  bitCount = pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cBitCount;
//  if ((pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.ulCompression != BCA_UNCOMP) ||
//      (pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cPlanes != 1) ||
//      (!((bitCount == 24) || (bitCount == 8))))
//  {
//    return (MMIO_ERROR);
//  }

  /********************************************************
   * Set JPEG header values
   ********************************************************/
//  jpegCompress.input_components = 3;
//  jpegCompress.in_color_space = JCS_RGB;

//  jpeg_set_defaults(&jpegCompress);

//  jpegCompress.dct_method = JDCT_FLOAT;
//  jpegCompress.image_width =
//    pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cx;
//  jpegCompress.image_height =
//    pMMImgHdr->mmXDIBHeader.BMPInfoHeader2.cy;

//  inputBytesPerLine =
//    ((jpegCompress.image_width * bitCount / 8 + 3) / 4) * 4;
//  bytesPerLine =
//    inputBytesPerLine * jpegCompress.input_components * 8 / bitCount;
//  bitCount = bitCount;

//  currentLine = jpegCompress.image_height - 1;  <- currentLine no longer defined
//  currentByte = 0;

  /*********************************************************
   * Allocate the whole-bitmap buffer
   *********************************************************/
//  if (!bitmap)
//    bitmap = (*jpegCompress.mem->alloc_sarray)
//                       ((j_common_ptr) &jpegCompress,
//                       JPOOL_IMAGE,
//                       bytesPerLine,
//                       jpegCompress.image_height);

  /************************************************************
   * Store palette for compression if not full-color input
   ************************************************************/
//  if (bitCount < 24)
//    memcpy((PVOID)palette, (PVOID)pMMImgHdr->bmiColors,
//           sizeof(palette));

  /************************************************************
   * Flag that MMIOM_SETHEADER has been done.  It can only
   *    be done ONCE for a file.  All future attempts will
   *    be flagged as errors.
   ************************************************************/

//  return (sizeof(MMIMAGEHEADER));

// ************* WRITE

//  PSZ start; /* Original position of input buffer */


  /************************************************************
   * Translation is on.
   ************************************************************/
  /************************************************************
   * Write the data into the image buffer.
   ************************************************************/

//  while (currentLine >= 0) /* Transfer a pel */
// {
//    switch (bitCount)
//    {
//      case 24:
//      {
//        for (LONG newBufferLength = bufferLength - 3;
//            bufferLength > newBufferLength; bufferLength--)
//        {
//          *(bitmap[currentLine] +
//                             currentByte++) = *buffer++;
//        }
//        break;
//      }

//      case 8:
//      {
//        RGB2 color = palette[*buffer++];
//        *(bitmap[currentLine] +
//                             currentByte++) = color.bBlue;
//        *(bitmap[currentLine] +
//                             currentByte++) = color.bGreen;
//        *(bitmap[currentLine] +
//                             currentByte++) = color.bRed;
//        bufferLength -= 1;
//        break;
//      }

//      default:
//        return (MMIO_ERROR);
//    } /* End switch (bitCount) */
//    if (currentByte >= bytesPerLine)
//    {
//      currentLine--;
//      currentByte = 0;
//    }
//  } /* End while (bufferLength) */


/************************************************************
 * COMPRESS
 ************************************************************/
//  jpeg_start_compress(&jpegCompress, TRUE);

//  while (jpegCompress.next_scanline < jpegCompress.image_height)
//    jpeg_write_scanlines(&jpegCompress, &bitmap[jpegCompress.next_scanline], 1);

//  jpeg_finish_compress(&jpegCompress);

//  return (MMIO_SUCCESS);
//}

/************************************************************
 * CLOSE
 ************************************************************/
LONG JFIFCodec::close()
{
  if (codecOpen_p->ulFlags & CODEC_DECOMPRESS)
    jpeg_destroy_decompress(&jpegDecompress);
//  if (codecOpen_p->ulFlags & CODEC_COMPRESS)
//    jpeg_destroy_compress(&jpegCompress);

  return (ImageCodec::close());
}

/************************************************************
 * HANDLE JPEG ERROR
 ************************************************************/
VOID JFIFCodec::handleJPEGError(j_common_ptr jpegInfo)
{
  (*jpegInfo->err->output_message) (jpegInfo);
  throw 0;
}


