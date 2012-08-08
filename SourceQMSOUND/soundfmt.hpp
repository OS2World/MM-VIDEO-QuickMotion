#include "qmsound.h"

#include "audiofmt.hpp"

/*
 * SoundFileFormat
 */

class SoundFileFormat : public AudioFileFormat
{
public:
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle);
};

