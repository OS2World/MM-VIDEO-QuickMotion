#ifndef MOOVPROC_H
#define MOOVPROC_H

/**********************************************************
 * Includes
 **********************************************************/
#define INCL_32
#define INCL_GPI
#define INCL_GPIBITMAPS
#define INCL_DOSRESOURCES
#define INCL_DOSMODULEMGR
#define INCL_PM
#define INCL_DOSDATETIME
#include <os2.h>

#define INCL_MMIOOS2
#define INCL_MMIO_CODEC
#include <os2me.h>

#include <stdlib.h>
#include <memory.h>

#pragma pack(2)

/**********************************************************
 * General I/O procedure declarations/definitions
 **********************************************************/
static char pszFormatExt[] = "MOV";

#define     FOURCC_IOPROC               mmioFOURCC('M', 'o', 'o', 'V')

/* Audio codec fourcc's */
#define     FOURCC_RAW                  mmioFOURCC('r', 'a', 'w', ' ')
#define     FOURCC_TWOS                 mmioFOURCC('t', 'w', 'o', 's')
#define     FOURCC_MAC3                 mmioFOURCC('M', 'A', 'C', '3')
#define     FOURCC_IMA4                 mmioFOURCC('i', 'm', 'a', '4')
#define     FOURCC_BLANKS               mmioFOURCC(' ', ' ', ' ', ' ')

/* RC file defines */
#define     MOOVPROC_ID                 1
#define     QUICKMO_ID                  2
#define     HEX_FOURCC_MOOV             0x566F6F4DL
#define     IOPROC_NAME_TABLE           7500
#define     NLS_CHARSET_INFO            8000
#define     DEFAULT_PALETTE_2_COLOR     0x0001
#define     DEFAULT_PALETTE_4_COLOR     0x0002
#define     DEFAULT_PALETTE_16_COLOR    0x0004
#define     DEFAULT_PALETTE_256_COLOR   0x0008
#define     DEFAULT_PALETTE_2_GRAY      0x0021
#define     DEFAULT_PALETTE_4_GRAY      0x0022
#define     DEFAULT_PALETTE_16_GRAY     0x0024
#define     DEFAULT_PALETTE_256_GRAY    0x0028

#ifndef BITT_NONE
#define BITT_NONE 0
#define BI_NONE   0
#endif

#ifndef SEEK_SET /* avoids the need to include stdio.h */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

/**********************************************************
 * MOOV I/O procedure declarations/definitions
 **********************************************************/
#define ATOM_TYPE_SIZE                   4
#define IMA4_BLOCK_SIZE                  0x22
#define IMA4_BLOCK_SAMPLES               0x40

/**********************************************************
 * Functions
 **********************************************************/
LONG EXPENTRY IOProc_Entry (PMMIOINFO pmmioinfo, USHORT usMsg,
                            LONG lParam1, LONG lParam2);
extern "C" int _CRT_init(void);
extern "C" void _CRT_term(void);
extern "C" void __ctordtorInit(void);
LONG getDefaultPalette(SHORT resourceID, PRGB2 palette, ULONG paletteLength);
void reverseBytes(PULONG inULong);
void reverseBytes(PUSHORT inUShort);
BOOL showExpiredMessage();
LONG GetFormatString (LONG lSearchId, PSZ  pszFormatString, LONG lBytes);
LONG GetFormatStringLength (LONG  lSearchId, PLONG plNameLength);
ULONG APIENTRY GetNLSData (PULONG, PULONG);
void ima4Expand(PUCHAR soundBuffer, USHORT channels, ULONG blocks);
ULONG const& key(ULONG const& element);

#pragma pack()
#endif
