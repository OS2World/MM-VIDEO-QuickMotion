#include "jpegcode.h"

/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

METHODDEF void
initSource (j_decompress_ptr jpegDecompress)
{
  MMIOSourceManager *src = (MMIOSourceManager *) jpegDecompress->src;

  /* We reset the empty-input-file flag for each image,
   * but we don't clear the input buffer.
   * This is correct behavior for reading a series of images from one source.
   */
  src->startOfFile = TRUE;
}


/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */

METHODDEF boolean
fillInputBuffer (j_decompress_ptr jpegDecompress)
{
  MMIOSourceManager *src = (MMIOSourceManager *)jpegDecompress->src;

  if ((*(src->mmDecompress))->ulSrcBufLen == 0)
    if (src->startOfFile)     /* Treat empty input file as fatal error */
      ERREXIT(jpegDecompress, JERR_INPUT_EMPTY);

  src->jpegSourceManager.next_input_byte = (const JOCTET*)((*(src->mmDecompress))->pSrcBuf);
  src->jpegSourceManager.bytes_in_buffer = (*(src->mmDecompress))->ulSrcBufLen;
  src->startOfFile = FALSE;

  return TRUE;
}


/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

METHODDEF void
skipInputData (j_decompress_ptr jpegDecompress, long numBytes)
{
  MMIOSourceManager *src = (MMIOSourceManager *) jpegDecompress->src;

  if (numBytes > 0) {
    while (numBytes > (long)src->jpegSourceManager.bytes_in_buffer) {
      numBytes -= (long)src->jpegSourceManager.bytes_in_buffer;
      (void)fillInputBuffer(jpegDecompress);
      /* note we assume that fillInputBuffer will never return FALSE,
       * so suspension need not be handled.
       */
    }
    src->jpegSourceManager.next_input_byte += (size_t)numBytes;
    src->jpegSourceManager.bytes_in_buffer -= (size_t)numBytes;
  }
}

/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

METHODDEF void
termSource (j_decompress_ptr jpegDecompress)
{
  /* no work necessary here */
}

GLOBAL void
initMMIOSourceManager(j_decompress_ptr jpegDecompress, PMMDECOMPRESS *mmDecompress)
{
  MMIOSourceManager *src;

  if (jpegDecompress->src == NULL)       /* first time for this JPEG object? */
  {
    jpegDecompress->src = (struct jpeg_source_mgr *)
      (*jpegDecompress->mem->alloc_small)
      ((j_common_ptr) jpegDecompress, JPOOL_PERMANENT,
       sizeof(MMIOSourceManager));
  }
  src = (MMIOSourceManager *) jpegDecompress->src;
  src->jpegSourceManager.init_source = initSource;
  src->jpegSourceManager.fill_input_buffer = fillInputBuffer;
  src->jpegSourceManager.skip_input_data = skipInputData;
  src->jpegSourceManager.term_source = termSource;
  src->jpegSourceManager.resync_to_restart = jpeg_resync_to_restart;
  src->jpegSourceManager.bytes_in_buffer = 0;
  src->jpegSourceManager.next_input_byte = NULL; /* until buffer loaded */
  src->mmDecompress = mmDecompress;
}


/*
 * Initialize destination --- called by jpeg_start_compress
 * before any data is actually written.
 */

//METHODDEF void
//init_destination (j_compress_ptr jpegCompress)
//{
//  MMIODestManager *dest = (MMIODestManager *) jpegCompress->dest;

//  dest->jpegDestManager.next_output_byte = (JOCTET*)((*(dest->mmCompress))->pDstBuf);
//  dest->jpegDestManager.free_in_buffer = (*(dest->mmCompress))->ulDstBufLen;
//}


/*
 * Empty the output buffer --- called whenever buffer fills up.
 *
 * In typical applications, this should write the entire output buffer
 * (ignoring the current state of next_output_byte & free_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been dumped.
 *
 * In applications that need to be able to suspend compression due to output
 * overrun, a FALSE return indicates that the buffer cannot be emptied now.
 * In this situation, the compressor will return to its caller (possibly with
 * an indication that it has not accepted all the supplied scanlines).  The
 * application should resume compression after it has made more room in the
 * output buffer.  Note that there are substantial restrictions on the use of
 * suspension --- see the documentation.
 *
 * When suspending, the compressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_output_byte & free_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point will be regenerated after resumption, so do not
 * write it out when emptying the buffer externally.
 */

//METHODDEF boolean
//empty_output_buffer (j_compress_ptr jpegCompress)
//{
//  MMIODestManager *dest = (MMIODestManager *) jpegCompress->dest;

//  dest->jpegDestManager.next_output_byte = (JOCTET*)((*(dest->mmCompress))->pDstBuf);
//  dest->jpegDestManager.free_in_buffer = (*(dest->mmCompress))->ulDstBufLen;

//  return TRUE;
//}


/*
 * Terminate destination --- called by jpeg_finish_compress
 * after all data has been written.  Usually needs to flush buffer.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

//METHODDEF void
//term_destination (j_compress_ptr jpegCompress)
//{
//  MMIODestManager *dest = (MMIODestManager *) jpegCompress->dest;

//  dest->jpegDestManager.next_output_byte = (JOCTET*)((*(dest->mmCompress))->pDstBuf);
//  dest->jpegDestManager.free_in_buffer = (*(dest->mmCompress))->ulDstBufLen;
//}

//GLOBAL void
//initMMIODestManager (j_compress_ptr jpegCompress, PMMCOMPRESS *mmCompress)
//{
//  MMIODestManager *dest;

//  if (jpegCompress->dest == NULL) /* first time for this JPEG object? */
//  {
//    jpegCompress->dest = (struct jpeg_destination_mgr *)
//      (*jpegCompress->mem->alloc_small)
//      ((j_common_ptr) jpegCompress, JPOOL_PERMANENT,
//       sizeof(MMIODestManager));
//  }
//  dest = (MMIODestManager *) jpegCompress->dest;
//  dest->jpegDestManager.init_destination = init_destination;
//  dest->jpegDestManager.empty_output_buffer = empty_output_buffer;
//  dest->jpegDestManager.term_destination = term_destination;
//  dest->mmCompress = mmCompress;
//}

