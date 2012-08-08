#include "moovproc.h"
#include "moovfile.hpp"
#include "moovfmt.hpp"

#include <ima4.h>
#include <pluginio.hpp>

#include <string.h>
#include <ctype.h>
#include <math.h>

#ifdef DEBUGGING_IOPROC
  #include <fstream.h>
  extern ofstream debugOutput;
#endif


/************************************************************
 * CONSTRUCTOR
 ************************************************************/
MOOVFile::MOOVFile()
        : videoTrack(0), soundTrack(0),
          videoMap(0), soundMap(0),
          current(0)
{
  movieAtom = 0;
  filePosition = totalDataBytes = nextBufferPosition = 0;
  autoInterleave = firstFrameDone = FALSE;
}

/************************************************************
 * IDENTIFY FILE - Will come here if an info block is passed
 ************************************************************/
LONG MOOVFile::identifyFile(PSZ fileName, HMMIO fileHandle)
{
  return MOOV().identifyFile(fileName, fileHandle);
}

/************************************************************
 * OPEN
 ************************************************************/
LONG MOOVFile::open(PSZ fileName)
{
  LONG returnCode;

  returnCode = MovieFile::open(fileName);
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
  * Initialize cursors; Find videotrack and soundtrack
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
          current->sampleToChunkAtom && current->timeToSampleAtom)
      {
        /* Initialize variables used in multiTrackRead */
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

        /* Set videotrack or soundtrack shortcuts to this track, if appropriate */
        if (mia->videoMediaInfoHeaderAtom &&
            (!videoTrack || videoTrack && current->totalDuration > videoMap->totalDuration))
        {
          current->mmioTrack = (videoMap ? videoMap->mmioTrack : nextTrack++);
          current->sampleSize = current->sampleSizeAtom->sampleSize;
          videoTrack = &trackCursor->element();
          videoMap = current;
        }
        if (mia->soundMediaInfoHeaderAtom &&
            (!soundTrack || soundTrack && current->totalDuration > soundMap->totalDuration))
        {
          ULONG soundCodec = mia->sampleTableAtom->sampleDescriptionAtom->sampleDescriptionTable.
                             elementAtPosition(1)->imageDescription.codecFourcc;
          if (soundCodec == FOURCC_RAW || soundCodec == FOURCC_TWOS ||
              soundCodec == FOURCC_BLANKS || soundCodec == FOURCC_IMA4 || !soundCodec)
          {
            current->mmioTrack = (soundMap ? soundMap->mmioTrack : nextTrack++);
            soundTrack = &trackCursor->element();
            soundMap = current;
          }
        } /* If sound */
      } /* If current */
    } /* If mia */
  } /* While track */
  delete trackCursor;

  if (!videoTrack && !soundTrack)
    return (MMIO_ERROR);

 /************************************************************
  * Set autoInterleave flag if tracks are laid down sequentially
  ************************************************************/
  autoInterleave = (videoTrack && soundTrack &&
    (videoMap->chunkOffsetAtom->chunkOffsetTable.lastElement() <
    soundMap->chunkOffsetAtom->chunkOffsetTable.firstElement() ||
    videoMap->chunkOffsetAtom->chunkOffsetTable.firstElement() >
    soundMap->chunkOffsetAtom->chunkOffsetTable.lastElement()));

 /************************************************************
  * Fill out the MMMOVIEHEADER structure.
  ************************************************************/
  movieHeader.ulStructLen = sizeof(MMMOVIEHEADER);
  movieHeader.ulContentType = MMIO_VIDEO_UNKNOWN;
  movieHeader.ulMediaType = MMIO_MEDIATYPE_MOVIE;
  movieHeader.ulMovieCapsFlags = MOVIE_CAN_SEEK
                               | MOVIE_CAN_SCAN;

  movieHeader.ulNumEntries = !!videoTrack + !!soundTrack;
  movieHeader.pmmTrackInfoList = trackInfoList;

  if (videoMap)
  {
    /* Video-specific movie header values */
    movieHeader.ulMovieCapsFlags |= MOVIE_HAS_VIDEO;
    trackInfoList[videoMap->mmioTrack].ulTrackID = videoMap->mmioTrack;
    trackInfoList[videoMap->mmioTrack].ulMediaType = MMIO_MEDIATYPE_DIGITALVIDEO;
    movieHeader.ulLength = videoMap->totalDuration * 3000.0 /
      videoTrack->mediaAtom->mediaHeaderAtom.timeScale;
    float floatSampleSize = videoMap->maxSampleSize;
    movieHeader.ulMaxBytesPerSec = floatSampleSize *
                                   videoTrack->mediaAtom->mediaHeaderAtom.timeScale /
                                   videoMap->averageDuration;
    movieHeader.ulSuggestedBufferSize = videoMap->maxSampleSize + 4095 >> 12 << 12;

   /************************************************************
    * Fill out the MMVIDEOHEADER structure.
    ************************************************************/
    videoHeader.ulStructLen = sizeof(MMVIDEOHEADER);
    videoHeader.ulContentType = MMIO_VIDEO_DATA;
    videoHeader.ulMediaType = MMIO_MEDIATYPE_DIGITALVIDEO;

    const ImageDescription& imageDescription = videoMap->sampleDescriptionAtom->
      sampleDescriptionTable.elementAtPosition(1)->imageDescription;
    videoHeader.ulWidth = imageDescription.width;
    videoHeader.ulHeight = imageDescription.height; // + 3 >> 2 << 2;

    videoHeader.ulScale = videoMap->averageDuration + 0.5;
    videoHeader.ulRate = videoTrack->mediaAtom->mediaHeaderAtom.timeScale;
    videoHeader.ulLength = movieHeader.ulLength;
    videoHeader.ulTotalFrames = videoMap->totalSamples;
    videoHeader.mmtimePerFrame = videoHeader.ulLength / videoHeader.ulTotalFrames;
    videoHeader.pmmXDIBHeader = &extendedBitmapHeader;
    videoHeader.ulSuggestedBufferSize = movieHeader.ulSuggestedBufferSize;

    BITMAPINFOHEADER2& bitmapHeader = extendedBitmapHeader.BMPInfoHeader2;
    bitmapHeader.cbFix = sizeof(BITMAPINFOHEADER2);
    bitmapHeader.cx = videoHeader.ulWidth;
    bitmapHeader.cy = videoHeader.ulHeight;
    bitmapHeader.cPlanes = 1;
    bitmapHeader.cBitCount = (imageDescription.depth == 32 ? 32 : imageDescription.depth & 0x1F);
    bitmapHeader.ulCompression = imageDescription.codecFourcc;

    /* Force default palette if bitcount < 16 and no palette given */
    SHORT colorTable = (bitmapHeader.cBitCount < 16 && imageDescription.colorTable == -1 ?
      bitmapHeader.cBitCount : imageDescription.colorTable);

    /* Set movie palette */
    if (colorTable == 0) /*Special movie palette stored in sample description */
    {
      videoHeader.genpalVideo.ulNumColors = imageDescription.firstColorEntry.blue + 1;
      videoHeader.genpalVideo.prgb2Entries = palette;
      for (USHORT paletteEntry = 0; paletteEntry < videoHeader.genpalVideo.ulNumColors; paletteEntry++)
      {
        palette[paletteEntry].bBlue = imageDescription.palette[paletteEntry].blue;
        palette[paletteEntry].bGreen = imageDescription.palette[paletteEntry].green;
        palette[paletteEntry].bRed = imageDescription.palette[paletteEntry].red;
        palette[paletteEntry].fcOptions = 0;
      }
    }
    else if (colorTable != -1) /* Specific palette */
    {
      if (getDefaultPalette(colorTable, palette, sizeof(RGB2) << bitmapHeader.cBitCount) == MMIO_SUCCESS)
      {
        videoHeader.genpalVideo.ulNumColors = 1 << bitmapHeader.cBitCount;
        videoHeader.genpalVideo.prgb2Entries = palette;
      }
    }

    /************************************************************ *
    * Open codec procedure
    ************************************************************/
    returnCode = openCodec();
    if (returnCode != MMIO_SUCCESS)
    {
      setError(MMIOERR_CODEC_NOT_SUPPORTED);
      return (MMIOERR_CODEC_NOT_SUPPORTED);
    }
  }
  else
    movieHeader.ulLength = soundMap->totalDuration * 3000.0 /
      soundTrack->mediaAtom->mediaHeaderAtom.timeScale;

  movieHeader.ulAvgBytesPerSec = totalDataBytes * 3000.0 / movieHeader.ulLength;

  if (soundMap)
  {
    /* Sound-specific movie header values */
    movieHeader.ulMovieCapsFlags |= MOVIE_HAS_AUDIO;
    trackInfoList[soundMap->mmioTrack].ulTrackID = soundMap->mmioTrack;
    trackInfoList[soundMap->mmioTrack].ulMediaType = MMIO_MEDIATYPE_AUDIO;

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

    movieHeader.ulMaxBytesPerSec += audioHeader.mmXWAVHeader.WAVEHeader.ulAvgBytesPerSec;

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
      for (current->sampleToChunkCursor->setToFirst(); current->sampleToChunkCursor->isValid();
           current->sampleToChunkCursor->setToNext())
        current->sampleToChunkAtom->sampleToChunkTable.elementAt(*(current->sampleToChunkCursor)).
          samplesPerChunk /= IMA4_BLOCK_SAMPLES;
      current->sampleToChunkCursor->setToFirst();
      current->samplesPerChunk = current->sampleToChunkCursor->element().samplesPerChunk;
      current->sampleToChunkCursor->setToNext();
    }
  }
  return (MMIO_SUCCESS);
}

