#include "moovproc.h"

#include "moviefil.hpp"
#include "moov.hpp"

class MOOVFile : public MovieFile
{
public:
  MOOVFile();
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle);
  virtual LONG open(PSZ fileName);
  virtual LONG getHeader(PVOID header, LONG headerLength);
  virtual LONG setHeader(PVOID header, LONG headerLength);
  virtual LONG read(PSZ buffer, LONG bufferLength);
  virtual LONG multiTrackRead(PMMMULTITRACKREAD multiTrackInfo);
  virtual LONG write(PSZ buffer, LONG bufferLength);
  virtual LONG seekByTime(LONG seekTime, LONG seekMode);
  virtual LONG close(USHORT closeFlags);
  ~MOOVFile();

private:
  SampleTableAtom *currentTrackMap(LONG track);
  void positionToCurrentSample();
  void decompressAudio(PBYTE buffer, ULONG bufferLength);
  LONG trackSeekByTime(SampleTableAtom *initMap, float mediaSeekTime, LONG seekMode);

  MovieAtom *movieAtom;
  const TrackAtom *videoTrack, *soundTrack;
  const SampleTableAtom *videoMap, *soundMap;
  SampleTableAtom *current;
  /*------------- Cannot change anything before this or QUICKMO will break --------------- */
  ULONG filePosition, totalDataBytes;
  LONG nextBufferPosition;
  BOOL autoInterleave, firstFrameDone;
  BOOL firstPluginFrame();

friend void EXPENTRY QUICKMO(MOOVFile *moovFile, PMMMULTITRACKREAD multiTrackInfo);
};

