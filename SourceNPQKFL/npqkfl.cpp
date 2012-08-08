#include "npqkfl.h"
#include "qfplugin.hpp"

#include <limits.h>
#include <ithread.hpp>

#ifdef TRACE_PLUGIN
#include <fstream.h>
ofstream *debugOutput;
#endif

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
HMODULE moduleHandle;

/************************************************************************/
/* Date/Time Stamp Routine for debugging                                */
/************************************************************************/
#ifdef TRACE_PLUGIN
void stampTime()
{
  DATETIME now;
  DosGetDateTime(&now);
  *debugOutput << (USHORT)(now.hours) << ':' << (USHORT)(now.minutes) << ':' <<
                  (USHORT)(now.seconds) << '.' << (USHORT)(now.hundredths) << '\t';
  *debugOutput << IThread::current().id() << '\t';
}
#endif

/************************************************************************/
/* exitFunction                                                         */
/************************************************************************/
static void _System exitFunction(ULONG reason)
{
  if (QuickFlickPlugin::mainThreadPriorityBoosted)
      IThread::current().setPriority(IApplication::regular, 0);

#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "QuickFlick unloaded:" << endl;
  debugOutput->close();
#endif

  __ctordtorTerm();
  DosExitList(EXLST_EXIT, exitFunction);
}


/************************************************************************/
/* _DLL_InitTerm:                                                       */
/************************************************************************/
ULONG _System _DLL_InitTerm (ULONG hModHandle, ULONG fTerm)
{
  ULONG rc;

  moduleHandle = hModHandle;

  if (!fTerm)
  {
    DosExitList(0x0000FF00 | EXLST_ADD, exitFunction);
    __ctordtorInit();

#ifdef TRACE_PLUGIN
  debugOutput = new ofstream("plugin.msg", ios::out | ios::trunc);
  stampTime();
  *debugOutput << "QuickFlick loaded:" << endl;
#endif
  }

  return (1L);  /* Success */
}

//----------------------------------------------------------------------------
// NPP_Initialize:
//----------------------------------------------------------------------------
NPError NPP_Initialize(void)
{
#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "->Initialize:" << endl;
#endif

  IThread::current().setPriority(IApplication::timeCritical, 15); // Writes compete with playback
  QuickFlickPlugin::mainThreadPriorityBoosted = true;

  QuickFlickPlugin::initialize();

  return NPERR_NO_ERROR;
}

//----------------------------------------------------------------------------
// NPP_Shutdown:
//----------------------------------------------------------------------------
void NPP_Shutdown(void)
{
#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "->Shutdown:" << endl;
#endif

  QuickFlickPlugin::shutdown();
}

//----------------------------------------------------------------------------
// NPP_New:
//----------------------------------------------------------------------------
NPError NP_LOADDS NPP_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char* argn[], char* argv[], NPSavedData* saved)
{
#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "->New instance:" << '\t' << instance->pdata << endl;
#endif

  if (instance == NULL)
    return NPERR_INVALID_INSTANCE_ERROR;

  instance->pdata = new QuickFlickPlugin(pluginType, mode, argc, argn, argv, saved, instance);

  if (instance->pdata == NULL)
    return NPERR_OUT_OF_MEMORY_ERROR;

#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "<-New instance:" << '\t' << instance->pdata << endl;
#endif

  return NPERR_NO_ERROR;
}

//----------------------------------------------------------------------------
// NPP_SetWindow:
//----------------------------------------------------------------------------
NPError NP_LOADDS NPP_SetWindow(NPP instance, NPWindow* window)
{
  if (!instance || !window)
    return NPERR_INVALID_INSTANCE_ERROR;

  QuickFlickPlugin *quickFlickPlugin = (QuickFlickPlugin*)(instance->pdata);

  if (quickFlickPlugin)
    return quickFlickPlugin->setWindow((IWindowHandle)(HWND)(window->window));
  else
    return NPERR_INVALID_INSTANCE_ERROR;
}

//----------------------------------------------------------------------------
// NPP_NewStream:
//----------------------------------------------------------------------------
NPError NP_LOADDS NPP_NewStream(NPP instance, NPMIMEType type, NPStream *stream, NPBool seekable, uint16 *stype)
{
  NPError returnCode;

#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "->New stream:" << '\t' << instance->pdata << endl;
#endif

  if (!instance || !stream)
    return NPERR_INVALID_INSTANCE_ERROR;

  QuickFlickPlugin *quickFlickPlugin = (QuickFlickPlugin*)(instance->pdata);

  if (quickFlickPlugin)
    returnCode = quickFlickPlugin->newStream(stream->url, stream->end);
  else
    returnCode = NPERR_INVALID_INSTANCE_ERROR;

#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "<-New stream:" << '\t' << instance->pdata << endl;
#endif

  return returnCode;
}

