#include "prnscp.h"

class StorageSystemFile
{
public:
  const ULONG identifier;
  StorageSystemFile();
  virtual void setFileInfo(PMMIOINFO fileInfo);
  virtual LONG dispatchMessage(USHORT usMsg, LONG lParam1, LONG lParam2);
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle) = 0;
  virtual LONG open(PSZ fileName) = 0;
  virtual LONG read(PSZ buffer, LONG bufferLength) = 0;
  virtual LONG write(PSZ buffer, LONG bufferLength) = 0;
  virtual LONG seek(LONG seekPosition, ULONG seekMode) = 0;
  virtual LONG close(USHORT closeFlags) = 0;

protected:
  virtual LONG defaultResponse(USHORT usMsg, LONG lParam1, LONG lParam2);
  VOID setError(ULONG error);
  ULONG flags();
  LONG currentFilePosition();

private:
  PMMIOINFO fileInfo_p;
};
