#ifndef QMMUSIC_H
#define QMMUSIC_H

/**********************************************************
 * Includes
 **********************************************************/
#define INCL_32
#define INCL_DOSRESOURCES
#define INCL_DOSMODULEMGR
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_GPI
#include <os2.h>

#define INCL_MMIOOS2
#define INCL_MMIO_CODEC
#include <os2me.h>

#include <midios2.h>

#pragma pack(2)

/**********************************************************
 * General I/O procedure declarations/definitions
 **********************************************************/
static char pszFormatExt[] = "MOV";

#define     FOURCC_IOPROC               mmioFOURCC('Q', 'M', 'M', 'U')


/* RC file defines */
#define     APPLICATION_ID              1
#define     QUICKMO_ID                  2
#define     HEX_FOURCC_MOOV             0x554D4D51L
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
 * Music declarations/definitions
 **********************************************************/
#define MIDI_CLOCKS_PER_BEAT 24 /* PPQN */
#define MIDI_TICK_RATE 240
#define MIDI_BEATS_PER_SECOND 2
#define MIDI_MAX_LEADER_LENGTH 1024
#define MIDI_CONTROLLER 0xB0
#define MIDI_PATCH 0xC0
#define MIDI_NOTE 0x90
#define MIDI_DRUM_KIT 16384
#define MIDI_PERCUSSION_CHANNEL 9

#define QTMA_NOTE 0x20000000
#define QTMA_EXTENDED_NOTE 0x80000000
#define QTMA_CONTROLLER 0x40000000
#define QTMA_REST 0x00000000
#define QTMA_MARKER 0x60000000

/**********************************************************
 * Functions
 **********************************************************/
LONG EXPENTRY IOProc_Entry (PMMIOINFO pmmioinfo, USHORT usMsg,
                            LONG lParam1, LONG lParam2);
extern "C" int _CRT_init(void);
extern "C" void _CRT_term(void);
extern "C" void __ctordtorInit(void);
extern "C" void __ctordtorTerm(void);
BOOL showExpiredMessage();
LONG GetFormatString (LONG lSearchId, PSZ  pszFormatString, LONG lBytes);
LONG GetFormatStringLength (LONG  lSearchId, PLONG plNameLength);
ULONG APIENTRY GetNLSData (PULONG, PULONG);

#pragma pack()
#endif
