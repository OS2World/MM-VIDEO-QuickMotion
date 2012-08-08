#include "navifile.hpp"
#include "navifmt.hpp"

#include <istring.hpp>

#ifdef TRACE_PLUGIN
#include <fstream.h>
#endif

/************************************************************
 * IDENTIFY FILE - Will come here if an info block is passed
 ************************************************************/
LONG NavigatorFile::identifyFile(PSZ fileName, HMMIO fileHandle)
{
  return NavigatorFormat().identifyFile(fileName, fileHandle);
}

/************************************************************
 * OPEN
 ************************************************************/
LONG NavigatorFile::open(PSZ fileName)
{
  // File name is an ascii representation of the memory file handle
  IString fileNameString(fileName);
  memoryFile = fileNameString.asInt();

  // Information maintained by the plugin is stored in memory file's pExtraInfoStruct
  MMIOINFO memoryFileInfo;
  mmioGetInfo(memoryFile, &memoryFileInfo, 0);
  totalBytes = memoryFileInfo.cchBuffer;
  pluginIOInfo = (PluginIOInfo*)(memoryFileInfo.pExtraInfoStruct);

#ifdef TRACE_PLUGIN
  *(pluginIOInfo->debugOutput) << "NSCP IOPROC OPEN:\t" << fileName << endl;
#endif

  // Open consists of resetting the file position to 0
  seek(0, SEEK_SET);

  if (pluginIOInfo->currentPosition)
    return (MMIO_ERROR);
  else
    return (MMIO_SUCCESS);
}

/************************************************************
 * READ
 ************************************************************/
LONG NavigatorFile::read(PSZ buffer, LONG bufferLength)
{
#ifdef TRACE_PLUGIN
  *(pluginIOInfo->debugOutput) << "NSCP IOPROC READ ->: Size: " << pluginIOInfo->currentSize << "\tPos: " <<
                                  pluginIOInfo->currentPosition << "\tLen: " << bufferLength << endl;
#endif

  ULONG postCount;
  LONG position;
  LONG bytesRead = 0;
  LONG returnCode;

  // Adjust requested read size if read would go beyond total size of file
  if (pluginIOInfo->currentPosition + bufferLength > totalBytes)
    bufferLength = totalBytes - pluginIOInfo->currentPosition;

  if (bufferLength)
  {
    // Wait, if necessary, for enough data to arrive
    while (pluginIOInfo->currentSize < pluginIOInfo->currentPosition + bufferLength)
    {
      if (!pluginIOInfo->waitForData)
        throw(NoMoreData());
      returnCode = DosWaitEventSem(pluginIOInfo->dataReceived, SEM_INDEFINITE_WAIT);
      returnCode = DosResetEventSem(pluginIOInfo->dataReceived, &postCount);
    }

    // Read private movie file
    returnCode = DosRequestMutexSem(pluginIOInfo->movieFileAccess, SEM_INDEFINITE_WAIT);

    position = mmioSeek(memoryFile, pluginIOInfo->currentPosition, SEEK_SET);
    bytesRead = mmioRead(memoryFile, buffer, bufferLength);
    pluginIOInfo->currentPosition += bytesRead;

    returnCode = DosReleaseMutexSem(pluginIOInfo->movieFileAccess);
  }

#ifdef TRACE_PLUGIN
  *(pluginIOInfo->debugOutput) << "NSCP IOPROC READ <-\t" << bytesRead << endl;
#endif

  return (bytesRead);
}

/************************************************************
 * WRITE
 ************************************************************/
LONG NavigatorFile::write(PSZ buffer, LONG bufferLength)
{
  return -1;
}

/************************************************************
 * SEEK
 ************************************************************/
LONG NavigatorFile::seek(LONG seekPosition, ULONG seekMode)
{
#ifdef TRACE_PLUGIN
  *(pluginIOInfo->debugOutput) << "NSCP IOPROC SEEK: Pos: " << seekPosition << "\tMode: " <<
                                  seekMode << endl;
#endif

  switch (seekMode)
  {
    case SEEK_SET:
      pluginIOInfo->currentPosition = seekPosition;
      break;

    case SEEK_CUR:
      pluginIOInfo->currentPosition += seekPosition;
      break;

    case SEEK_END:
      pluginIOInfo->currentPosition = pluginIOInfo->currentSize + seekPosition;
      break;

    default:
      break;
  }

  return (pluginIOInfo->currentPosition);
}

/************************************************************
 * CLOSE
 ************************************************************/
LONG NavigatorFile::close(USHORT closeFlags)
{
#ifdef TRACE_PLUGIN
  *(pluginIOInfo->debugOutput) << "NSCP IOPROC CLOSE:\t" << endl;
#endif

  LONG returnCode = 0;

  if (returnCode)
    return (MMIO_ERROR);
  else
    return (MMIO_SUCCESS);
}

