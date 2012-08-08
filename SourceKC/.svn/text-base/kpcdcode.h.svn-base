#ifndef KPCDCODE_H
#define KPCDCODE_H

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

/**********************************************************
 * Functions
 **********************************************************/
ULONG EXPENTRY Codec_Entry(PULONG phCodec, USHORT usMsg, LONG lParam1, LONG lParam2);
extern "C" int _CRT_init(void);
extern "C" void _CRT_term(void);
extern "C" void __ctordtorInit(void);
extern "C" void __ctordtorTerm(void);

#endif
