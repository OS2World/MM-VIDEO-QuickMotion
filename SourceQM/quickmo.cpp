#include "moovfile.hpp"

void EXPENTRY QUICKMO(MOOVFile *moovFile, PMMMULTITRACKREAD multiTrackInfo)
{
#ifndef DEMO_VERSION
  if (moovFile->current->chunkCursor->isValid() &&
      !((moovFile->extendedInfo_p.ulFlags & MMIO_REVERSE_READ) && moovFile->current->sampleInMedia == 1))
    multiTrackInfo->ulFlags &= ~MULTITRACKREAD_EOF;
#endif
}

