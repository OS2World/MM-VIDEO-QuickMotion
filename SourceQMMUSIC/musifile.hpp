#include "qmmusic.h"
#include "midifile.hpp"
#include "moov.hpp"

#include <istring.hpp>
#include <iksbag.h>

struct Part
{
  ULONG part;
  ULONG channel;
};

struct NoteEnd
{
  ULONG endTime;
  ULONG channel;
  ULONG pitch;
};

ULONG const& key(Part const& element);
ULONG const& key(NoteEnd const& element);

class MusicFile : public MIDIFile
{
public:
  MusicFile();
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle);
  virtual LONG open(PSZ fileName);
  virtual LONG getFileLength();
  virtual LONG getHeader(PVOID header, LONG headerLength);
  virtual LONG setHeader(PVOID header, LONG headerLength);
  virtual LONG read(PSZ buffer, LONG bufferLength);
  virtual LONG write(PSZ buffer, LONG bufferLength);
  virtual LONG seekByTime(LONG seekTime, LONG seekMode);
  virtual LONG close(USHORT closeFlags);
  ~MusicFile();

private:
  SampleTableAtom *currentTrackMap();
  void midiFromQTMA(PULONG buffer, ULONG bufferLength);

  MovieAtom *movieAtom; /* Top of QuickTime header structure */

  const TrackAtom *musicTrack;
  const SampleTableAtom *musicMap;
  SampleTableAtom *current;
  ULONG currentTime, lastEventTime;

  IKeySortedSet <Part, ULONG> partTable;
  IKeySortedSet<Part, ULONG>::Cursor* partCursor;
  IKeySortedBag <NoteEnd, ULONG> noteEndTable;

  ULONG filePosition, totalDataBytes;

  Boolean leaderReturned;
  PCHAR fileBuffer;
  IString midiLeader, midiBuffer;
};

