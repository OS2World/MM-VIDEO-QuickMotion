#ifndef IMA4_H
#define IMA4_H

#ifndef OS2DEF_INCLUDED
#include <os2def.h>
#endif

#define IMA4_BLOCK_SIZE                  0x22
#define IMA4_BLOCK_SAMPLES               0x40

void ima4Expand(PUCHAR soundBuffer, USHORT channels, ULONG blocks);

#endif
