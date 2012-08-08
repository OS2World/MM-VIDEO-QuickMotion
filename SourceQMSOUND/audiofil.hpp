#include "qmsound.h"

#ifdef DEBUGGING_IOPROC
#include <fstream.h>
#endif

class SoundFile;

class AudioFile
{
public:
  AudioFile();
  virtual void setFileInfo(PMMIOINFO fileInfo);
  virtual LONG dispatchMessage(USHORT usMsg, LONG lParam1, LONG lParam2);
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle) = 0;
  virtual LONG queryHeaderLength();
  virtual LONG open(PSZ fileName);
  virtual LONG getHeader(PVOID header, LONG headerLength) = 0;
  virtual LONG setHeader(PVOID header, LONG headerLength) = 0;
  virtual LONG read(PSZ buffer, LONG bufferLength) = 0;
  virtual LONG seek(LONG seekPosition, LONG seekMode) = 0;
  virtual LONG write(PSZ buffer, LONG bufferLength) = 0;
  virtual LONG seekByTime(LONG seekTime, LONG seekMode) = 0;
  virtual LONG close(USHORT closeFlags);
  ~AudioFile();

#ifdef DEBUGGING_IOPROC
  ofstream debugOutput;
#endif

protected:
  /* Standard-format audio header info.  Specific formats will need to use this. */
  MMAUDIOHEADER audioHeader;

  virtual LONG defaultResponse(USHORT usMsg, LONG lParam1, LONG lParam2);

  HMMIO storageSystemFileHandle();
  VOID setError(ULONG error);
  ULONG flags();

private:
  PMMIOINFO fileInfo_p;
  HMMIO storageSystemFileHandle_p;
};
