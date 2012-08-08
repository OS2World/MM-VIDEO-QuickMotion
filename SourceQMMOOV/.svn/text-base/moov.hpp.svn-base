#include "qmmoov.h"

#include <iostream.h>
#include <itbseq.h>
#include <iksset.h>


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
  LONG _Export load(HMMIO fileHandle);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag) = 0;

  AtomTag atomTag_p;
};

/*
 * NavigationAtom
 */

struct NavigationAtom
{
  USHORT version;
  USHORT numberOfColumns;
  USHORT numberOfRows;
  USHORT reserved1;
  USHORT loopSize;
  USHORT frameDuration;
  USHORT movieType;
  USHORT loopTicks;
  float fieldOfView;
  float startHPan;
  float endHPan;
  float startVPan;
  float endVPan;
  float initialHPan;
  float initialVPan;
  ULONG reserved2;
};
ostream& _Export operator<<(ostream& outStream, const NavigationAtom& navigationAtom);


/*
 * ControllerTypeAtom
 */

struct ControllerTypeAtom
{
  ULONG controllerFourcc;
};
ostream& _Export operator<<(ostream& outStream, const ControllerTypeAtom& controllerTypeAtom);


/*
 * UserDataAtom
 */

class UserDataAtom : public ContainerAtom
{
public:
  UserDataAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  ~UserDataAtom();

  NavigationAtom *navigationAtom;
  ControllerTypeAtom *controllerTypeAtom;
};
ostream& _Export operator<<(ostream& outStream, const UserDataAtom& userDataAtom);


#pragma pack(1)
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
  char matrix[36];
  ULONG width;
  ULONG height;
  char reserved3[2];
};
#pragma pack()

ostream& _Export operator<<(ostream& outStream, const TrackHeaderAtom& trackHeaderAtom);

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
ostream& _Export operator<<(ostream& outStream, const MediaHeaderAtom& mediaHeaderAtom);


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
  char componentName[256];
};
ostream& _Export operator<<(ostream& outStream, const HandlerReferenceAtom& handlerReferenceAtom);


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
 * ImageDescription
 */

struct ImageDescription
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

/*
 * SoundDescription
 */

struct SoundDescription
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

/*
 * NoteRequest
 */

struct NoteRequestInfo
{
  char flags;
  char reserved1;
  USHORT polyphony;
  USHORT typicalPolyphony;
};

struct ToneDescription
{
  char synthesizerType[6];
  char synthesizerNameLength;
  char synthesizerName[31];
  char instrumentNameLength;
  char instrumentName[31];
  ULONG instrumentNumber;
  ULONG gmNumber;
};

struct NoteRequest
{
  NoteRequestInfo noteRequestInfo;
  ToneDescription toneDescription;
};

struct NoteRequestEvent
{
  char F0;
  char part8;
  USHORT structLen; /* = 0x17 */
  NoteRequest noteRequest;
  ULONG tail;
};

/*
 * MusicDescription
 */

struct MusicDescription
{
  ULONG codecFourcc;
  ULONG reserved1; /* Used by QuickMotion for number of parts */
  char reserved2[8];
  NoteRequestEvent noteRequestEvent[1];
};

/*
 * PanoramaDescription
 */

struct PanoramaDescription
{
  ULONG codecFourcc;
  ULONG reserved1;
  ULONG reserved2;
  USHORT majorVersion;
  USHORT minorVersion;
  ULONG sceneTrackID;
  ULONG loResSceneTrackID;
  ULONG reserved3[6];
  ULONG hotSpotTrackID;
  ULONG reserved4[9];
  float hPanStart;
  float hPanEnd;
  float vPanTop;
  float vPanBottom;
  float minimumZoom;
  float maximumZoom;
  ULONG sceneSizeX;
  ULONG sceneSizeY;
  ULONG numFrames;
  USHORT reserved5;
  USHORT sceneNumFramesX;
  USHORT sceneNumFramesY;
  USHORT sceneColorDepth;
  ULONG hotSpotSizeX;
  ULONG hotSpotSizeY;
  USHORT reserved6;
  USHORT hotSpotNumFramesX;
  USHORT hotSpotNumFramesY;
  USHORT hotSpotColorDepth;
};

