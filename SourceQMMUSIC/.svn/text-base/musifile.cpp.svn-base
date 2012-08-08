#include "qmmusic.h"
#include "musifile.hpp"
#include "musifmt.hpp"

#include <pluginio.hpp>

/************************************************************************/
/* Constant strings used in MIDI output                                 */
/************************************************************************/
const IString MIDI_SYSEX_START_MMPM(IString("F000003A").x2c());
const IString MIDI_SYSEX_TIMING(IString("030118").x2c());
const IString MIDI_SYSEX_GM(IString("7E7F0901").x2c());
const IString MIDI_SYSEX_TEMPO(IString("0302300900").x2c());
const IString MIDI_SYSEX_START = '\xF0';
const IString MIDI_SYSEX_LONG_TIME = '\x01';
const IString MIDI_SYSEX_END = '\xF7';
const IString MIDI_TICK = '\xF8';
const IString MIDI_CHANNEL_VOLUME_NORMAL(IString("077F").x2c());
const IString NULLSTRING = '\x00';


#define _playNote(channel, pitch, velocity)           \
midiBuffer += (char)(MIDI_NOTE | channel);            \
midiBuffer += (char)pitch;                            \
midiBuffer += (velocity == 0 ? NULLSTRING : IString((char)velocity));

#define _waitFor(waitTime)                                      \
if (waitTime > 127)                                             \
{                                                               \
  midiBuffer += MIDI_SYSEX_START_MMPM + MIDI_SYSEX_LONG_TIME;   \
  ULONG waitLSB = waitTime & 0x7F;                              \
  midiBuffer += (waitLSB == 0 ? NULLSTRING : IString((char)waitLSB)); \
  ULONG waitMSB = waitTime >> 7 & 0x7F;                         \
  midiBuffer += (waitMSB == 0 ? NULLSTRING : IString((char)waitMSB)); \
  midiBuffer += MIDI_SYSEX_END;                                 \
}                                                               \
else if (waitTime > 6)                                          \
{                                                               \
  midiBuffer += MIDI_SYSEX_START_MMPM;                          \
  midiBuffer += (char)(waitTime & 0x7F);                        \
  midiBuffer += MIDI_SYSEX_END;                                 \
}                                                               \
else                                                            \
  for (int j = waitTime; j > 0; j--)                            \
    midiBuffer += MIDI_TICK;


/************************************************************
 * CONSTRUCTOR
 ************************************************************/
MusicFile::MusicFile()
        : musicTrack(NULL),
          musicMap(NULL)
{
  current = NULL;
  movieAtom = NULL;
  filePosition = totalDataBytes = 0;
  leaderReturned = false;
  fileBuffer = NULL;
}

/************************************************************
 * IDENTIFY FILE - Will come here if an info block is passed
 ************************************************************/
LONG MusicFile::identifyFile(PSZ fileName, HMMIO fileHandle)
{
  return MusicFormat().identifyFile(fileName, fileHandle);
}

/************************************************************
 * OPEN
 ************************************************************/
