#ifndef IMAGECOD_H
#define IMAGECOD_H

/**********************************************************
 * Includes
 **********************************************************/
#define INCL_32
#define INCL_GPI
#define INCL_DOSRESOURCES
#include <os2.h>

#define INCL_MMIOOS2
#define INCL_MMIO_CODEC
#include <os2me.h>

// Client is expected to provide name table and charset resources with these ID's
#define     CODEC_ID                       1
#define     NAME_TABLE                  7500

#endif

