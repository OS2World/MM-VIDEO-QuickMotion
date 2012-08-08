#include "qmsound.h"
#include "soundfil.hpp"
#include "soundfmt.hpp"

#include <ima4.h>
#include <pluginio.hpp>

#include <string.h>
#include <ctype.h>
#include <math.h>

/************************************************************
 * CONSTRUCTOR
 ************************************************************/
SoundFile::SoundFile()
        : soundTrack(0),
          soundMap(0),
          current(0)
{
  movieAtom = 0;
  filePosition = totalDataBytes = dataStartPosition = nextBufferPosition = 0;
}

/************************************************************
 * IDENTIFY FILE - Will come here if an info block is passed
 ************************************************************/
LONG SoundFile::identifyFile(PSZ fileName, HMMIO fileHandle)
{
  return SoundFileFormat().identifyFile(fileName, fileHandle);
}

/************************************************************
 * OPEN
 ************************************************************/
LONG SoundFile::open(PSZ fileName)
{
  LONG returnCode;

  returnCode = AudioFile::open(fileName);
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
  * Initialize cursors; Find soundtrack
  ************************************************************/
  ITabularSequence<TrackAtom>::Cursor* trackCursor = movieAtom->trackList.newCursor();
  for (trackCursor->setToFirst(); trackCursor->isValid(); trackCursor->setToNext())
  {
    MediaInformationAtom* mia = trackCursor->element().mediaAtom->mediaInformationAtom;

    if (mia)
    {
      current = mia->sampleTableAtom;
      if (current && current->chunkOffsetAtom && current->sampleSizeAtom &&
          current->sampleToChunkAtom && current->timeToSampleAtom)
      {
        /* Initialize variables used in multiTrackRead */
        current->track = &trackCursor->element();
        current->chunkCursor = current->chunkOffsetAtom->chunkOffsetTable.newCursor();
        current->chunkCursor->setToFirst();
        if (current->chunkCursor->isValid())
          dataStartPosition = current->newFilePosition = current->chunkCursor->element();
        current->sampleCursor = current->sampleSizeAtom->sampleSizeTable.newCursor();
        current->sampleCursor->setToFirst();
        current->sampleToChunkCursor = current->sampleToChunkAtom->sampleToChunkTable.newCursor();
        current->sampleToChunkCursor->setToFirst();
        current->samplesPerChunk = current->sampleToChunkCursor->element().samplesPerChunk;
        current->sampleToChunkCursor->setToNext();
        current->sampleInChunk = 1;
        current->sampleInMedia = 1;
        current->currentTime = .0;

        if (mia->soundMediaInfoHeaderAtom &&
            (!soundTrack || soundTrack && current->totalDuration > soundMap->totalDuration))
        {
          ULONG soundCodec = mia->sampleTableAtom->sampleDescriptionAtom->sampleDescriptionTable.
                             elementAtPosition(1)->imageDescription.codecFourcc;
          if (soundCodec == FOURCC_RAW || soundCodec == FOURCC_TWOS ||
              soundCodec == FOURCC_BLANKS || soundCodec == FOURCC_IMA4 || !soundCodec)
          {
          soundTrack = current->track;
          soundMap = current;
          }
        } /* If sound */
      } /* If current */
    }  /* If mia */
  } /* trackCursor */
  delete trackCursor;

  if (!soundTrack)
    return (MMIO_ERROR);

  /************************************************************
  * Fill out the MMAUDIOHEADER structure.
  ************************************************************/
  audioHeader.ulHeaderLength = sizeof(MMAUDIOHEADER);
  audioHeader.ulContentType = MMIO_AUDIO_UNKNOWN;
  audioHeader.ulMediaType = MMIO_MEDIATYPE_AUDIO;
  audioHeader.mmXWAVHeader.WAVEHeader.usFormatTag = MCI_WAVE_FORMAT_PCM;

  const SoundDescription& soundDescription = soundMap->sampleDescriptionAtom->
    sampleDescriptionTable.elementAtPosition(1)->soundDescription;

  audioHeader.mmXWAVHeader.WAVEHeader.usChannels = HIUSHORT(soundDescription.temporalQuality);
  audioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample = LOUSHORT(soundDescription.temporalQuality);
  audioHeader.mmXWAVHeader.WAVEHeader.usBlockAlign = audioHeader.mmXWAVHeader.WAVEHeader.usChannels *
    (audioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample + 7 >> 3);
  ((SampleTableAtom *)soundMap)->sampleSize = audioHeader.mmXWAVHeader.WAVEHeader.usBlockAlign;

  audioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec =
    soundTrack->mediaAtom->mediaHeaderAtom.timeScale / soundMap->averageDuration;
  audioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec =
    audioHeader.mmXWAVHeader.WAVEHeader.usBlockAlign *
    audioHeader.mmXWAVHeader.WAVEHeader.ulSamplesPerSec;

  audioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInMS =
    soundMap->totalDuration * 1000.0 / soundTrack->mediaAtom->mediaHeaderAtom.timeScale;
  audioHeader.mmXWAVHeader.XWAVHeaderInfo.ulAudioLengthInBytes =
    soundMap->totalSamples * audioHeader.mmXWAVHeader.WAVEHeader.usBlockAlign;
  audioHeader.mmXWAVHeader.XWAVHeaderInfo.pAdditionalInformation = (PVOID)soundDescription.codecFourcc;

  /* Adjust sample size by blocking factor if audio is compressed */
  if (soundDescription.codecFourcc == FOURCC_IMA4)
  {
    current = (SampleTableAtom *)soundMap;
    current->sampleSize *= IMA4_BLOCK_SAMPLES;
    current->averageDuration *= IMA4_BLOCK_SAMPLES;
    current->totalSamples /= IMA4_BLOCK_SAMPLES;
  }

  /* Adjust sample-to-chunk entries, if necessary, and find max samples per chunk */
  ULONG maxSamplesPerChunk = 0;
  for (current->sampleToChunkCursor->setToFirst(); current->sampleToChunkCursor->isValid(); current->sampleToChunkCursor->setToNext())
  {
    SampleToChunk &sampleToChunk = current->sampleToChunkAtom->sampleToChunkTable.elementAt(*(current->sampleToChunkCursor));
    if (soundDescription.codecFourcc == FOURCC_IMA4)
      sampleToChunk.samplesPerChunk /= IMA4_BLOCK_SAMPLES;
    if (sampleToChunk.samplesPerChunk > maxSamplesPerChunk)
      maxSamplesPerChunk = sampleToChunk.samplesPerChunk;
  }
  current->sampleToChunkCursor->setToFirst();
  current->samplesPerChunk = current->sampleToChunkCursor->element().samplesPerChunk;
  current->sampleToChunkCursor->setToNext();

  fileBuffer = new CHAR[maxSamplesPerChunk * current->sampleSize];

  return (MMIO_SUCCESS);
}

