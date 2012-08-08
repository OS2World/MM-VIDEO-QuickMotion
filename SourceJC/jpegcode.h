#ifndef JPEGCODE_H
#define JPEGCODE_H

/**********************************************************
 * Includes
 **********************************************************/
#define INCL_32
#define INCL_GPI
#define INCL_GPIBITMAPS
#define INCL_DOSRESOURCES
#define INCL_DOSMODULEMGR
#define INCL_PM
#include <os2.h>

#define INCL_MMIOOS2
#define INCL_MMIO_CODEC
#include <os2me.h>

#include <stdlib.h>
#include <memory.h>
#include <jpeglib.h>
#include <jerror.h>

#pragma pack(2)

/**********************************************************
 * JPEG library source / destination managers for buffer
 **********************************************************/
typedef struct
{
  struct jpeg_source_mgr jpegSourceManager; /* base fields */
  PMMDECOMPRESS *mmDecompress; /* address of mmDecompress pointer in ImageCodec */
  boolean startOfFile; /* Have we gotten any data yet? */
} MMIOSourceManager;

typedef struct
{
  struct jpeg_destination_mgr jpegDestManager; /* base fields */
  PMMCOMPRESS *mmCompress; /* address of mmCompress pointer in ImageCodec */
} MMIODestManager;

/**********************************************************
 * Functions
 **********************************************************/
ULONG EXPENTRY Codec_Entry(PULONG phCodec, USHORT usMsg, LONG lParam1, LONG lParam2);
extern "C" int _CRT_init(void);
extern "C" void _CRT_term(void);
extern "C" void __ctordtorInit(void);
extern "C" void __ctordtorTerm(void);

METHODDEF void init_source (j_decompress_ptr);
METHODDEF boolean fill_input_buffer (j_decompress_ptr);
METHODDEF void skip_input_data (j_decompress_ptr, long);
METHODDEF void term_source (j_decompress_ptr);
GLOBAL void initMMIOSourceManager (j_decompress_ptr, PMMDECOMPRESS*);
METHODDEF void init_destination (j_compress_ptr);
METHODDEF boolean empty_output_buffer (j_compress_ptr);
METHODDEF void term_destination (j_compress_ptr);
GLOBAL void initMMIODestManager (j_compress_ptr, PMMCOMPRESS*);

#pragma pack()
#endif
