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
#define INCL_DOSPROCESS
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
#define     APPLICATION_ID              1
#define     QUICKMO_ID                  2
#define     HEX_FOURCC_MOOV             0x566F6F4DL
#define     IOPROC_NAME_TABLE           7500
#define     NLS_CHARSET_INFO            8000

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
 * Functions
 **********************************************************/
LONG EXPENTRY IOProc_Entry (PMMIOINFO pmmioinfo, USHORT usMsg,
                            LONG lParam1, LONG lParam2);
extern "C" int _CRT_init(void);
extern "C" void _CRT_term(void);
extern "C" void __ctordtorInit(void);
BOOL showExpiredMessage();
LONG GetFormatString (LONG lSearchId, PSZ  pszFormatString, LONG lBytes);
LONG GetFormatStringLength (LONG  lSearchId, PLONG plNameLength);
ULONG APIENTRY GetNLSData (PULONG, PULONG);

#pragma pack()
#endif