LONG MusicFile::open(PSZ fileName)
{
  LONG returnCode;

  returnCode = MIDIFile::open(fileName);
  if (returnCode != MMIO_SUCCESS)
    return (returnCode);

 /************************************************************
  * Read QuickTime header structure
  ************************************************************/
  movieAtom = new MovieAtom();
  returnCode = loadMovie(*movieAtom, totalDataBytes, storageSystemFileHandle());
  if (returnCode != MMIO_SUCCESS)
    return (returnCode);

 /************************************************************
  * Initialize cursors; Find musictrack
  ************************************************************/
  LONG nextTrack = 0;
  ITabularSequence<TrackAtom>::Cursor* trackCursor = movieAtom->trackList.newCursor();
  for (trackCursor->setToFirst(); trackCursor->isValid(); trackCursor->setToNext())
  {
    MediaInformationAtom* mia = trackCursor->element().mediaAtom->mediaInformationAtom;

    if (mia)
    {
      current = mia->sampleTableAtom;
      if (current && current->chunkOffsetAtom && current->sampleSizeAtom &&
          current->sampleToChunkAtom && current->timeToSampleAtom && current->sampleDescriptionAtom)
      {
        /* Initialize variables used in read */
        current->track = &trackCursor->element();
        current->chunkCursor = current->chunkOffsetAtom->chunkOffsetTable.newCursor();
        current->chunkCursor->setToFirst();
        if (current->chunkCursor->isValid())
          current->newFilePosition = current->chunkCursor->element();
        current->sampleCursor = current->sampleSizeAtom->sampleSizeTable.newCursor();
        current->sampleCursor->setToFirst();
        current->sampleToChunkCursor = current->sampleToChunkAtom->sampleToChunkTable.newCursor();
        current->sampleToChunkCursor->setToFirst();
        current->samplesPerChunk = current->sampleToChunkCursor->element().samplesPerChunk;
        current->sampleToChunkCursor->setToNext();
        current->sampleInChunk = 1;
        current->sampleInMedia = 1;
        current->currentTime = .0;

        /* Set musictrack shortcut to this track, if appropriate */
        if (mia->baseMediaInfoHeaderAtom &&
            !memcmp(&current->sampleDescriptionAtom->sampleDescriptionTable.elementAtPosition(1)->
                    musicDescription.codecFourcc, "musi", 4) &&
            (!musicTrack || musicTrack && current->totalDuration > musicMap->totalDuration))
        {
          current->mmioTrack = (musicMap ? musicMap->mmioTrack : nextTrack++);
          current->sampleSize = current->sampleSizeAtom->sampleSize;
          musicTrack = &trackCursor->element();
          current->mediaToTicks = MIDI_TICK_RATE /
                                  musicTrack->mediaAtom->mediaHeaderAtom.timeScale;
          musicMap = current;
        }
      } /* If current */
    } /* If mia */
  } /* While track */
  delete trackCursor;

  if (!musicTrack)
    return (MMIO_ERROR);

 /************************************************************
  * Fill out the MMMIDIHEADER structure.
  ************************************************************/
  midiHeader.ulHeaderLength = sizeof(MMMIDIHEADER);
  midiHeader.ulContentType = MMIO_MIDI_UNKNOWN;
  midiHeader.ulMediaType = MMIO_MEDIATYPE_MIDI;
  memcpy(&midiHeader.midiheader.chHeaderChunk, "MThd", 4);
  midiHeader.midiheader.ulHeaderLength = 6;
  midiHeader.midiheader.usFormat = 0; /* 1 for multi-track file */
  midiHeader.midiheader.usNumTracks = 1;
  midiHeader.midiheader.usDivision = MIDI_BEATS_PER_SECOND * 60;
  midiHeader.midiheader.vpAdditionalInformation = NULL;

  /************************************************************
  * Construct leader
  ************************************************************/
  midiLeader = MIDI_SYSEX_START_MMPM + MIDI_SYSEX_TIMING;
  char tickRateMultiplier = MIDI_TICK_RATE / MIDI_CLOCKS_PER_BEAT / MIDI_BEATS_PER_SECOND - 1;
  midiLeader += IString(tickRateMultiplier) + NULLSTRING + MIDI_SYSEX_END;
  midiLeader += MIDI_SYSEX_START + MIDI_SYSEX_GM + MIDI_SYSEX_END;
  midiLeader += MIDI_SYSEX_START_MMPM + MIDI_SYSEX_TEMPO + MIDI_SYSEX_END;

  const MusicDescription& musicDescription = musicMap->sampleDescriptionAtom->
    sampleDescriptionTable.elementAtPosition(1)->musicDescription;
  ULONG melodicChannel = 0;
  ULONG percussionChannel = MIDI_PERCUSSION_CHANNEL;
  for (int i = 0; i < musicDescription.reserved1; i++)
  {
    /* Set up part -> channel table */
    Part part;
    part.part = ((musicDescription.noteRequestEvent[i].F0 & 0xF) << 8) |
                 musicDescription.noteRequestEvent[i].part8;
    ULONG instrument = musicDescription.noteRequestEvent[i].noteRequest.toneDescription.gmNumber - 1;
    part.channel = (instrument < MIDI_DRUM_KIT ? melodicChannel++ : percussionChannel);
    partTable.add(part);
    /* Patch in the instrument */
    if (instrument >= MIDI_DRUM_KIT)
      instrument = 0;
    midiLeader += IString((char)(MIDI_PATCH | part.channel)) + IString((char)instrument);
    /* Set normal volume */
    midiLeader += IString((char)(MIDI_CONTROLLER | part.channel)) + MIDI_CHANNEL_VOLUME_NORMAL;
  }
  partCursor = partTable.newCursor();

  /* Allocate file buffer */
  fileBuffer = new CHAR[musicMap->maxSampleSize];

  currentTime = lastEventTime = 0;

  midiBuffer = midiLeader;

  return (MMIO_SUCCESS);
}