/************************************************************
 * GET HEADER
 ************************************************************/
LONG SoundFile::getHeader(PVOID header, LONG headerLength)
{
  if (!header)
  {
    setError(MMIOERR_INVALID_STRUCTURE);
    return (MMIO_ERROR);
  }

  memcpy(header, (PVOID)&audioHeader, sizeof(MMAUDIOHEADER));

  return (sizeof(MMAUDIOHEADER));
}

/************************************************************
 * SET HEADER
 ************************************************************/
LONG SoundFile::setHeader(PVOID header, LONG headerLength)
{
  return (0);
}

/************************************************************
 * decompressAudio
 ************************************************************/
void SoundFile::decompressAudio(PBYTE buffer, ULONG bufferLength)
{
  FOURCC soundCodec = (FOURCC)audioHeader.mmXWAVHeader.XWAVHeaderInfo.pAdditionalInformation;

  /* ima4 compressed sound */
  if (soundCodec == FOURCC_IMA4)
  {
    ima4Expand((PUCHAR)buffer, audioHeader.mmXWAVHeader.WAVEHeader.usChannels,
               bufferLength / current->sampleSize);
    return;
  }

  /* Reverse byte order of raw 16-bit samples */
  if (soundCodec != FOURCC_IMA4 && audioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample == 16)
  {
    PUSHORT sample = (PUSHORT)buffer;
    while ((PBYTE)sample < buffer + bufferLength)
      reverseBytes(sample++);
    return;
  }

  /* Convert 8-bit twos-complement to linear */
  if (soundCodec == FOURCC_TWOS && audioHeader.mmXWAVHeader.WAVEHeader.usBitsPerSample == 8)
  {
    PBYTE sample = buffer;
    while (sample < buffer + bufferLength)
      *sample++ ^= 0x80;
    return;
  }
}

