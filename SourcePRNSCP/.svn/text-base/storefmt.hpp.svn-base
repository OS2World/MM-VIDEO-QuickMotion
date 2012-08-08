#include "prnscp.h"

class StorageSystemFormat
{
public:
  virtual LONG dispatchMessage(USHORT usMsg, LONG lParam1, LONG lParam2);
  virtual LONG getFormatInfo(PMMFORMATINFO pMMFormatInfo);
  virtual LONG getFormatName(PSZ formatName, LONG formatNameSize);
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle) = 0;

protected:
  virtual LONG defaultResponse();
};