/************************************************************
 * GET FILE LENGTH
 ************************************************************/
LONG MusicFile::getFileLength()
{
  return musicMap->totalDuration * 3000.0 / musicTrack->mediaAtom->mediaHeaderAtom.timeScale;
}

/************************************************************
 * GET HEADER
 ************************************************************/
LONG MusicFile::getHeader(PVOID header, LONG headerLength)
{
  if (!header)
  {
    setError(MMIOERR_INVALID_STRUCTURE);
    return (MMIO_ERROR);
  }
  else
  {
    memcpy(header, (PVOID)&midiHeader, sizeof(MMMIDIHEADER));
    return (sizeof(MMMIDIHEADER));
   }
}

/************************************************************
 * SET HEADER
 ************************************************************/
LONG MusicFile::setHeader(PVOID header, LONG headerLength)
{
  return (0);
}

/************************************************************
 * currentTrackMap - Read and Seek Helper Function
 ************************************************************/
SampleTableAtom *MusicFile::currentTrackMap()
{
  return (SampleTableAtom *)musicMap;
}

/************************************************************
 * midiFromQTMA - Translates QTMA events to MIDI
 ************************************************************/
void MusicFile::midiFromQTMA(PULONG buffer, ULONG bufferLength)
{
  ULONG event, event2, eventType, part, velocity, duration, waitTime, controller, value;
  NoteEnd noteEnd;

  for (int i = bufferLength; i > 0; i--)
  {
    event = *buffer++;
    reverseBytes(&event);
    eventType = event & 0xE0000000;

    switch (eventType)
    {
      case QTMA_NOTE:
        part = event >> 24 & 0x1F;
        noteEnd.pitch = (event >> 18 & 0x3F) + 32;
        velocity = event >> 11 & 0x7F;
        duration = (event & 0x7FF) * MIDI_TICK_RATE /
                                     musicTrack->mediaAtom->mediaHeaderAtom.timeScale;

        if (partTable.containsElementWithKey(part))
        {
          noteEnd.channel = partTable.elementWithKey(part).channel;
          _playNote(noteEnd.channel, noteEnd.pitch, velocity);
          noteEnd.endTime = currentTime + duration;
          noteEndTable.add(noteEnd);
#ifdef DEBUGGING_IOPROC
//  debugOutput <<  currentTime << "\tStart\t" << noteEnd.endTime << '\t' << noteEnd.channel << '\t' << noteEnd.pitch << endl;
#endif
        } else {
#ifdef DEBUGGING_IOPROC
  debugOutput <<  currentTime << "\t note badPart\t" << part << '\t' << noteEnd.pitch << '\t' << velocity << '\t' << duration << endl;
#endif
        }
        break;

      case QTMA_EXTENDED_NOTE:
        event2 = *buffer++;
        reverseBytes(&event2);
        i--;
        part = event >> 16 & 0xFFF;
        noteEnd.pitch = event & 0x7FFF;
        velocity = event2 >> 22 & 0x7F;
        duration = (event2 & 0x3FFFFF) * MIDI_TICK_RATE /
                                         musicTrack->mediaAtom->mediaHeaderAtom.timeScale;

        if (partTable.containsElementWithKey(part))
        {
          noteEnd.channel = partTable.elementWithKey(part).channel;
          _playNote(noteEnd.channel, noteEnd.pitch, velocity);
          noteEnd.endTime = currentTime + duration;
          noteEndTable.add(noteEnd);
#ifdef DEBUGGING_IOPROC
//  debugOutput <<  currentTime << "\tStart\t" << noteEnd.endTime << '\t' << noteEnd.channel << '\t' << noteEnd.pitch << endl;
#endif
        } else {
#ifdef DEBUGGING_IOPROC
 debugOutput <<  currentTime << "\t extNote badPart\t" << part << '\t' << noteEnd.pitch << '\t' << velocity << '\t' << duration << endl;
#endif
        }
        break;

      case QTMA_CONTROLLER:
        part = event >> 24 & 0x1F;
        controller = event >> 16 & 0x7F;
        value = event >> 8 & 0x7F;   /* What are the last 8 bits of this event? */

        if (partTable.containsElementWithKey(part))
        {
          noteEnd.channel = partTable.elementWithKey(part).channel;
          midiBuffer += (char)(MIDI_CONTROLLER | noteEnd.channel);
          midiBuffer += (char)controller;
          midiBuffer += (char)value;
        } else {
#ifdef DEBUGGING_IOPROC
 debugOutput <<  currentTime << "\tcontroller badPart\t" << part << '\t' << controller << '\t' << value << endl;
#endif
        }
        break;

      case QTMA_REST:
        duration = (event & 0xFFFFF) * MIDI_TICK_RATE /
                                       musicTrack->mediaAtom->mediaHeaderAtom.timeScale;
        currentTime += duration;
        while (!noteEndTable.isEmpty() && noteEndTable.firstElement().endTime <= currentTime)
        {
          noteEnd = noteEndTable.firstElement();
          waitTime = noteEnd.endTime - lastEventTime;
          _waitFor(waitTime);
          _playNote(noteEnd.channel, noteEnd.pitch, 0);
#ifdef DEBUGGING_IOPROC
//  debugOutput <<  lastEventTime << "\tEnd\t" << noteEnd.endTime << '\t' << noteEnd.channel << '\t' << noteEnd.pitch << '\t' << noteEndTable.numberOfElements() << endl;
#endif
          lastEventTime = noteEnd.endTime;
          noteEndTable.removeAtPosition(1);
        }
        waitTime = currentTime - lastEventTime;
        if (waitTime)
        {
          _waitFor(waitTime);
          lastEventTime = currentTime;
        }
        break;

      case QTMA_MARKER:
//        while (!noteEndTable.isEmpty())
//        {
//          noteEnd = noteEndTable.firstElement();
#ifdef DEBUGGING_IOPROC
//  debugOutput <<  currentTime << "\tClear\t" << noteEnd.endTime << '\t' << noteEnd.channel << '\t' << noteEnd.pitch << '\t' << noteEndTable.numberOfElements() << endl;
#endif
//          _playNote(noteEnd.channel, noteEnd.pitch, 0);
//          noteEndTable.removeAtPosition(1);
//        }
        break;

      default:
      //  *(char*)0 = 0;
        break;
    }
  }
}