/************************************************************
 * READ
 ************************************************************/
LONG SoundFile::read(PSZ buffer, LONG bufferLength)
{
  ULONG bytesToRead;
  ULONG samplesToRead;

  /* Get records while more data in file,*/
  while (current->chunkCursor->isValid() && soundBuffer.length() < bufferLength)
  {
    /* Seek to better-known file position, if available */
    if (filePosition != current->newFilePosition)
      filePosition = mmioSeek(storageSystemFileHandle(), current->newFilePosition, SEEK_SET);

    /* Read rest of chunk (fixed sample size media) or one sample (if sound samples are ever variable?) */
    if (current->sampleSize)
    {
      samplesToRead = current->samplesPerChunk - current->sampleInChunk + 1;
      bytesToRead = samplesToRead * current->sampleSize;
    }
    else
    {
      samplesToRead = 1;
      bytesToRead = samplesToRead * current->sampleCursor->element();
    }

    /* Number of bytes to physically read from the file.  Smaller under IMA compression */
    ULONG ioBytes = (current == soundMap &&
                     (FOURCC)audioHeader.mmXWAVHeader.XWAVHeaderInfo.pAdditionalInformation == FOURCC_IMA4 ?
                     bytesToRead * audioHeader.mmXWAVHeader.WAVEHeader.usChannels *
                     IMA4_BLOCK_SIZE / current->sampleSize : bytesToRead);

    /* Read */
    try
    {
      filePosition += mmioRead(storageSystemFileHandle(), fileBuffer, ioBytes);
    }
    catch (NoMoreData noMoreData)
    {
      return 0;
    }

    /* Decompress or translate audio */
    if (current == soundMap)
      decompressAudio((PBYTE)fileBuffer, bytesToRead);

    soundBuffer += IString(fileBuffer, bytesToRead);

#ifdef DEBUGGING_IOPROC
  debugOutput << "Read: bytesToRead=" << bytesToRead << " ioBytes=" << ioBytes << " filePosition=" << filePosition << endl;
#endif

    /* Update position tracking variables */
    current->newFilePosition += ioBytes;
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
    } /* New chunk */
  } /* current->chunkCursor->isValid() */

  /* Give it up! */
  ULONG bytesToCopy = min(soundBuffer.length(), bufferLength);
  memcpy(buffer, (PCHAR)soundBuffer, bytesToCopy);
  soundBuffer.remove(1, bytesToCopy);

#ifdef DEBUGGING_IOPROC
  debugOutput << "Read: bytesReturned=" << bytesToCopy << endl;
#endif

  return(bytesToCopy);
}

/************************************************************
 * SEEK
 ************************************************************/
LONG SoundFile::seek(LONG seekPosition, LONG seekMode)
{
  float mediaSeekTime = seekPosition * current->averageDuration / current->sampleSize;

#ifdef DEBUGGING_IOPROC
  debugOutput << "Seek: mediaSeekTime=" << mediaSeekTime << endl;
#endif

  trackSeekByTime((SampleTableAtom *)soundMap, mediaSeekTime, seekMode);

  return seekPosition;
}

/************************************************************
 * WRITE
 ************************************************************/
LONG SoundFile::write(PSZ buffer, LONG bufferLength)
{
  return (0);
}

/************************************************************
 * trackSeekByTime - Time seek for a single track
 ************************************************************/