/************************************************************
 * GET HEADER
 ************************************************************/
LONG MOOVFile::getHeader(PVOID header, LONG headerLength)
{
  if (!header)
  {
    setError(MMIOERR_INVALID_STRUCTURE);
    return (MMIO_ERROR);
  }

  if (track() == MMIO_RESETTRACKS)
  {
    memcpy(header, (PVOID)&movieHeader, sizeof(MMMOVIEHEADER));
    return (sizeof(MMMOVIEHEADER));
  }
  else if (videoMap && (track() == videoMap->mmioTrack))
  {
    memcpy(header, (PVOID)&videoHeader, sizeof(MMVIDEOHEADER));
    return (sizeof(MMVIDEOHEADER));
  }
  else if (soundMap && (track() == soundMap->mmioTrack))
  {
    memcpy(header, (PVOID)&audioHeader, sizeof(MMAUDIOHEADER));
    return (sizeof(MMAUDIOHEADER));
  }
  else
    return (MMIO_ERROR);
}

/************************************************************
 * SET HEADER
 ************************************************************/
LONG MOOVFile::setHeader(PVOID header, LONG headerLength)
{
  return (0);
}

/************************************************************
 * READ
 ************************************************************/
LONG MOOVFile::read(PSZ buffer, LONG bufferLength)
{
  return (0);
}