/************************************************************
 * READ
 ************************************************************/
LONG MusicFile::read(PSZ buffer, LONG bufferLength)
{
  ULONG bytesToRead;
  ULONG samplesToRead = 1;

  /* Set track to that of the first chunk. */
  if (filePosition == 0)
    current = currentTrackMap();

  /* Get records while more data in file,*/
  while (current->chunkCursor->isValid() && midiBuffer.length() < bufferLength)
  {
    /* Seek to better-known file position, if available */
    if (filePosition != current->newFilePosition)
      filePosition = mmioSeek(storageSystemFileHandle(), current->newFilePosition, SEEK_SET);

    /* Set bytes to read based on fixed sample size or sample size table */
    if (current->sampleSize)
      bytesToRead = samplesToRead * current->sampleSize;
    else
      bytesToRead = samplesToRead * current->sampleCursor->element();

    /* Read */
    try
    {
      filePosition += mmioRead(storageSystemFileHandle(), fileBuffer, bytesToRead);
    }
    catch (NoMoreData noMoreData)
    {
      return 0;
    }

    /* Translate QTMA -> MIDI */
    midiFromQTMA((PULONG)fileBuffer, bytesToRead / 4);

     /* Update position tracking variables */
    current->newFilePosition += bytesToRead;
    current->currentTime += samplesToRead * current->averageDuration;
    current->sampleInMedia += samplesToRead;
    current->sampleInChunk += samplesToRead;
    if (!current->sampleSize)
      current->sampleCursor->setToNext();

    /* Update chunk cursors if we have hit the end of the chunk */
    if (current->sampleInChunk > current->samplesPerChunk)
    {
      current->chunkCursor->setToNext();

      /* Update samples-per-chunk value, if necessary */
      if (current->chunkCursor->isValid())
      {
        current->newFilePosition = current->chunkCursor->element();
        if (current->sampleToChunkCursor->isValid() &&
            current->chunkOffsetAtom->chunkOffsetTable.position(*(current->chunkCursor)) ==
            current->sampleToChunkCursor->element().firstChunk)
        {
          current->samplesPerChunk = current->sampleToChunkCursor->element().samplesPerChunk;
          current->sampleToChunkCursor->setToNext();
        }
      }
      current->sampleInChunk = 1;
      /* Update current track map pointer */
      current = currentTrackMap();
    } /* New chunk */
  } /* current->chunkCursor->isValid() */

  /* Give it up! */
  ULONG bytesToCopy = min(midiBuffer.length(), bufferLength);
  memcpy(buffer, (PCHAR)midiBuffer, bytesToCopy);
  midiBuffer.remove(1, bytesToCopy);

  return(bytesToCopy);
}

