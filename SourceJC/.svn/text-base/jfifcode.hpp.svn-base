#include "jpegcode.h"

#include "imagecod.hpp"

class JFIFCodec : public ImageCodec
{
public:
  virtual LONG open(PCODECOPEN codecOpen);
  virtual LONG decompress(PMMDECOMPRESS mmDecompress);
//  virtual LONG compress(PMMCOMPRESS mmCompress);
  virtual LONG close();

private:
  METHODDEF VOID handleJPEGError(j_common_ptr jpegInfo);

  JSAMPARRAY bitmap; /* Decompressed bitmap */
  JSAMPARRAY workLines; /* Working buffer for returned decompressed lines */
  ULONG rgbBytesPerLine; /* Bytes per line of full-color output */
  struct jpeg_error_mgr mmioErrorManager;  /* JPEG library error manager object */

  struct jpeg_decompress_struct jpegDecompress; /* JPEG library decompression object */

//  struct jpeg_compress_struct jpegCompress; /* JPEG library compression object */
  RGB2 palette[MAX_PALETTE]; /* Palette used for colormapped input */
  USHORT bitCount;
};

