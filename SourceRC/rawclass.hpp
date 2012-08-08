#include "rawcodec.h"

#include "imagecod.hpp"

class RawCodec : public ImageCodec
{
public:
  virtual LONG open(PCODECOPEN codecOpen);
  virtual LONG decompress(PMMDECOMPRESS mmDecompress);
//  virtual LONG compress(PMMCOMPRESS mmCompress);
  virtual LONG close();
};

