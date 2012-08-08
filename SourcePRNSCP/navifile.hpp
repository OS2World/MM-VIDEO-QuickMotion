#include "prnscp.h"

#include <pluginio.hpp>

#include "storefil.hpp"

class NavigatorFile : public StorageSystemFile
{
public:
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle);
  virtual LONG open(PSZ fileName);
  virtual LONG read(PSZ buffer, LONG bufferLength);
  virtual LONG write(PSZ buffer, LONG bufferLength);
  virtual LONG seek(LONG seekPosition, ULONG seekMode);
  virtual LONG close(USHORT closeFlags);

private:
  HMMIO memoryFile;
  PluginIOInfo *pluginIOInfo;
  LONG totalBytes;
};