/************************************************************
 * setCurrentTrackMap - Read and Seek Helper Function
 ************************************************************/
SampleTableAtom *MOOVFile::currentTrackMap(LONG mmioTrack)
{
  if (videoMap && (mmioTrack == videoMap->mmioTrack))
    return ((SampleTableAtom *)videoMap);

  if (soundMap && (mmioTrack == soundMap->mmioTrack))
    return ((SampleTableAtom *)soundMap);

  if (videoMap && (!soundMap || soundMap && !soundMap->chunkCursor->isValid()))
    return ((SampleTableAtom *)videoMap);

  if (soundMap && (!videoMap || videoMap && !videoMap->chunkCursor->isValid()))
    return ((SampleTableAtom *)soundMap);

  if (autoInterleave)
    return (SampleTableAtom *)
            (videoMap->currentTime / videoTrack->mediaAtom->mediaHeaderAtom.timeScale <
             soundMap->currentTime / soundTrack->mediaAtom->mediaHeaderAtom.timeScale ?
             videoMap : soundMap);
  else
    return (SampleTableAtom *)
            (videoMap->chunkCursor->element() <
             soundMap->chunkCursor->element() ?
             videoMap : soundMap); /* Should this use newFilePosition's now? */
}

/************************************************************
 * positionToCurrentSample
 ************************************************************/