#pragma pack()

 /*
 * SampleDescription
 */

union SampleDescription
{
  ImageDescription imageDescription;
  SoundDescription soundDescription;
  MusicDescription musicDescription;
  PanoramaDescription panoramaDescription;
};

/*
 * SampleDescriptionAtom
 */

struct SampleDescriptionAtom
{
  char version;
  char flags[3];
  ULONG numberOfEntries;
  ITabularSequence <SampleDescription*> sampleDescriptionTable;
};
ostream& _Export operator<<(ostream& outStream, const SampleDescriptionAtom& sampleDescriptionAtom);

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
ostream& _Export operator<<(ostream& outStream, const TimeToSampleAtom& timeToSampleAtom);

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
ostream& _Export operator<<(ostream& outStream, const SyncSampleAtom& syncSampleAtom);


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
ostream& _Export operator<<(ostream& outStream, const SampleToChunkAtom& sampleToChunkAtom);

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
ostream& _Export operator<<(ostream& outStream, const SampleSizeAtom& sampleSizeAtom);

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
ostream& _Export operator<<(ostream& outStream, const ChunkOffsetAtom& chunkOffsetAtom);

/*
 * SampleTableAtom
 */

class TrackAtom;

class SampleTableAtom : public ContainerAtom
{
public:
  SampleTableAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  void _Export seekByTime(float mediaSeekTime, LONG seekMode);
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

  /* Added after QUICKMO release */
  ULONG mediaToTicks;
};
ostream& _Export operator<<(ostream& outStream, const SampleTableAtom& sampleTableAtom);

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
ostream& _Export operator<<(ostream& outStream, const VideoMediaInfoHeaderAtom& videoMediaInfoHeaderAtom);


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
ostream& _Export operator<<(ostream& outStream, const SoundMediaInfoHeaderAtom& soundMediaInfoHeaderAtom);


/*
 * BaseMediaInfoAtom
 */

struct BaseMediaInfoAtom
{
  char version;
  char flags[3];
  USHORT graphicsMode;
  USHORT opcolor[3];
  USHORT balance;
  char reserved1[2];
};
ostream& _Export operator<<(ostream& outStream, const BaseMediaInfoAtom& baseMediaInfoAtom);

/*
 * IDTableEntry
 */

struct IDTableEntry
{
  ULONG nodeID;
  ULONG nodeTime;
};

/*
 * PanoramaMediaInfoAtom
 */

struct PanoramaMediaInfoAtom
{
  ULONG sizePInf;
  ULONG typePInf;
  char nameLength;
  char name[31];
  ULONG defNodeID;
  float defZoom;
  LONG reserved1;
  SHORT reserved2;
  SHORT numEntries;
  IDTableEntry idToTime[1];
};

/*
 * BaseMediaInfoHeaderAtom
 */

class BaseMediaInfoHeaderAtom : public ContainerAtom
{
public:
  BaseMediaInfoHeaderAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  ~BaseMediaInfoHeaderAtom();

  BaseMediaInfoAtom baseMediaInfoAtom;
  PanoramaMediaInfoAtom *panoramaMediaInfoAtom;
};
ostream& _Export operator<<(ostream& outStream, const BaseMediaInfoHeaderAtom& baseMediaInfoHeaderAtom);


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
  BaseMediaInfoHeaderAtom* baseMediaInfoHeaderAtom;
  HandlerReferenceAtom* dataHandlerReferenceAtom;
  SampleTableAtom* sampleTableAtom;
};
ostream& _Export operator<<(ostream& outStream, const MediaInformationAtom& mediaInformationAtom);

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
ostream& _Export operator<<(ostream& outStream, const MediaAtom& mediaAtom);

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
ostream& _Export operator<<(ostream& outStream, const EditListAtom& editListAtom);

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
ostream& _Export operator<<(ostream& outStream, const EditAtom& editAtom);

/*
 * TrackAtom
 */

class TrackAtom : public ContainerAtom
{
public:
  _Export TrackAtom();
  TrackAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  _Export ~TrackAtom();

