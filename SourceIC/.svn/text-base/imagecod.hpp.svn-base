#include "imagecod.h"

#include <dive.h>

class ImageCodec
{
public:
  virtual LONG dispatchMessage(USHORT usMsg, LONG lParam1, LONG lParam2);
  virtual LONG queryNameLength(PLONG nameLength);
  virtual LONG queryName(PSZ name, PLONG nameLength);
  virtual LONG open(PCODECOPEN codecOpen);
  virtual LONG decompress(PMMDECOMPRESS mmDecompress) = 0;
//  virtual LONG compress(PMMCOMPRESS mmCompress) = 0;
  virtual LONG close();

  static HMODULE moduleHandle; //Must be set by client so that name can be returned from resource

protected:
  virtual LONG defaultResponse(USHORT usMsg, LONG lParam1, LONG lParam2);
  LONG setupSource(PBYTE sourceBuffer);
  LONG copyToDestination(PBYTE destinationBuffer);
  ULONG width() const;
  ULONG height() const;
  ULONG depth() const;

  PCODECOPEN codecOpen_p;
  PMMDECOMPRESS mmDecompress_p;
  PMMCOMPRESS mmCompress_p;
  FOURCC sourceColorSpace;
  ULONG bytesPerLine;

private:
  static LONG ImageCodec::GetFormatString(LONG lSearchId, PSZ  pszFormatString, LONG lBytes);            /* Caller provided maximum   */
  static LONG ImageCodec::GetFormatStringLength(LONG  lSearchId, PLONG plNameLength);

  HDIVE diveHandle;
  RECTL imageRectangle;
  SETUP_BLITTER setupBlitter;
  ULONG diveSourceBuffer;
};

