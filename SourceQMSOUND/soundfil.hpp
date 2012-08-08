#include "qmsound.h"

#include "audiofil.hpp"
#include "moov.hpp"

#include <istring.hpp>

class SoundFile : public AudioFile
{
public:
  SoundFile();
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle);
  virtual LONG open(PSZ fileName);
  virtual LONG getHeader(PVOID header, LONG headerLength);
  virtual LONG setHeader(PVOID header, LONG headerLength);
  virtual LONG read(PSZ buffer, LONG bufferLength);
  virtual LONG seek(LONG seekPosition, LONG seekMode);
  virtual LONG write(PSZ buffer, LONG bufferLength);
  virtual LONG seekByTime(LONG seekTime, LONG seekMode);
  virtual LONG close(USHORT closeFlags);
  ~SoundFile();

private:
  void decompressAudio(PBYTE buffer, ULONG bufferLength);
  LONG trackSeekByTime(SampleTableAtom *initMap, float mediaSeekTime, LONG seekMode);

  MovieAtom *movieAtom; /* Top of QuickTime header structure */
  const TrackAtom *videoTrack, *soundTrack;
  const SampleTableAtom *videoMap, *soundMap;
  SampleTableAtom *current;
  ULONG filePosition, totalDataBytes, dataStartPosition;
  LONG nextBufferPosition;

  PCHAR fileBuffer;
  IString soundBuffer;
};

