#include "qmmusic.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <ireslib.hpp>

#include "musifmt.hpp"
#include "musifile.hpp"

#ifdef DEBUGGING_IOPROC
#include <fstream.h>
ofstream *debugOutput;
#endif

/*************************************************************************************
 * DLL module handle.  Gets set at initialization.
 * Data area is separate for each using process, so this value
 * could be different (or same) per process.
 *************************************************************************************/
HMODULE hModuleHandle;

/*************************************************************************************
 * Old MIDI ioproc and entry point
 *************************************************************************************/
IDynamicLinkLibrary oldMIDILibrary("MIDIIO");
typedef LONG (EXPENTRY IOPROCENTRY) (PMMIOINFO pmmioinfo, USHORT usMsg, LONG lParam1, LONG lParam2);
IOPROCENTRY *MIDIIOProc;

/************************************************************************/
/* exitFunction                                                         */
/************************************************************************/
static void _System exitFunction(ULONG reason)
{
  oldMIDILibrary.close();
#ifdef DEBUGGING_IOPROC
  if (debugOutput)
    debugOutput->close();
#endif

  __ctordtorTerm();
  DosExitList(EXLST_EXIT, exitFunction);
}

/************************************************************************/
/* DLL Initialization/Termination Function                              */
/************************************************************************/

ULONG _System _DLL_InitTerm (ULONG hModHandle, ULONG fTerm)
{
   hModuleHandle = hModHandle;  /* Remember for NLS lookup */
   if (!fTerm)
   {
#ifdef DEBUGGING_IOPROC
  debugOutput = 0;
#endif
     DosExitList(0x0000FF00 | EXLST_ADD, exitFunction);
     __ctordtorInit();
     oldMIDILibrary.open();
     MIDIIOProc = (IOPROCENTRY*)oldMIDILibrary.procAddress("MIDIIOProc");
   }

   return (1L);                 /* Success */
}

/************************************************************************/
/* DLL Entry Point                                                      */
/************************************************************************/
LONG EXPENTRY IOProc_Entry(PMMIOINFO pmmioinfo, USHORT usMsg, LONG lParam1, LONG lParam2)
{
  LONG returnCode;
  MusicFile* musicFile;

  if (usMsg == MMIOM_OPEN)
  {
    if (MusicFormat().identifyFile((PSZ)lParam1, 0) == MMIO_SUCCESS)
    {
      pmmioinfo->pExtraInfoStruct = new MusicFile();
      pmmioinfo->aulInfo[0] = FOURCC_IOPROC;
      musicFile = (MusicFile*)(pmmioinfo->pExtraInfoStruct);
      musicFile->setFileInfo(pmmioinfo);
#ifdef DEBUGGING_IOPROC
  debugOutput = &(((MusicFile *)(pmmioinfo->pExtraInfoStruct))->debugOutput);
#endif
    } else
      return MIDIIOProc(pmmioinfo, usMsg, lParam1, lParam2);
  }

  if (pmmioinfo && pmmioinfo->pExtraInfoStruct && pmmioinfo->aulInfo[0] == FOURCC_IOPROC)
  {
    musicFile = (MusicFile*)(pmmioinfo->pExtraInfoStruct);
    musicFile->setFileInfo(pmmioinfo);
    try {
      returnCode = musicFile->dispatchMessage(usMsg, lParam1, lParam2); }
    catch (IException ie) {
      *(char*)0 = 0; }
    if ((usMsg == MMIOM_CLOSE) && (returnCode == MMIO_SUCCESS)  ||
        (usMsg == MMIOM_OPEN) && (returnCode != MMIO_SUCCESS))
    {
      delete musicFile;
#ifdef DEBUGGING_IOPROC
      debugOutput = 0;
#endif
    }
    return returnCode;
  } else
    return MIDIIOProc(pmmioinfo, usMsg, lParam1, lParam2);
   // return musicFormat.dispatchMessage(usMsg, lParam1, lParam2);
}

/**************************************************************************
 *   showExpiredMessage - Annoying window in beta version                **
 **************************************************************************/
//BOOL showExpiredMessage()
//{
//  DATETIME currentDateTime = {0};
//  ULONG currentCompare;
//  ULONG warningCompare = 19960627;
//  ULONG expireCompare = 19960701;
//  CHAR message[256];
//  MB2INFO mb2info;
//
//  DosGetDateTime(&currentDateTime);
//  currentCompare = currentDateTime.year * 10000 + currentDateTime.month * 100 + currentDateTime.day;
//
//  if (currentCompare < warningCompare)
//    return (FALSE);
//
//  MB2D mb2d[1] = {              /* Array of button defs     */
//      { "  ~Ok  ", DID_OK, BS_DEFAULT}};
//
//  mb2info.hIcon = WinLoadPointer(HWND_DESKTOP, hModuleHandle, QUICKMO_ID);
//  mb2info.cButtons = 1;                          /* number of buttons   */
//  mb2info.flStyle = MB_CUSTOMICON | MB_MOVEABLE; /* style flags         */
//  mb2info.hwndNotify = NULLHANDLE;               /* Reserved            */
//  mb2info.cb = sizeof(MB2INFO) + ((mb2info.cButtons > 1) ?
//   (mb2info.cButtons -1) * sizeof (MB2D) : 0);
//  memcpy (&mb2info.mb2d, &mb2d, mb2info.cb);
//
//  if (currentCompare < expireCompare)
//    sprintf(message, "%s\n\n%s\n\n%s\n%s\n",
//                     "QuickMotion for OS/2",
//                     "Version 1.0.0b",
//                     "This version of QuickMotion",
//                     "will expire on July 1, 1996");
//  else
//    sprintf(message, "%s\n\n%s\n\n%s\n%s\n",
//                     "QuickMotion for OS/2",
//                     "Version 1.0.0b",
//                     "This version of QuickMotion",
//                     "expired on July 1, 1996");
//
//  WinMessageBox2(HWND_DESKTOP,
//     HWND_DESKTOP, /* client-window handle */
//     message, /* body of the message */
//     "Expiration Notice", /* title of the message */
//     12001, /* message box id */
//     &mb2info); /* message box input structure */
//
//  return (currentCompare >= expireCompare);
//}