  TrackHeaderAtom trackHeaderAtom;
  MediaAtom *mediaAtom;
  EditAtom *editAtom;
};
ostream& _Export operator<<(ostream& outStream, const TrackAtom& trackAtom);

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
ostream& _Export operator<<(ostream& outStream, const MovieHeaderAtom& movieHeaderAtom);

/*
 * MovieAtom
 */

class MovieAtom : public ContainerAtom
{
public:
  _Export MovieAtom();
  _Export MovieAtom(AtomTag& atomTag);
  virtual LONG loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag);
  virtual Boolean _Export isWorld();
  virtual Boolean _Export hasVideo();
  virtual Boolean _Export isMusicOnly();
  virtual Boolean _Export isSoundOnly();
  _Export ~MovieAtom();

  MovieHeaderAtom movieHeaderAtom;
  ITabularSequence <TrackAtom> trackList;
  UserDataAtom *userDataAtom;

  Boolean hasVideo_p, isMusicOnly_p, isSoundOnly_p;
};
ostream& _Export operator<<(ostream& outStream, const MovieAtom& movieAtom);

LONG _Export loadMovie(MovieAtom& movieAtom, ULONG& totalDataBytes, HMMIO fileHandle);

/*
 * QTVR 1.0 Panorama Track Formats
 */

struct PanoSampleHeaderAtom
{
  ULONG nodeID;
  float defHPan;
  float defVPan;
  float defZoom;
  float minHPan;
  float minVPan;
  float minZoom;
  float maxHPan;
  float maxVPan;
  float maxZoom;
  LONG reserved1;
  LONG reserved2;
  LONG nameStrOffset;
  LONG commentStrOffset;
};

#pragma pack(1)

/*
 * HotSpot
 */

struct HotSpot
{
  USHORT hotSpotID;
  SHORT reserved1;
  ULONG type;
  ULONG typeData;
  float viewHPan;
  float viewVPan;
  float viewZoom;
  USHORT hotSpotRectCoord1;
  USHORT hotSpotRectCoord2;
  USHORT hotSpotRectCoord3;
  USHORT hotSpotRectCoord4;
  LONG mouseOverCursorID;
  LONG mouseDownCursorID;
  LONG mouseUpCursorID;
  LONG reserved2;
  LONG nameStrOffset;
  LONG commentStrOffset;
};

/*
 * HotSpotTableAtom
 */

struct HotSpotTableAtom
{
  SHORT reserved1;
  SHORT numHotSpots;
  HotSpot hotSpots[1];
};

/*
 * PanoLink
 */

struct PanoLink
{
  USHORT linkID;
  SHORT reserved1;
  LONG reserved2;
  LONG reserved3;
  ULONG toNodeID;
  LONG reserved4[3];
  float toHPan;
  float toVPan;
  float toZoom;
  LONG reserved5;
  LONG reserved6;
  LONG nameStrOffset;
  LONG commentStrOffset;
};

/*
 * LinkTableAtom
 */

struct LinkTableAtom
{
  SHORT reserved1;
  SHORT numLinks;
  PanoLink links[1];
};

/*
 * NavgObject
 */

struct NavgObject
{
  USHORT objID;
  SHORT reserved1;
  LONG reserved2;
  float navgHPan;
  float navgVPan;
  float navgZoom;
  USHORT zoomRectCoord1;
  USHORT zoomRectCoord2;
  USHORT zoomRectCoord3;
  USHORT zoomRectCoord4;
  LONG reserved3;
  LONG nameStrOffset;
  LONG commentStrOffset;
};

/*
 * NavgTableAtom
 */

struct NavgTableAtom
{
  SHORT reserved1;
  SHORT numObjects;
  NavgObject objects[1];
};

#pragma pack()

/*
 * PanoramaSample
 */

class PanoramaSample
{
public:
  _Export PanoramaSample();
  void _Export load(HMMIO fileHandle, ULONG size);
  _Export ~PanoramaSample();

  PanoSampleHeaderAtom panoSampleHeaderAtom;
  HotSpotTableAtom *hotSpotTableAtom;
  LinkTableAtom *linkTableAtom;
  NavgTableAtom *navgTableAtom;
  char *stringTableAtom;
};

