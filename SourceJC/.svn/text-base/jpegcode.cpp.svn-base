#include "jpegcode.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "jfifcode.hpp"

/************************************************************************/
/* JFIF codec object                                                    */
/************************************************************************/
HMODULE ImageCodec::moduleHandle;

/************************************************************************/
/* DLL Initialization/Termination Function                              */
/************************************************************************/
ULONG _System _DLL_InitTerm (ULONG hModHandle, ULONG fTerm)
{
   ImageCodec::moduleHandle = hModHandle;  /* Remember for NLS lookup */
   return (1L);                 /* Success */
}

/************************************************************************/
/* DLL Entry Point                                                      */
/************************************************************************/
ULONG EXPENTRY Codec_Entry(PULONG phCodec, USHORT usMsg, LONG lParam1, LONG lParam2)
{
  LONG returnCode;

  if (usMsg == MMIOM_CODEC_OPEN)
    *phCodec = (ULONG)(new JFIFCodec());

  if (phCodec && *phCodec)
  {
    JFIFCodec* jfifCodec = (JFIFCodec*)(*phCodec);
    returnCode = jfifCodec->dispatchMessage(usMsg, lParam1, lParam2);
    if ((usMsg == MMIOM_CODEC_CLOSE) && (returnCode == MMIO_SUCCESS)  ||
        (usMsg == MMIOM_CODEC_OPEN) && (returnCode != MMIO_SUCCESS))
      delete jfifCodec;
    return returnCode;
  }
  else
    return MMIO_ERROR;
}

