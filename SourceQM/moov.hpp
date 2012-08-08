#include "moovproc.h"

#include "moviefmt.hpp"

#include <iostream.h>
#include <itbseq.h>
#include <iksset.h>

/*
 * MOOV
 */

class MOOV : public MovieFileFormat
{
public:
  virtual LONG identifyFile(PSZ fileName, HMMIO fileHandle);
};

/*
 * Following are the class definitions for the QuickTime header structure
 */

/*
 * AtomTag
 */

struct AtomTag
{
  ULONG size;
  char type[ATOM_TYPE_SIZE];
};

/*
 * ContainerAtom
 */

class ContainerAtom
{
public:
  ContainerAtom();
  ContainerAtom(AtomTag& atomTag);
  LONG load(HMMIO fileHandle);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag) = 0;

  AtomTag atomTag_p;
};

/*
 * TrackHeaderAtom
 */

struct TrackHeaderAtom
{
  char version;
  char flags[3];
  ULONG creationTime;
  ULONG modificationTime;
  ULONG track;
  char reserved1[4];
  ULONG duration;
  char reserved2[8];
  USHORT layer;
  USHORT alternateGroup;
  USHORT volume;
  char reserved3[2];
  char matrix[36];
  ULONG width;
  ULONG height;
};

/*
 * MediaHeaderAtom
 */

struct MediaHeaderAtom
{
  char version;
  char flags[3];
  ULONG creationTime;
  ULONG modificationTime;
  ULONG timeScale;
  ULONG duration;
  USHORT language;
  USHORT quality;
};


/*
 * HandlerReferenceAtom
 */

struct HandlerReferenceAtom
{
  char version;
  char flags[3];
  char componentType[4];
  char componentSubType[4];
  char componentManufacturer[4];
  ULONG componentFlags;
  ULONG componentFlagsMask;
  char *componentName;
};


#pragma pack(1)
/*
 * Color
 */

struct Color
{
  USHORT reserved1;
  USHORT red;
  USHORT green;
  USHORT blue;
};

/*
 * SampleDescription
 */

struct SampleDescription
{
  ULONG codecFourcc;
  ULONG reserved1;
  USHORT reserved2;
  USHORT dataReference;
  USHORT version;
  USHORT revisionLevel;
  char vendor[4];
  ULONG temporalQuality;
  ULONG spatialQuality;
  USHORT width;
  USHORT height;
  ULONG hResolution;
  ULONG vResolution;
  ULONG dataSize;
  USHORT frameCount;
  char nameLength;
  char name[31];
  USHORT depth;
  SHORT colorTable;
  Color firstColorEntry;
  Color palette[256];
};
#pragma pack()

/*
 * SampleDescriptionAtom
 */

struct SampleDescriptionAtom
{
  char version;
  char flags[3];
  ULONG numberOfEntries;
  ITabularSequence <SampleDescription> sampleDescriptionTable;
};

/*
 * TimeToSample
 */

struct TimeToSample
{
  ULONG count;
  ULONG duration;
};

/*
 * TimeToSampleAtom
 */

struct TimeToSampleAtom
{
  char version;
  char flags[3];
  ULONG numberOfEntries;
  ITabularSequence <TimeToSample> timeToSampleTable;
};

/*
 * SyncSampleAtom
 */

struct SyncSampleAtom
{
  char version;
  char flags[3];
  ULONG numberOfEntries;
  IKeySortedSet <ULONG, ULONG> syncSampleTable;
};

/*
 * SampleToChunk
 */

struct SampleToChunk
{
  ULONG firstChunk;
  ULONG samplesPerChunk;
  ULONG sampleDescription;
};

/*
 * SampleToChunkAtom
 */

struct SampleToChunkAtom
{
  char version;
  char flags[3];
  ULONG numberOfEntries;
  ITabularSequence <SampleToChunk> sampleToChunkTable;
};

/*
 * SampleSizeAtom
 */

struct SampleSizeAtom
{
  char version;
  char flags[3];
  ULONG sampleSize;
  ULONG numberOfEntries;
  ITabularSequence <ULONG> sampleSizeTable;
};

/*
 * ChunkOffsetAtom
 */

