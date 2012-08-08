#include "prnscp.h"
#include "navifmt.hpp"
#include "navifile.hpp"

#include <limits.h>
#include <string.h>

/************************************************************************/
/* Variables                                                            */
/************************************************************************/
HMODULE moduleHandle;

/************************************************************************/
/* DLL Initialization / Termination Routine                             */
/************************************************************************/
ULONG _System _DLL_InitTerm (ULONG hModHandle, ULONG fTerm)
{
  moduleHandle = hModHandle;
  return (1L);                 /* Success */
}

/************************************************************************/
/* Navigator Plugio I/O Procedure Entry Point                           */
/************************************************************************/
LONG EXPENTRY MM_Message(PMMIOINFO pmmioinfo, USHORT usMsg, LONG lParam1, LONG lParam2)
{
  NavigatorFile* navigatorFile;
  LONG returnCode;

  if (pmmioinfo)
  {
    if (usMsg == MMIOM_OPEN)
      pmmioinfo->pExtraInfoStruct = new NavigatorFile();
    navigatorFile = (NavigatorFile*)(pmmioinfo->pExtraInfoStruct);
    if (navigatorFile && navigatorFile->identifier == FOURCC_IOPROC)
    {
      navigatorFile->setFileInfo(pmmioinfo);
      returnCode = navigatorFile->dispatchMessage(usMsg, lParam1, lParam2);
      if ((usMsg == MMIOM_CLOSE) && (returnCode == MMIO_SUCCESS)  ||
          (usMsg == MMIOM_OPEN) && (returnCode != MMIO_SUCCESS))
        delete navigatorFile;
      return returnCode;
    }
  }

  return NavigatorFormat().dispatchMessage(usMsg, lParam1, lParam2);
}

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

   if(DosGetResource(moduleHandle, RT_RCDATA, IOPROC_NAME_TABLE, &pResourceData))
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

   if (DosGetResource(moduleHandle, RT_RCDATA, NLS_CHARSET_INFO, &pResourceData ))
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