/************************************************************
 * WRITE
 ************************************************************/
LONG MusicFile::write(PSZ buffer, LONG bufferLength)
{
  return (0);
}

/************************************************************
 * SEEKBYTIME
 ************************************************************/
LONG MusicFile::seekByTime(LONG seekTime, LONG seekMode)
{
  float mediaSeekTime = seekTime / 3000.0 * musicTrack->mediaAtom->mediaHeaderAtom.timeScale;
  float newTime;

  switch (seekMode & (SEEK_SET | SEEK_CUR | SEEK_END))
  {
    case SEEK_SET:
    {
      newTime = mediaSeekTime;
      break;
    }
    case SEEK_CUR:
    {
      newTime = current->currentTime + mediaSeekTime;
      break;
    }
    case SEEK_END:
    {
      newTime = current->totalDuration + mediaSeekTime;
      break;
    }
    default :
      return (MMIO_ERROR);
  }

  /* Rewind */
  current->chunkCursor->setToFirst();
  if (current->chunkCursor->isValid())
    current->newFilePosition = current->chunkCursor->element();
  current->sampleCursor->setToFirst();
  current->sampleToChunkCursor->setToFirst();
  current->samplesPerChunk = current->sampleToChunkCursor->element().samplesPerChunk;
  current->sampleToChunkCursor->setToNext();
  current->sampleInChunk = 1;
  current->sampleInMedia = 1;
  current->currentTime = .0;

  /* Create a time-to-sample cursor for this special operation */
  ITabularSequence<TimeToSample>::Cursor* timeToSampleCursor = current->timeToSampleAtom->timeToSampleTable.newCursor();
  timeToSampleCursor->setToFirst();
  unsigned long sampleInCount = 1;

  /* Get records while more data in file,*/
  while (current->chunkCursor->isValid() && current->currentTime < newTime)
  {
    /* Update position tracking variables */
    current->newFilePosition += current->sampleCursor->element();
    current->currentTime += timeToSampleCursor->element().duration;
    current->sampleInMedia++;
    current->sampleInChunk++;
    sampleInCount++;
    current->sampleCursor->setToNext();

    /* Update time-to-sample cursor if we have hit the end of the current count */
    if (sampleInCount > timeToSampleCursor->element().count)
    {
      timeToSampleCursor->setToNext();
      sampleInCount = 1;
    }

    /* Update chunk cursors if we have hit the end of the chunk */
    if (current->sampleInChunk > current->samplesPerChunk)
    {
      current->chunkCursor->setToNext();

      /* Update samples-per-chunk value, if necessary */
      if (current->chunkCursor->isValid())
      {
        current->newFilePosition = current->chunkCursor->element();
        if (current->sampleToChunkCursor->isValid() &&
            current->chunkOffsetAtom->chunkOffsetTable.position(*(current->chunkCursor)) ==
            current->sampleToChunkCursor->element().firstChunk)
        {
          current->samplesPerChunk = current->sampleToChunkCursor->element().samplesPerChunk;
          current->sampleToChunkCursor->setToNext();
        }
      }
      current->sampleInChunk = 1;
    } /* New chunk */
  } /* current->chunkCursor->isValid() */

  delete timeToSampleCursor;

  noteEndTable.removeAll();
  currentTime = lastEventTime = current->currentTime * MIDI_TICK_RATE /
                                musicTrack->mediaAtom->mediaHeaderAtom.timeScale;;
#ifdef DEBUGGING_IOPROC
  debugOutput <<  "Sought: " << currentTime << '\t' << current->currentTime << endl;
#endif
  midiBuffer = midiLeader;

  return (current->currentTime * 3000.0 / musicTrack->mediaAtom->mediaHeaderAtom.timeScale);
}

/************************************************************
 * CLOSE
 ************************************************************/
LONG MusicFile::close(USHORT closeFlags)
{
  delete movieAtom;
  delete fileBuffer;
  delete partCursor;

  return (MIDIFile::close(closeFlags));
}

MusicFile::~MusicFile()
{
}

ULONG const& key(Part const& element) {return element.part;};
ULONG const& key(NoteEnd const& element) {return element.endTime;};