struct ChunkOffsetAtom
{
  char version;
  char flags[3];
  ULONG numberOfEntries;
  ITabularSequence <ULONG> chunkOffsetTable;
};

/*
 * SampleTableAtom
 */

class TrackAtom;

class SampleTableAtom : public ContainerAtom
{
public:
  SampleTableAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  ~SampleTableAtom();

  SampleDescriptionAtom* sampleDescriptionAtom;
  TimeToSampleAtom* timeToSampleAtom;
  SyncSampleAtom* syncSampleAtom;
  SampleToChunkAtom* sampleToChunkAtom;
  SampleSizeAtom* sampleSizeAtom;
  ChunkOffsetAtom* chunkOffsetAtom;

  /* Calculated during moov load */
  ULONG totalSamples, totalDuration, maxSampleSize;
  float averageDuration;

  /* Set during open */
  ITabularSequence<ULONG>::Cursor* chunkCursor;
  ITabularSequence<ULONG>::Cursor* sampleCursor;
  ITabularSequence<SampleToChunk>::Cursor* sampleToChunkCursor;
  const TrackAtom *track;
  LONG mmioTrack; /* mmioTrack to be replaced by position in map array */
  ULONG sampleSize;

  /* Used dynamically during read */
  ULONG samplesPerChunk;
  ULONG sampleInChunk;
  ULONG sampleInMedia;
  float currentTime;
  ULONG newFilePosition;
  ULONG maxEntries;
  ULONG mtReadIndex;
};

/*
 * VideoMediaInfoHeaderAtom
 */

struct VideoMediaInfoHeaderAtom
{
  char version;
  char flags[3];
  USHORT graphicsMode;
  char opcolor[6];
};

/*
 * SoundMediaInfoHeaderAtom
 */

struct SoundMediaInfoHeaderAtom
{
  char version;
  char flags[3];
  USHORT balance;
  char reserved1[2];
};

/*
 * MediaInformationAtom
 */

class MediaInformationAtom : public ContainerAtom
{
public:
  MediaInformationAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  ~MediaInformationAtom();

  VideoMediaInfoHeaderAtom* videoMediaInfoHeaderAtom;
  SoundMediaInfoHeaderAtom* soundMediaInfoHeaderAtom;
  HandlerReferenceAtom* dataHandlerReferenceAtom;
  SampleTableAtom* sampleTableAtom;
};

/*
 * MediaAtom
 */

class MediaAtom : public ContainerAtom
{
public:
  MediaAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  ~MediaAtom();

  MediaHeaderAtom mediaHeaderAtom;
  HandlerReferenceAtom* mediaHandlerReferenceAtom;
  MediaInformationAtom* mediaInformationAtom;
};

/*
 * EditList
 */

struct EditList
{
  ULONG duration;
  LONG mediaTime;
  ULONG rate;
};

/*
 * EditListAtom
 */

struct EditListAtom
{
  char version;
  char flags[3];
  ULONG numberOfEntries;
  ITabularSequence <EditList> editListTable;
};

/*
 * EditAtom
 */

class EditAtom : public ContainerAtom
{
public:
  EditAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  ~EditAtom();

  EditListAtom editListAtom;
};

/*
 * TrackAtom
 */

class TrackAtom : public ContainerAtom
{
public:
  TrackAtom();
  TrackAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  ~TrackAtom();

  TrackHeaderAtom trackHeaderAtom;
  MediaAtom *mediaAtom;
  EditAtom *editAtom;
};

/*
 * MovieHeaderAtom
 */

struct MovieHeaderAtom
{
  char version;
  char flags[3];
  ULONG creationTime;
  ULONG modificationTime;
  ULONG timeScale;
  ULONG duration;
  ULONG preferredRate;
  USHORT preferredVolume;
  char reserved1[10];
  char matrix[36];
  ULONG previewTime;
  ULONG previewDuration;
  ULONG posterTime;
  ULONG selectionTime;
  ULONG selectionDuration;
  ULONG currentTime;
  ULONG nextTrack;
};

/*
 * MovieAtom
 */

class MovieAtom : public ContainerAtom
{
public:
  MovieAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  ~MovieAtom();

  MovieHeaderAtom movieHeaderAtom;
  ITabularSequence <TrackAtom> trackList;
};