//----------------------------------------------------------------------------
// NPP_WriteReady:
//----------------------------------------------------------------------------
int32 NP_LOADDS NPP_WriteReady(NPP instance, NPStream *stream)
{
  if (!instance)
    return 0;

  QuickFlickPlugin *quickFlickPlugin = (QuickFlickPlugin*)(instance->pdata);

  if (quickFlickPlugin)
    return quickFlickPlugin->writeReady();
  else
    return 0;
}

//----------------------------------------------------------------------------
// NPP_Write:
//----------------------------------------------------------------------------
int32 NP_LOADDS NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
  int32 bytesWritten;

#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "->Write:" << "\t\t" << instance->pdata << '\t' << stream->url << '\t' << len << endl;
#endif

  if (!instance)
    return 0;

  QuickFlickPlugin *quickFlickPlugin = (QuickFlickPlugin*)(instance->pdata);

  if (quickFlickPlugin)
    bytesWritten = quickFlickPlugin->write(len, (PCHAR)buffer);
  else
    bytesWritten = 0;

  return bytesWritten;
}

//----------------------------------------------------------------------------
// NPP_DestroyStream:
//----------------------------------------------------------------------------
NPError NP_LOADDS NPP_DestroyStream(NPP instance, NPStream *stream, NPError reason)
{
  NPError returnCode;

#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "->Destroy stream:\t" << instance->pdata << "\treason=" << reason << '\t' << stream->url << endl;
#endif

  if (!instance)
    return NPERR_INVALID_INSTANCE_ERROR;

  QuickFlickPlugin *quickFlickPlugin = (QuickFlickPlugin*)(instance->pdata);

  if (quickFlickPlugin)
    returnCode = quickFlickPlugin->destroyStream(reason);
  else
    returnCode = NPERR_INVALID_INSTANCE_ERROR;

#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "<-Destroy stream:" << '\t' << instance->pdata << '\t' << stream->url << endl;
#endif

  return returnCode;

}

//-----------------------------------------------------------------------------
// NPP_Destroy:
//-----------------------------------------------------------------------------
NPError NP_LOADDS NPP_Destroy(NPP instance, NPSavedData** save)
{
  NPError returnCode;

#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "->Destroy instance:" << '\t' << instance->pdata << endl;
#endif

  if (!instance)
    return NPERR_INVALID_INSTANCE_ERROR;

  QuickFlickPlugin *quickFlickPlugin = (QuickFlickPlugin*)(instance->pdata);

  if (quickFlickPlugin)
  {
    delete quickFlickPlugin;
    instance->pdata = 0;
    returnCode = NPERR_NO_ERROR;
  }
  else
    returnCode = NPERR_INVALID_INSTANCE_ERROR;

#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "<-Destroy instance:" << '\t' << instance->pdata << endl;
#endif

  return returnCode;

/*
        // make some saved instance data if necessary
        if(This->pSavedInstanceData == 0   ) {
            // make a struct header for the data
            This->pSavedInstanceData =
                (NPSavedData*)NPN_MemAlloc(sizeof (struct _NPSavedData));

            // fill in the struct
            if(This->pSavedInstanceData != 0   ) {
                This->pSavedInstanceData->len = 0;
                This->pSavedInstanceData->buf = 0   ;

                // replace the def below and references to it with your data
                #define SIDATA "aSavedInstanceDataBlock"

                // the data
                This->pSavedInstanceData->buf = NPN_MemAlloc(sizeof SIDATA);

                if(This->pSavedInstanceData->buf != 0   ) {
                    strcpy((char*)This->pSavedInstanceData->buf, SIDATA);
                    This->pSavedInstanceData->len = sizeof SIDATA;
                }
            }
        }

        // save some instance data
        *save = This->pSavedInstanceData;

    }      */
}

//----------------------------------------------------------------------------
// NPP_StreamAsFile:
//----------------------------------------------------------------------------
void NP_LOADDS NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "->Stream as file:" << '\t' << instance->pdata << endl;
#endif

}

//----------------------------------------------------------------------------
// NPP_Print:
//----------------------------------------------------------------------------
void NP_LOADDS NPP_Print(NPP instance, NPPrint* printInfo)
{
#ifdef TRACE_PLUGIN
  stampTime();
  *debugOutput << "->Print:" << '\t' << instance->pdata << endl;
#endif

}

