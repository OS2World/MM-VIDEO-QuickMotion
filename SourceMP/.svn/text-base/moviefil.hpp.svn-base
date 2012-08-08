#include "moovproc.h"

class MOOVFile;

class MovieFile
{
public:
  MovieFile();
  virtual void setMovieFileInfo(PMMIOINFO movieFileInfo);
  virtual LONG dispatchMessage(USHORT usMsg, LONG lParam1, LONG lParam2);
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle) = 0;
  virtual LONG queryHeaderLength();
  virtual LONG open(PSZ fileName);
  virtual LONG set(PMMEXTENDINFO extendedInfo, LONG flags);
  virtual LONG getHeader(PVOID header, LONG headerLength) = 0;
  virtual LONG setHeader(PVOID header, LONG headerLength) = 0;
  virtual LONG read(PSZ buffer, LONG bufferLength) = 0;
  virtual LONG multiTrackRead(PMMMULTITRACKREAD multiTrackInfo) = 0;
  virtual LONG decompress(PMMDECOMPRESS decompress);
  virtual LONG write(PSZ buffer, LONG bufferLength) = 0;
  virtual LONG seekByTime(LONG seekTime, LONG seekMode) = 0;
  virtual LONG close(USHORT closeFlags);
  ~MovieFile();

protected:
  /* Standard-format movie header info.  Specific formats will need to use this. */
  MMMOVIEHEADER movieHeader;
  MMTRACKINFO trackInfoList[2];
  MMVIDEOHEADER videoHeader;
  RGB2 palette[256];
  MMXDIBHEADER extendedBitmapHeader;
  MMAUDIOHEADER audioHeader;

  virtual LONG defaultResponse(USHORT usMsg, LONG lParam1, LONG lParam2);
  virtual LONG openCodec();
  HMMIO storageSystemFileHandle();
  VOID setError(ULONG error);
  ULONG flags();
  FOURCC childIOProc();
  ULONG extendedFlags();
  ULONG track();

private:
  PMMIOINFO movieFileInfo_p;
  HMMIO storageSystemFileHandle_p;
  MMEXTENDINFO extendedInfo_p;  /* MMIO extended info for movies */
  CODECASSOC codecAssoc;
  CODECOPEN codecOpen;
  CODECINIFILEINFO codecIniFileInfo;
  CODECVIDEOHEADER sourceHeader;
  CODECVIDEOHEADER destinationHeader;
  HMODULE codecModule;
  PCODECPROC sendCodecMessage;
  ULONG codecHandle;

friend void EXPENTRY QUICKMO(MOOVFile *moovFile, PMMMULTITRACKREAD multiTrackInfo);
};