LONG SoundFile::trackSeekByTime(SampleTableAtom *seekMap, float mediaSeekTime, LONG seekMode)
{
  ULONG samplesToRead, bytesToRead; /* For shadow-read */
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
      newTime = seekMap->currentTime + mediaSeekTime;
      break;
    }
    case SEEK_END:
    {
      newTime = seekMap->totalDuration + mediaSeekTime;
      break;
    }
    default :
      return (MMIO_ERROR);
  }

  /* Rewind */
  seekMap->currentTime = .0;
  seekMap->chunkCursor->setToFirst();
  if (seekMap->chunkCursor->isValid())
    seekMap->newFilePosition = seekMap->chunkCursor->element();
  seekMap->sampleCursor->setToFirst();
  seekMap->sampleToChunkCursor->setToFirst();
  seekMap->samplesPerChunk = seekMap->sampleToChunkCursor->element().samplesPerChunk;
  seekMap->sampleToChunkCursor->setToNext();
  seekMap->sampleInChunk = 1;
  seekMap->sampleInMedia = 1;

  /* Shadow-read the movie until the desired time is reached */
  while (seekMap->chunkCursor->isValid() && (seekMap->currentTime < newTime))
  {
    if (seekMap->sampleSize) /* Fixed sample size media */
    {
      ULONG samplesLeftInTime = ceil(newTime / seekMap->averageDuration -
                                     seekMap->sampleInMedia + 1);
      ULONG samplesLeftInChunk = seekMap->samplesPerChunk - seekMap->sampleInChunk + 1;
      samplesToRead = min(samplesLeftInTime, samplesLeftInChunk);
      bytesToRead = samplesToRead * seekMap->sampleSize;
    }
    else /* Using sample size table for media */
    {
      samplesToRead = 1;
      bytesToRead = samplesToRead * seekMap->sampleCursor->element();
      seekMap->sampleCursor->setToNext();
    }

    ULONG ioBytes = (seekMap == soundMap &&
       (FOURCC)audioHeader.mmXWAVHeader.XWAVHeaderInfo.pAdditionalInformation == FOURCC_IMA4 ?
       bytesToRead * audioHeader.mmXWAVHeader.WAVEHeader.usChannels *
       IMA4_BLOCK_SIZE / seekMap->sampleSize : bytesToRead);

    /* Update position tracking variables */
    seekMap->newFilePosition += ioBytes;
    seekMap->currentTime += samplesToRead * seekMap->averageDuration;
    seekMap->sampleInMedia += samplesToRead;
    seekMap->sampleInChunk += samplesToRead;

    /* Update chunk cursors if we have hit the end of the chunk */
    if (seekMap->sampleInChunk > seekMap->samplesPerChunk)
    {
      seekMap->sampleInChunk = 1;
      seekMap->chunkCursor->setToNext();

      /* Update samples-per-chunk value, if necessary */
      if (seekMap->chunkCursor->isValid())
      {
        seekMap->newFilePosition = seekMap->chunkCursor->element();
        if (seekMap->sampleToChunkCursor->isValid() &&
            seekMap->chunkOffsetAtom->chunkOffsetTable.position(*(seekMap->chunkCursor)) ==
            seekMap->sampleToChunkCursor->element().firstChunk)
        {
          seekMap->samplesPerChunk = seekMap->sampleToChunkCursor->element().samplesPerChunk;
          seekMap->sampleToChunkCursor->setToNext();
        }
      }
    } /* New chunk */
  } /* Seeking */

  soundBuffer = IString();

  return (seekMap->currentTime * 3000.0 / seekMap->track->mediaAtom->mediaHeaderAtom.timeScale);
}

/************************************************************
 * SEEKBYTIME
 ************************************************************/
LONG SoundFile::seekByTime(LONG seekTime, LONG seekMode)
{
  float mediaSeekTime;
  LONG soundTime;
  LONG soundSeekMode = seekMode;

  if (seekTime < 0)
    return (MMIO_ERROR);

  if (soundMap)
  {
    mediaSeekTime = seekTime / 3000.0 * soundTrack->mediaAtom->mediaHeaderAtom.timeScale;
    soundTime = trackSeekByTime((SampleTableAtom *)soundMap, mediaSeekTime, soundSeekMode);
  }

  return (soundTime);
}

/************************************************************
 * CLOSE
 ************************************************************/
LONG SoundFile::close(USHORT closeFlags)
{
  delete fileBuffer;
  delete movieAtom;

  return (AudioFile::close(closeFlags));
}

SoundFile::~SoundFile()
{
}
