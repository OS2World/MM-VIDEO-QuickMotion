#ifndef PRNSCP_H
#define PRNSCP_H

/**********************************************************
 * Includes
 **********************************************************/
#define INCL_32
#define INCL_DOSRESOURCES
#define INCL_DOSMODULEMGR
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_GPI
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
static char pszFormatExt[] = "nscp";

#define     FOURCC_IOPROC               mmioFOURCC('N', 'S', 'C', 'P')


/* RC file defines */
#define     APPLICATION_ID              1
#define     PRNSCP_ID                   2
#define     HEX_FOURCC_NSCP             0x5043534EL
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
LONG EXPENTRY IOProc_Entry (PMMIOINFO pmmioinfo, USHORT usMsg, LONG lParam1, LONG lParam2);
extern "C" int _CRT_init(void);
extern "C" void _CRT_term(void);
extern "C" void __ctordtorInit(void);
extern "C" void __ctordtorTerm(void);
LONG GetFormatString (LONG lSearchId, PSZ  pszFormatString, LONG lBytes);
LONG GetFormatStringLength (LONG  lSearchId, PLONG plNameLength);
ULONG APIENTRY GetNLSData (PULONG, PULONG);

#pragma pack()
#endif