/**************************************************************************
 *   GetFormatString                                                     **
 **************************************************************************
 *
 * ARGUMENTS:
 *
 *     lSearchId         - Table search key
 *     pszFormatString   - Address where to return string.
 *                         If null, then string is not returned.
 *     lBytes            - Number of bytes to copy.
 *
 * RETURN:
 *
 *     Returns 0 if string not found, or the number of characters (bytes)
 *     copied to the callers buffer.
 *     Note, returned string is not ASCII nul terminated
 *
 * DESCRIPTION:
 *
 *     This function will retrieve the format string for the specified
 *     IOProc from the resource file that contains the strings.
 *
 ***************************************************************************/
LONG GetFormatString (LONG lSearchId,
                      PSZ  pszFormatString,   /* Null, or dest address     */
                      LONG lBytes)            /* Caller provided maximum   */
{
   PVOID   pResourceData;
   CHAR    *pTemp;
   LONG    lStringLen;      /* Length of format string  */
   LONG    lRetVal = 0;     /* Function return value    */
   LONG    lSearchTemp;

   if(DosGetResource(hModuleHandle, RT_RCDATA, IOPROC_NAME_TABLE, &pResourceData))
     return (MMIO_ERROR);

   /*
    * The resource table is of the form : FOURCC String\0
    * Loop until a match is found, then return the string.
    * Copy up to the number of bytes specified.
    */

   lStringLen = 0;
   pTemp = (CHAR *)pResourceData;

   while (pTemp)
      {
      memmove(&lSearchTemp, pTemp, sizeof(LONG));

      if (lSearchTemp == 0L)
         {
         break;  /* End of table, search item not found */
         }

      if (lSearchTemp == lSearchId)   /* Search item found?               */
         {
         pTemp += sizeof(LONG);       /* Bypass numeric search id         */
         lStringLen = strlen(pTemp);  /* Determine length of found string */
         if (lStringLen >= lBytes)    /* Truncate, if necessary           */
            {
            if (lBytes > 0)
               lStringLen = lBytes;   /* Number of characters to return   */
            else
               {
               /* Callers buffer has zero size.  Cannot return any data   */
               lRetVal = 0;           /* Size of returned data            */
               break;                 /* Done searching                   */
               }
            }
         if (pszFormatString != NULL)
           memcpy(pszFormatString, pTemp, lStringLen); /* Copy string to caller */

         lRetVal = lStringLen;        /* Function return value            */
         break;                       /* We're done searching             */
         }

      pTemp += sizeof(FOURCC);
      pTemp += (strlen(pTemp) + 1);   /* Advance to next search key       */
      }

   DosFreeResource(pResourceData);

   return (lRetVal);  /* Zero or strlen */
}


/**************************************************************************
 *   GetFormatStringLength                                               **
 **************************************************************************
 *
 * ARGUMENTS:
 *
 *     lSearchId         - Table search key
 *     plNameLength      - Address where to return string length.
 *
 * RETURN:
 *
 *     Length of the format string not including the terminating '\0'.
 *     That is, the same value as would be returned from strlen().
 *
 * DESCRIPTION:
 *
 *     This function will retrieve the length of the format string
 *     for the specified IOProc from the resource file that contains
 *     the strings.
 *
 ***************************************************************************/
LONG GetFormatStringLength (LONG  lSearchId,
                            PLONG plNameLength)
{
   LONG  lStringSize;
   LONG  lRetVal;

   lStringSize = GetFormatString (lSearchId, NULL, LONG_MAX);
   if (lStringSize > 0)             /* String found?                      */
      {
      *plNameLength = lStringSize;  /* yes, return strlen                 */
      lRetVal = 0;                  /* and indicate success to caller     */
      }
   else
      {
      *plNameLength = 0;            /* no, error.  Return zero for length */
      lRetVal = lStringSize;        /* and error code from called routine */
      }
   return (lRetVal);
}


/**************************************************************************
 *   GetNLSData                                                          **
 **************************************************************************
 *
 * ARGUMENTS:
 *
 *     pulCodePage       - Address where to return the code page/country.
 *     pulLanguage       - Address where to return the language/dialect.
 *
 * RETURN:
 *
 *     Error code or 0.
 *
 * DESCRIPTION:
 *
 *     This function will retrieve the NLS information for the IOProc
 *     strings contained in the resource file.
 *
 ***************************************************************************/

ULONG APIENTRY GetNLSData(PULONG pulCodePage, PULONG pulLanguage)
{
   PVOID   pResourceData;
   CHAR    *pTemp;

   if (DosGetResource(hModuleHandle, RT_RCDATA, NLS_CHARSET_INFO, &pResourceData ))
     return (MMIO_ERROR);

   /*
    * The resource table is of the form :
    *   usCodePage     Low
    *   usCountryCode  High
    *   usLanguage     Low
    *   usDialect      High
    */

   pTemp = (CHAR *)pResourceData;

   while (pTemp)
      {
      memmove(pulCodePage, pTemp, sizeof(ULONG));
      pTemp += sizeof(ULONG);

      if (pTemp == NULL)
        break;

      memmove(pulLanguage, pTemp, sizeof(ULONG));

      break;
      }

   DosFreeResource(pResourceData);

   return (MMIO_SUCCESS);
}