void MOOVFile::positionToCurrentSample()
{
  if (current->chunkCursor->isValid() && (current->sampleSize || current->sampleCursor->isValid()))
  {
    current->newFilePosition = current->chunkCursor->element();
    if (current->sampleSize)
      current->newFilePosition += current->sampleSize * (current->sampleInChunk - 1);
    else
    {
      ITabularSequence<ULONG>::Cursor tempSampleCursor = *(current->sampleCursor);
      for (int i = current->sampleInChunk - 1; i > 0; i--)
      {
        tempSampleCursor.setToPrevious();
        current->newFilePosition += tempSampleCursor.element();
      }
    }
  }
}

/************************************************************
 * decompressAudio
 ************************************************************/
void MOOVFile::decompressAudio(PBYTE buffer, ULONG bufferLength)
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
 * firstPluginFrame
 ************************************************************/
BOOL MOOVFile::firstPluginFrame()
{
  if (videoMap && videoMap->sampleInMedia > 1)
  {
    firstFrameDone = TRUE;
    return (childIOProc() == mmioFOURCC('N', 'S', 'C', 'P'));
  }
  return FALSE;
}

/************************************************************
 * MULTITRACKREAD
 ************************************************************/
LONG MOOVFile::multiTrackRead(PMMMULTITRACKREAD multiTrackInfo)
{
  ULONG track = MMIO_RESETTRACKS;
  LONG bufferPosition = nextBufferPosition;
  ULONG bytesToRead;
  ULONG samplesToRead;
  BOOL failedRead = FALSE;

#ifdef DEBUGGING_IOPROC
  debugOutput <<  multiTrackInfo->ulNumTracks << '\t' << multiTrackInfo->ulLength << '\t' << multiTrackInfo->pBuffer << endl;
#endif

  /* Reset next buffer position.  Used for not-done and/or buffer-too-small */
  nextBufferPosition = 0;

  /* Store maximum entries and reset record counts to 0.  We will pass back actual # of entries */
  for (ULONG i = 0; i < multiTrackInfo->ulNumTracks; i++)
  {
    if (videoMap && multiTrackInfo->pTrackMapList[i].ulTrackID == videoMap->mmioTrack)
    {
      ((SampleTableAtom *)videoMap)->maxEntries = multiTrackInfo->pTrackMapList[i].ulNumEntries;
      ((SampleTableAtom *)videoMap)->mtReadIndex = i;
    }
    else if (soundMap && multiTrackInfo->pTrackMapList[i].ulTrackID == soundMap->mmioTrack)
    {
      ((SampleTableAtom *)soundMap)->maxEntries = multiTrackInfo->pTrackMapList[i].ulNumEntries;
      ((SampleTableAtom *)soundMap)->mtReadIndex = i;
    }
    multiTrackInfo->pTrackMapList[i].ulNumEntries = 0;
  }

  /* Set specific track, if requested */
  if (multiTrackInfo->ulNumTracks == 1)
    track = multiTrackInfo->pTrackMapList[0].ulTrackID;

  /* Set track requested, or that of the first chunk. */
  if (filePosition == 0 || track != MMIO_RESETTRACKS)
    current = currentTrackMap(track);

  /* If reverse scanning and pointer is at end, set to last frame */
  if ((extendedFlags() & MMIO_REVERSE_READ) && !current->chunkCursor->isValid())
  {
    current->chunkCursor->setToLast();
    current->sampleCursor->setToLast();
    current->sampleInChunk = current->samplesPerChunk;
    current->sampleInMedia -= 1;
    current->currentTime -= current->averageDuration;
    positionToCurrentSample();
  }

  /* Get records while more data in file,*/
  while (current->chunkCursor->isValid())
  {
    if (current->sampleSize) /* Fixed sample size media */
    {
      LONG samplesLeftInMedia = ((LONG)multiTrackInfo->ulLength - bufferPosition) / (LONG)current->sampleSize;
      if (samplesLeftInMedia < 0) /* Could be negative if an overread has been done */
        samplesToRead = 0;
      else
      {
        /* With a new buffer, read the frame even if it doesn't fit, if it fits in ulBufferLength */
        if (!samplesLeftInMedia && !bufferPosition && current->sampleSize <= multiTrackInfo->ulBufferLength)
          samplesLeftInMedia = 1;
        ULONG samplesLeftInChunk = current->samplesPerChunk - current->sampleInChunk + 1;
        samplesToRead = min(samplesLeftInMedia, samplesLeftInChunk);
        if (current == videoMap && samplesToRead)
          samplesToRead = 1;
      }
      bytesToRead = samplesToRead * current->sampleSize;
    }
    else /* Using sample size table for media */
    {
      samplesToRead = (bufferPosition + current->sampleCursor->element() <= multiTrackInfo->ulLength ||
                       !bufferPosition && current->sampleCursor->element() <= multiTrackInfo->ulBufferLength);
      bytesToRead = samplesToRead * current->sampleCursor->element();
    }

    if (!bytesToRead)
      break;

    if (multiTrackInfo->pTrackMapList[current->mtReadIndex].ulNumEntries < current->maxEntries)
    {
      /* Number of bytes to physically read from the file.  Smaller under IMA compression */
      ULONG ioBytes = (current == soundMap &&
        (FOURCC)audioHeader.mmXWAVHeader.XWAVHeaderInfo.pAdditionalInformation == FOURCC_IMA4 ?
        bytesToRead * audioHeader.mmXWAVHeader.WAVEHeader.usChannels *
         IMA4_BLOCK_SIZE / current->sampleSize : bytesToRead);

      /* If scanning and this is not a key frame, don't actually read and return this record */
      if (!(extendedFlags() & (MMIO_SCAN_READ | MMIO_REVERSE_READ)) || !current->syncSampleAtom ||
          current->syncSampleAtom->syncSampleTable.containsElementWithKey(current->sampleInMedia))
      {
        /* Seek to better-known file position, if available */
        if (filePosition != current->newFilePosition)
          filePosition = mmioSeek(storageSystemFileHandle(), current->newFilePosition, SEEK_SET);

        /* Read */
        try
        {
          filePosition += mmioRead(storageSystemFileHandle(),
                                   (PCHAR)multiTrackInfo->pBuffer + bufferPosition, ioBytes);
        }
        catch (NoMoreData noMoreData)
        {
          failedRead = TRUE;
          break;
        }

        /* Enter information into record table */
        PRECORDTAB recordTable = &(multiTrackInfo->pTrackMapList[current->mtReadIndex].
          pRecordTabList[multiTrackInfo->pTrackMapList[current->mtReadIndex].ulNumEntries++]);
        recordTable->pRecord = (PCHAR)multiTrackInfo->pBuffer + bufferPosition;
        recordTable->ulLength = bytesToRead;
        recordTable->ulParm2 = current->sampleInMedia;
        bufferPosition += bytesToRead;

        /* Decompress or translate audio */
        if (current == soundMap)
          decompressAudio((PBYTE)recordTable->pRecord, bytesToRead);

#ifdef DEBUGGING_IOPROC
        debugOutput << '\t' << current->mmioTrack << '\t' << current->sampleInMedia << '\t' <<
                       bytesToRead << '\t' << filePosition << '\t' << recordTable->pRecord << endl;
#endif
      }

      /* Reverse scan */
      if (extendedFlags() & MMIO_REVERSE_READ)
      {
        if (current->sampleInMedia == 1)
          break;

        /* Update position tracking variables */
        current->currentTime -= samplesToRead * current->averageDuration;
        current->sampleInMedia -= samplesToRead;
        current->sampleInChunk -= samplesToRead;
        if (!current->sampleSize)
          current->sampleCursor->setToPrevious();

        /* Update chunk cursors if we have hit the beginning of the chunk */
        if (current->sampleInChunk < 1)
        {
          current->chunkCursor->setToPrevious();

          /* Update samples-per-chunk value, if necessary */
          ITabularSequence<SampleToChunk>::Cursor tempSCCursor = *(current->sampleToChunkCursor);
          if (tempSCCursor.isValid())
            tempSCCursor.setToPrevious();
          else
            tempSCCursor.setToLast();
          if (current->chunkOffsetAtom->chunkOffsetTable.position(*(current->chunkCursor)) <
              tempSCCursor.element().firstChunk)
          {
            tempSCCursor.setToPrevious();
            current->samplesPerChunk = tempSCCursor.element().samplesPerChunk;
            if (current->sampleToChunkCursor->isValid())
              current->sampleToChunkCursor->setToPrevious();
            else
              current->sampleToChunkCursor->setToLast();
          }
          current->sampleInChunk = current->samplesPerChunk;
        } /* New chunk */
        positionToCurrentSample();
      }
      else /* Normal read */
      {
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
          current = currentTrackMap(track);
        } /* New chunk */
      } /* Not reverse read */
    } /* Room in recordtab structure */
    else
    {
      multiTrackInfo->ulFlags |= MULTITRACKREAD_NOTDONE;
      nextBufferPosition = bufferPosition;
      break;
    }
  } /* current->chunkCursor->isValid() */

  if (!firstFrameDone && firstPluginFrame())
    failedRead = TRUE;

  if (failedRead ||
      !current->chunkCursor->isValid() ||
      current->currentTime / current->track->mediaAtom->mediaHeaderAtom.timeScale >= movieHeader.ulStructLen >> 2 ||
      (extendedFlags() & MMIO_REVERSE_READ) && current->sampleInMedia == 1)
    multiTrackInfo->ulFlags |= MULTITRACKREAD_EOF;

  if (!nextBufferPosition)
    nextBufferPosition = (multiTrackInfo->ulLength > bufferPosition ? 0 :
                          bufferPosition - multiTrackInfo->ulLength);

  if (!failedRead)
    QUICKMO(this, multiTrackInfo);

  return(MMIO_SUCCESS);
}
/************************************************************
 * WRITE
 ************************************************************/
