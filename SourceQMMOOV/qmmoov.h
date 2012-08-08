#ifndef QMMOOV_H
#define QMMOOV_H

/**********************************************************
 * Includes
 **********************************************************/
#ifndef INCL_32
#define INCL_32
#define INCL_DOSRESOURCES
#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#define INCL_GPI
#include <os2.h>
#endif

#ifndef INCL_MMIOOS2
#define INCL_MMIOOS2
#include <os2me.h>
#endif

#define ATOM_TYPE_SIZE                   4

#define     DEFAULT_PALETTE_2_COLOR     0x0001
#define     DEFAULT_PALETTE_4_COLOR     0x0002
#define     DEFAULT_PALETTE_16_COLOR    0x0004
#define     DEFAULT_PALETTE_256_COLOR   0x0008
#define     DEFAULT_PALETTE_2_GRAY      0x0021
#define     DEFAULT_PALETTE_4_GRAY      0x0022
#define     DEFAULT_PALETTE_16_GRAY     0x0024
#define     DEFAULT_PALETTE_256_GRAY    0x0028

#ifndef SEEK_SET /* avoids the need to include stdio.h */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

void _Export reverseBytes(PULONG inULong);
void _Export reverseBytes(PUSHORT inUShort);
ULONG const& _Export key(ULONG const& element);

/**********************************************************
 * Functions
 **********************************************************/
extern "C" int _CRT_init(void);
extern "C" void _CRT_term(void);
extern "C" void __ctordtorInit(void);
extern "C" void __ctordtorTerm(void);

LONG _Export getDefaultPalette(SHORT resourceID, PRGB2 palette, ULONG paletteLength);

#endif

