#include "rawcodec.h"

#include "rawclass.hpp"

/************************************************************************/
/* raw codec object                                                     */
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
    *phCodec = (ULONG)(new RawCodec());

  if (phCodec && *phCodec)
  {
    RawCodec* rawCodec = (RawCodec*)(*phCodec);
    returnCode = rawCodec->dispatchMessage(usMsg, lParam1, lParam2);
    if ((usMsg == MMIOM_CODEC_CLOSE) && (returnCode == MMIO_SUCCESS)  ||
        (usMsg == MMIOM_CODEC_OPEN) && (returnCode != MMIO_SUCCESS))
      delete rawCodec;
    return returnCode;
  }
  else
    return MMIO_ERROR;
}