LONG MOOVFile::write(PSZ buffer, LONG bufferLength)
{
  return (0);
}

/************************************************************
 * trackSeekByTime - Time seek for a single track
 ************************************************************/
LONG MOOVFile::trackSeekByTime(SampleTableAtom *seekMap, float mediaSeekTime, LONG seekMode)
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

  /* Adjust seek time to that of the previous i-frame, if requested */
  if ((seekMode & MMIO_SEEK_IFRAME) && seekMap->syncSampleAtom)
  {
    IKeySortedSet<ULONG, ULONG>::Cursor *syncSampleCursor = seekMap->syncSampleAtom->syncSampleTable.newCursor();
    for (syncSampleCursor->setToLast(); syncSampleCursor->isValid() && (syncSampleCursor->element() - 1) *
         seekMap->averageDuration > newTime; syncSampleCursor->setToPrevious());
    if (syncSampleCursor->isValid())
      newTime = (syncSampleCursor->element() - 1) * seekMap->averageDuration;
    delete syncSampleCursor;
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

  return (seekMap->currentTime * 3000.0 / seekMap->track->mediaAtom->mediaHeaderAtom.timeScale);
}

/************************************************************
 * SEEKBYTIME
 ************************************************************/
LONG MOOVFile::seekByTime(LONG seekTime, LONG seekMode)
{
  float mediaSeekTime;
  LONG videoTime, soundTime;
  LONG soundSeekMode = seekMode;

  if (seekTime < 0)
    return (MMIO_ERROR);

  /* If there is a video track, seek to nearest frame >= the i-frame <= the requested time */
  if (videoMap)
  {
    mediaSeekTime = seekTime / 3000.0 * videoTrack->mediaAtom->mediaHeaderAtom.timeScale;
    videoTime = trackSeekByTime((SampleTableAtom *)videoMap, mediaSeekTime, seekMode | MMIO_SEEK_IFRAME);
    soundSeekMode = SEEK_SET;
  }
  else
    videoTime = seekTime;

  /* Seek sound to match video */
  if (soundMap)
  {
    mediaSeekTime = videoTime / 3000.0 * soundTrack->mediaAtom->mediaHeaderAtom.timeScale;
    soundTime = trackSeekByTime((SampleTableAtom *)soundMap, mediaSeekTime, soundSeekMode);
  }

  /* Set current track map */
  if (soundMap && soundMap->chunkCursor->isValid() &&
      soundMap->newFilePosition < videoMap->newFilePosition)
    current = (SampleTableAtom *)soundMap;
  else
    current = (SampleTableAtom *)videoMap;

  if (videoMap)
    return (videoTime);
  else
    return (soundTime);
}

/************************************************************
 * CLOSE
 ************************************************************/
LONG MOOVFile::close(USHORT closeFlags)
{
  return (MovieFile::close(closeFlags));
}

MOOVFile::~MOOVFile()
{
  delete movieAtom;
}
