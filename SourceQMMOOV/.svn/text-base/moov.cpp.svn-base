#include "moov.hpp"
#include <stdlib.h>
#include <stddef.h>
#include <math.h>

#include <pluginio.hpp>

HMODULE moduleHandle;

ULONG const& key(ULONG const& element) {return element;};

/**************************************************************************
 *   reverseBytes - utility function to reverse byte order in a ULONG    **
 **************************************************************************/
void reverseBytes(PULONG inULong)
{
  struct FourChars
  {
    char char1;
    char char2;
    char char3;
    char char4;
  };
  char tempChar;

  FourChars* fourChars = (FourChars*)inULong;
  tempChar = fourChars->char1;
  fourChars->char1 = fourChars->char4;
  fourChars->char4 = tempChar;
  tempChar = fourChars->char2;
  fourChars->char2 = fourChars->char3;
  fourChars->char3 = tempChar;
}

/**************************************************************************
 *   reverseBytes - utility function to reverse byte order in a USHORT   **
 **************************************************************************/
void reverseBytes(PUSHORT inUShort)
{
  struct TwoChars
  {
    char char1;
    char char2;
  };
  char tempChar;

  TwoChars* twoChars = (TwoChars*)inUShort;
  tempChar = twoChars->char1;
  twoChars->char1 = twoChars->char2;
  twoChars->char2 = tempChar;
}

/**************************************************************************
 *   fixedToFloat - utility function to convert a 16.16 Fixed to a float **
 **************************************************************************/
void fixedToFloat(float* inFloat)
{
  LONG aLong;
  ((PBYTE)&aLong)[0] = ((PBYTE)inFloat)[3];
  ((PBYTE)&aLong)[1] = ((PBYTE)inFloat)[2];
  ((PBYTE)&aLong)[2] = ((PBYTE)inFloat)[1];
  ((PBYTE)&aLong)[3] = ((PBYTE)inFloat)[0];
  *inFloat = aLong / 65536.0;
}

/************************************************************************/
/* exitFunction                                                         */
/************************************************************************/
static void _System exitFunction(ULONG reason)
{
  __ctordtorTerm();
  DosExitList(EXLST_EXIT, exitFunction);
}


/************************************************************************/
/* _DLL_InitTerm:                                                       */
/************************************************************************/
ULONG _System _DLL_InitTerm (ULONG hModHandle, ULONG fTerm)
{
  moduleHandle = hModHandle;

  if (!fTerm)
  {
    DosExitList(0x0000FF00 | EXLST_ADD, exitFunction);
    __ctordtorInit();
  }

  return (1L);  /* Success */
}

/**************************************************************************
 *   getDefaultPalette - Attempt to load palette resource                **
 **************************************************************************/
LONG getDefaultPalette(SHORT resourceID, PRGB2 palette, ULONG paletteLength)
{
  PVOID resourceData;
  LONG returnCode;

  returnCode = DosGetResource(moduleHandle, RT_RCDATA, resourceID, &resourceData);
  if (!returnCode)
  {
    memcpy(palette, resourceData, paletteLength);
    DosFreeResource(resourceData);
    return (MMIO_SUCCESS);
  }
  else
    return (MMIO_ERROR);
}

/**************************************************************************
 * QUICKTIME HEADER DATA STRUCTURES                                      **
 **************************************************************************/

/*
 * ContainerAtom
 */

ContainerAtom::ContainerAtom()
{
  memcpy(&atomTag_p, "    ", sizeof(AtomTag));
}

ContainerAtom::ContainerAtom(AtomTag& atomTag)
{
  memcpy(&atomTag_p, &atomTag, sizeof(atomTag));
}

LONG ContainerAtom::load(HMMIO fileHandle)
{
  ULONG bytesRead = 0;
  AtomTag atomTag;

  // Read in a subatom and process it using the overridden loadAtom routine for this container atom
  while (bytesRead + sizeof(atomTag_p) + sizeof(atomTag) < atomTag_p.size)
  {
    bytesRead += mmioRead(fileHandle, (PCHAR)&atomTag, sizeof(AtomTag));
    reverseBytes(&atomTag.size);
    if (loadAtom(fileHandle, bytesRead, atomTag) != MMIO_SUCCESS) // Skip tag if unrecognized
    {
      mmioSeek(fileHandle, atomTag.size - sizeof(AtomTag), SEEK_CUR);
      bytesRead += atomTag.size - sizeof(AtomTag);
    }
  }
  // Position to end of atom if not already there
  if (bytesRead != atomTag_p.size - sizeof(atomTag_p))
    mmioSeek(fileHandle, atomTag_p.size - sizeof(atomTag_p) - bytesRead, SEEK_CUR);

  return (MMIO_SUCCESS);
}

/*
 * UserDataAtom
 */

UserDataAtom::UserDataAtom(AtomTag& atomTag)
            : ContainerAtom(atomTag)
{
  navigationAtom = 0;
  controllerTypeAtom = 0;
}

LONG UserDataAtom::loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag)
{
  if      (!memcmp(&atomTag.type, "NAVG", ATOM_TYPE_SIZE))
  {
    navigationAtom = new NavigationAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)navigationAtom, sizeof(NavigationAtom));
    reverseBytes(&navigationAtom->version);
    reverseBytes(&navigationAtom->numberOfColumns);
    reverseBytes(&navigationAtom->numberOfRows);
    reverseBytes(&navigationAtom->loopSize);
    reverseBytes(&navigationAtom->frameDuration);
    reverseBytes(&navigationAtom->movieType);
    reverseBytes(&navigationAtom->loopTicks);
    fixedToFloat(&navigationAtom->fieldOfView);
    fixedToFloat(&navigationAtom->startHPan);
    fixedToFloat(&navigationAtom->endHPan);
    fixedToFloat(&navigationAtom->startVPan);
    fixedToFloat(&navigationAtom->endVPan);
    fixedToFloat(&navigationAtom->initialHPan);
    fixedToFloat(&navigationAtom->initialVPan);
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "ctyp", ATOM_TYPE_SIZE))
  {
    controllerTypeAtom = new ControllerTypeAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)controllerTypeAtom, sizeof(ControllerTypeAtom));
    return(MMIO_SUCCESS);
  }
  else
    return (MMIO_ERROR);
}

UserDataAtom::~UserDataAtom()
{
  delete navigationAtom;
  delete controllerTypeAtom;
}

ostream& operator<<(ostream& outStream, const UserDataAtom& userDataAtom)
{
  outStream << "User Data [udat]\n";
  if (userDataAtom.navigationAtom)
    outStream << *userDataAtom.navigationAtom;
  if (userDataAtom.controllerTypeAtom)
    outStream << *userDataAtom.controllerTypeAtom;
  return (outStream);
}

/*
 * SampleTableAtom
 */

SampleTableAtom::SampleTableAtom(AtomTag& atomTag)
               : ContainerAtom(atomTag)
{
  sampleDescriptionAtom = 0;
  timeToSampleAtom = 0;
  syncSampleAtom = 0;
  sampleToChunkAtom = 0;
  sampleSizeAtom = 0;
  chunkOffsetAtom = 0;

  totalSamples = totalDuration = maxSampleSize = 0;
  averageDuration = .0;
  mediaToTicks = 0;

  chunkCursor = 0;
  sampleCursor = 0;
  sampleToChunkCursor = 0;
  mmioTrack = -1;
  sampleSize = 0;
}

LONG SampleTableAtom::loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag)
{
  if      (!memcmp(&atomTag.type, "stsd", ATOM_TYPE_SIZE))
  {
    sampleDescriptionAtom = new SampleDescriptionAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)sampleDescriptionAtom,
                          sizeof(SampleDescriptionAtom) -
                          sizeof(SampleDescriptionAtom::sampleDescriptionTable));
    reverseBytes(&sampleDescriptionAtom->numberOfEntries);
    for (int i = sampleDescriptionAtom->numberOfEntries; i > 0; i--)
    {
      ULONG sampleDescriptionLength;
      bytesRead += mmioRead(fileHandle, (PCHAR)&sampleDescriptionLength, sizeof(ULONG));
      reverseBytes(&sampleDescriptionLength);
      char *sampleDescriptionBuffer = new char[sampleDescriptionLength - sizeof(ULONG)];
      try
      {
        bytesRead += mmioRead(fileHandle, sampleDescriptionBuffer, sampleDescriptionLength - sizeof(ULONG));
      }
      catch (NoMoreData noMoreData)
      {
        delete sampleDescriptionBuffer;
        throw;
      }
      SampleDescription* sampleDescription = (SampleDescription*)sampleDescriptionBuffer;
      if (!memcmp(&sampleDescription->imageDescription.codecFourcc, "musi", 4))
      {
        ULONG numberOfParts = (sampleDescriptionLength - 24) / 92;
        sampleDescription->musicDescription.reserved1 = numberOfParts; /* Tuck this away */
        for (int j = 0; j < numberOfParts; j++)
          reverseBytes(&sampleDescription->musicDescription.noteRequestEvent[j].noteRequest.
            toneDescription.gmNumber);
      } else
      if (!memcmp(&sampleDescription->imageDescription.codecFourcc, "pano", 4))
      {
        reverseBytes(&sampleDescription->panoramaDescription.sceneTrackID);
        reverseBytes(&sampleDescription->panoramaDescription.hotSpotTrackID);
        fixedToFloat(&sampleDescription->panoramaDescription.hPanStart);
        fixedToFloat(&sampleDescription->panoramaDescription.hPanEnd);
        fixedToFloat(&sampleDescription->panoramaDescription.vPanTop);
        fixedToFloat(&sampleDescription->panoramaDescription.vPanBottom);
        fixedToFloat(&sampleDescription->panoramaDescription.minimumZoom);
        fixedToFloat(&sampleDescription->panoramaDescription.maximumZoom);
        reverseBytes(&sampleDescription->panoramaDescription.sceneSizeX);
        reverseBytes(&sampleDescription->panoramaDescription.sceneSizeY);
        reverseBytes(&sampleDescription->panoramaDescription.numFrames);
        reverseBytes(&sampleDescription->panoramaDescription.sceneNumFramesX);
        reverseBytes(&sampleDescription->panoramaDescription.sceneNumFramesY);
        reverseBytes(&sampleDescription->panoramaDescription.sceneColorDepth);
        reverseBytes(&sampleDescription->panoramaDescription.hotSpotSizeX);
        reverseBytes(&sampleDescription->panoramaDescription.hotSpotSizeY);
        reverseBytes(&sampleDescription->panoramaDescription.hotSpotNumFramesX);
        reverseBytes(&sampleDescription->panoramaDescription.hotSpotNumFramesY);
        reverseBytes(&sampleDescription->panoramaDescription.hotSpotColorDepth);
      } else
      {
        if (sampleDescriptionLength > offsetof(SampleDescription, soundDescription.temporalQuality))
          reverseBytes(&sampleDescription->soundDescription.temporalQuality);
        if (sampleDescriptionLength > offsetof(SampleDescription, imageDescription.width))
          reverseBytes(&sampleDescription->imageDescription.width);
        if (sampleDescriptionLength > offsetof(SampleDescription, imageDescription.height))
          reverseBytes(&sampleDescription->imageDescription.height);
        if (sampleDescriptionLength > offsetof(SampleDescription, imageDescription.depth))
          reverseBytes(&sampleDescription->imageDescription.depth);
        if (sampleDescriptionLength > offsetof(SampleDescription, imageDescription.colorTable))
          reverseBytes((PUSHORT)&sampleDescription->imageDescription.colorTable);
        if (sampleDescriptionLength > offsetof(SampleDescription, imageDescription.firstColorEntry.blue))
          reverseBytes(&sampleDescription->imageDescription.firstColorEntry.blue);
      }
      sampleDescriptionAtom->sampleDescriptionTable.add(sampleDescription);
    }
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "stts", ATOM_TYPE_SIZE))
  {
    timeToSampleAtom = new TimeToSampleAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)timeToSampleAtom,
                          sizeof(TimeToSampleAtom) -
                          sizeof(TimeToSampleAtom::timeToSampleTable));
    reverseBytes(&timeToSampleAtom->numberOfEntries);
    for (int i = timeToSampleAtom->numberOfEntries; i > 0; i--)
    {
      TimeToSample timeToSample;
      bytesRead += mmioRead(fileHandle, (PCHAR)&timeToSample,
                            sizeof(TimeToSample));
      reverseBytes(&timeToSample.count);
      reverseBytes(&timeToSample.duration);
      timeToSampleAtom->timeToSampleTable.add(timeToSample);
      totalSamples += timeToSample.count;
      totalDuration += timeToSample.count * timeToSample.duration;
    }
    if (totalSamples)
    {
      float floatTotalDuration = totalDuration;
      averageDuration = floatTotalDuration / totalSamples;
    }
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "stss", ATOM_TYPE_SIZE))
  {
    syncSampleAtom = new SyncSampleAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)syncSampleAtom,
                          sizeof(SyncSampleAtom) -
                          sizeof(SyncSampleAtom::syncSampleTable));
    reverseBytes(&syncSampleAtom->numberOfEntries);
    for (int i = syncSampleAtom->numberOfEntries; i > 0; i--)
    {
      ULONG syncSample;
      bytesRead += mmioRead(fileHandle, (PCHAR)&syncSample, sizeof(ULONG));
      reverseBytes(&syncSample);
      syncSampleAtom->syncSampleTable.add(syncSample);
    }
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "stsc", ATOM_TYPE_SIZE))
  {
    sampleToChunkAtom = new SampleToChunkAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)sampleToChunkAtom,
                          sizeof(SampleToChunkAtom) -
                          sizeof(SampleToChunkAtom::sampleToChunkTable));
    reverseBytes(&sampleToChunkAtom->numberOfEntries);
    for (int i = sampleToChunkAtom->numberOfEntries; i > 0; i--)
    {
      SampleToChunk sampleToChunk;
      bytesRead += mmioRead(fileHandle, (PCHAR)&sampleToChunk,
                            sizeof(SampleToChunk));
      reverseBytes(&sampleToChunk.firstChunk);
      reverseBytes(&sampleToChunk.samplesPerChunk);
      reverseBytes(&sampleToChunk.sampleDescription);
      sampleToChunkAtom->sampleToChunkTable.add(sampleToChunk);
    }
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "stsz", ATOM_TYPE_SIZE))
  {
    sampleSizeAtom = new SampleSizeAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)sampleSizeAtom,
                          sizeof(SampleSizeAtom) -
                          sizeof(SampleSizeAtom::sampleSizeTable));
    reverseBytes(&sampleSizeAtom->sampleSize);
    maxSampleSize = sampleSizeAtom->sampleSize;
    if (!sampleSizeAtom->sampleSize)
    {
      reverseBytes(&sampleSizeAtom->numberOfEntries);
      for (int i = sampleSizeAtom->numberOfEntries; i > 0; i--)
      {
        ULONG sampleSize;
        bytesRead += mmioRead(fileHandle, (PCHAR)&sampleSize, sizeof(ULONG));
        reverseBytes(&sampleSize);
        sampleSizeAtom->sampleSizeTable.add(sampleSize);
        if (sampleSize > maxSampleSize)
          maxSampleSize = sampleSize;
      }
    }
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "stco", ATOM_TYPE_SIZE))
  {
    chunkOffsetAtom = new ChunkOffsetAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)chunkOffsetAtom,
                          sizeof(ChunkOffsetAtom) -
                          sizeof(ChunkOffsetAtom::chunkOffsetTable));
    reverseBytes(&chunkOffsetAtom->numberOfEntries);
    for (int i = chunkOffsetAtom->numberOfEntries; i > 0; i--)
    {
      ULONG chunkOffset;
      bytesRead += mmioRead(fileHandle, (PCHAR)&chunkOffset, sizeof(ULONG));
      reverseBytes(&chunkOffset);
      chunkOffsetAtom->chunkOffsetTable.add(chunkOffset);
    }
    return(MMIO_SUCCESS);
  }
  else
    return(MMIO_ERROR);
}

/************************************************************
 * trackSeekByTime - Time seek for a single track
 ************************************************************/
void SampleTableAtom::seekByTime(float mediaSeekTime, LONG seekMode)
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
      newTime = currentTime + mediaSeekTime;
      break;
    }
    case SEEK_END:
    {
      newTime = totalDuration + mediaSeekTime;
      break;
    }
    default :
      return;
  }

  /* Adjust seek time to that of the previous i-frame, if requested */
  if ((seekMode & MMIO_SEEK_IFRAME) && syncSampleAtom)
  {
    IKeySortedSet<ULONG, ULONG>::Cursor *syncSampleCursor = syncSampleAtom->syncSampleTable.newCursor();
    for (syncSampleCursor->setToLast(); syncSampleCursor->isValid() && (syncSampleCursor->element() - 1) *
         averageDuration > newTime; syncSampleCursor->setToPrevious());
    if (syncSampleCursor->isValid())
      newTime = (syncSampleCursor->element() - 1) * averageDuration;
    delete syncSampleCursor;
  }

  /* Rewind */
  currentTime = .0;
  chunkCursor->setToFirst();
  if (chunkCursor->isValid())
    newFilePosition = chunkCursor->element();
  sampleCursor->setToFirst();
  sampleToChunkCursor->setToFirst();
  samplesPerChunk = sampleToChunkCursor->element().samplesPerChunk;
  sampleToChunkCursor->setToNext();
  sampleInChunk = 1;
  sampleInMedia = 1;

  /* Shadow-read the movie until the desired time is reached */
  while (chunkCursor->isValid() && (currentTime < newTime))
  {
    if (sampleSize) /* Fixed sample size media */
    {
      ULONG samplesLeftInTime = ceil(newTime / averageDuration -
                                     sampleInMedia + 1);
      ULONG samplesLeftInChunk = samplesPerChunk - sampleInChunk + 1;
      samplesToRead = min(samplesLeftInTime, samplesLeftInChunk);
      bytesToRead = samplesToRead * sampleSize;
    }
    else /* Using sample size table for media */
    {
      samplesToRead = 1;
      bytesToRead = samplesToRead * sampleCursor->element();
      sampleCursor->setToNext();
    }

    /* Update position tracking variables */
    newFilePosition += bytesToRead;
    currentTime += samplesToRead * averageDuration;
    sampleInMedia += samplesToRead;
    sampleInChunk += samplesToRead;

    /* Update chunk cursors if we have hit the end of the chunk */
    if (sampleInChunk > samplesPerChunk)
    {
      sampleInChunk = 1;
      chunkCursor->setToNext();

      /* Update samples-per-chunk value, if necessary */
      if (chunkCursor->isValid())
      {
        newFilePosition = chunkCursor->element();
        if (sampleToChunkCursor->isValid() &&
            chunkOffsetAtom->chunkOffsetTable.position(*(chunkCursor)) ==
            sampleToChunkCursor->element().firstChunk)
        {
          samplesPerChunk = sampleToChunkCursor->element().samplesPerChunk;
          sampleToChunkCursor->setToNext();
        }
      }
    } /* New chunk */
  } /* Seeking */
}

SampleTableAtom::~SampleTableAtom()
{
  if (sampleDescriptionAtom)
  {
    ITabularSequence<SampleDescription*>::Cursor* sampleDescriptionCursor =
      sampleDescriptionAtom->sampleDescriptionTable.newCursor();
    for (sampleDescriptionCursor->setToFirst();
         sampleDescriptionCursor->isValid();
         sampleDescriptionCursor->setToNext())
      delete[] (char*)(sampleDescriptionCursor->element());
    delete sampleDescriptionCursor;
  }
  delete sampleDescriptionAtom;
  delete timeToSampleAtom;
  delete syncSampleAtom;
  delete sampleToChunkAtom;
  delete sampleSizeAtom;
  delete chunkOffsetAtom;
  delete chunkCursor;
  delete sampleCursor;
  delete sampleToChunkCursor;
}

ostream& operator<<(ostream& outStream, const SampleTableAtom& sampleTableAtom)
{
  outStream << "Sample Table [stbl]\n";
  if (sampleTableAtom.sampleDescriptionAtom)
    outStream << *sampleTableAtom.sampleDescriptionAtom;
  if (sampleTableAtom.timeToSampleAtom)
    outStream << *sampleTableAtom.timeToSampleAtom;
  if (sampleTableAtom.syncSampleAtom)
    outStream << *sampleTableAtom.syncSampleAtom;
  if (sampleTableAtom.sampleToChunkAtom)
    outStream << *sampleTableAtom.sampleToChunkAtom;
  if (sampleTableAtom.sampleSizeAtom)
    outStream << *sampleTableAtom.sampleSizeAtom;
  if (sampleTableAtom.chunkOffsetAtom)
    outStream << *sampleTableAtom.chunkOffsetAtom;
  return (outStream);
}

/*
 * BaseMediaInfoHeaderAtom
 */

BaseMediaInfoHeaderAtom::BaseMediaInfoHeaderAtom(AtomTag& atomTag)
                       : ContainerAtom(atomTag)
{
  panoramaMediaInfoAtom = 0;
}

LONG BaseMediaInfoHeaderAtom::loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag)
{
  if      (!memcmp(&atomTag.type, "gmin", ATOM_TYPE_SIZE))
  {
    bytesRead += mmioRead(fileHandle, (PCHAR)&baseMediaInfoAtom, sizeof(BaseMediaInfoAtom));
    return (MMIO_SUCCESS);
  }  else
  if      (!memcmp(&atomTag.type, "STpn", ATOM_TYPE_SIZE))
  {
    panoramaMediaInfoAtom = (PanoramaMediaInfoAtom*)(new char[atomTag.size - sizeof(AtomTag)]);
    bytesRead += mmioRead(fileHandle, (PCHAR)panoramaMediaInfoAtom, atomTag.size - sizeof(AtomTag));
    reverseBytes(&panoramaMediaInfoAtom->defNodeID);
    fixedToFloat(&panoramaMediaInfoAtom->defZoom);
    reverseBytes((PUSHORT)&panoramaMediaInfoAtom->numEntries);
    for (int j = 0; j < panoramaMediaInfoAtom->numEntries; j++)
    {
      reverseBytes(&panoramaMediaInfoAtom->idToTime[j].nodeID);
      reverseBytes(&panoramaMediaInfoAtom->idToTime[j].nodeTime);
    }
    return (MMIO_SUCCESS);
  }  else
    return(MMIO_ERROR);
}

BaseMediaInfoHeaderAtom::~BaseMediaInfoHeaderAtom()
{
  delete[] (char*)panoramaMediaInfoAtom;
}

ostream& operator<<(ostream& outStream, const BaseMediaInfoHeaderAtom& baseMediaInfoHeaderAtom)
{
  outStream << "Base Media Info Header [gmhd]\n" <<
               baseMediaInfoHeaderAtom.baseMediaInfoAtom;
  return (outStream);
}

/*
 * MediaInformationAtom
 */

MediaInformationAtom::MediaInformationAtom(AtomTag& atomTag)
                    : ContainerAtom(atomTag)
{
  videoMediaInfoHeaderAtom = 0;
  soundMediaInfoHeaderAtom = 0;
  baseMediaInfoHeaderAtom = 0;
  dataHandlerReferenceAtom = 0;
  sampleTableAtom = 0;
}

LONG MediaInformationAtom::loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag)
{
  if      (!memcmp(&atomTag.type, "vmhd", ATOM_TYPE_SIZE))
  {
    videoMediaInfoHeaderAtom = new VideoMediaInfoHeaderAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)videoMediaInfoHeaderAtom,
                          sizeof(VideoMediaInfoHeaderAtom));
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "smhd", ATOM_TYPE_SIZE))
  {
    soundMediaInfoHeaderAtom = new SoundMediaInfoHeaderAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)soundMediaInfoHeaderAtom,
                          sizeof(SoundMediaInfoHeaderAtom));
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "gmhd", ATOM_TYPE_SIZE))
  {
    baseMediaInfoHeaderAtom = new BaseMediaInfoHeaderAtom(atomTag);
    baseMediaInfoHeaderAtom->load(fileHandle);
    bytesRead += atomTag.size - sizeof(AtomTag);
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "hdlr", ATOM_TYPE_SIZE))
  {
    dataHandlerReferenceAtom = new HandlerReferenceAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)dataHandlerReferenceAtom,
                          sizeof(HandlerReferenceAtom) -
                          sizeof(HandlerReferenceAtom::componentName));
    LONG nameSize = atomTag.size - sizeof(AtomTag) - sizeof(HandlerReferenceAtom) +
                    sizeof(HandlerReferenceAtom::componentName);
    if (nameSize > 0)
    {
      BYTE nameLength;
      bytesRead += mmioRead(fileHandle, &nameLength, sizeof(BYTE));
      bytesRead += mmioRead(fileHandle, dataHandlerReferenceAtom->componentName, nameSize - sizeof(BYTE));
      dataHandlerReferenceAtom->componentName[nameLength] = '\0';
    }
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "stbl", ATOM_TYPE_SIZE))
  {
    sampleTableAtom = new SampleTableAtom(atomTag);
    sampleTableAtom->load(fileHandle);
    bytesRead += atomTag.size - sizeof(AtomTag);
    return(MMIO_SUCCESS);
  }
  else
    return(MMIO_ERROR);
}

MediaInformationAtom::~MediaInformationAtom()
{
  delete videoMediaInfoHeaderAtom;
  delete soundMediaInfoHeaderAtom;
  delete baseMediaInfoHeaderAtom;
  delete dataHandlerReferenceAtom;
  delete sampleTableAtom;
}

ostream& operator<<(ostream& outStream, const MediaInformationAtom& mediaInformationAtom)
{
  outStream << "Media Information [minf]\n";
  if (mediaInformationAtom.videoMediaInfoHeaderAtom)
    outStream << *mediaInformationAtom.videoMediaInfoHeaderAtom;
  if (mediaInformationAtom.soundMediaInfoHeaderAtom)
    outStream << *mediaInformationAtom.soundMediaInfoHeaderAtom;
  if (mediaInformationAtom.baseMediaInfoHeaderAtom)
    outStream << *mediaInformationAtom.baseMediaInfoHeaderAtom;
  if (mediaInformationAtom.dataHandlerReferenceAtom)
    outStream << *mediaInformationAtom.dataHandlerReferenceAtom;
  if (mediaInformationAtom.sampleTableAtom)
    outStream << *mediaInformationAtom.sampleTableAtom;
  return (outStream);
}

/*
 * MediaAtom
 */

MediaAtom::MediaAtom(AtomTag& atomTag)
         : ContainerAtom(atomTag)
{
  mediaHandlerReferenceAtom = 0;
  mediaInformationAtom = 0;
}

LONG MediaAtom::loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag)
{
  if      (!memcmp(&atomTag.type, "mdhd", ATOM_TYPE_SIZE))
  {
    bytesRead += mmioRead(fileHandle, (PCHAR)&mediaHeaderAtom, sizeof(MediaHeaderAtom));
    reverseBytes(&mediaHeaderAtom.timeScale);
    return (MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "hdlr", ATOM_TYPE_SIZE))
  {
    mediaHandlerReferenceAtom = new HandlerReferenceAtom;
    bytesRead += mmioRead(fileHandle, (PCHAR)mediaHandlerReferenceAtom,
                          sizeof(HandlerReferenceAtom) -
                          sizeof(HandlerReferenceAtom::componentName));
    LONG nameSize = atomTag.size - sizeof(AtomTag) - sizeof(HandlerReferenceAtom) +
                    sizeof(HandlerReferenceAtom::componentName);
    if (nameSize > 0)
    {
      BYTE nameLength;
      bytesRead += mmioRead(fileHandle, &nameLength, sizeof(BYTE));
      bytesRead += mmioRead(fileHandle, mediaHandlerReferenceAtom->componentName, nameSize - sizeof(BYTE));
      mediaHandlerReferenceAtom->componentName[nameLength] = '\0';
    }
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "minf", ATOM_TYPE_SIZE))
  {
    mediaInformationAtom = new MediaInformationAtom(atomTag);
    mediaInformationAtom->load(fileHandle);
    bytesRead += atomTag.size - sizeof(AtomTag);
    return(MMIO_SUCCESS);
  }
  else
    return(MMIO_ERROR);
}

MediaAtom::~MediaAtom()
{
  delete mediaHandlerReferenceAtom;
  delete mediaInformationAtom;
}

ostream& operator<<(ostream& outStream, const MediaAtom& mediaAtom)
{
  outStream << "Media [mdia]\n" <<
               mediaAtom.mediaHeaderAtom;
  if (mediaAtom.mediaHandlerReferenceAtom)
    outStream << *mediaAtom.mediaHandlerReferenceAtom;
  if (mediaAtom.mediaInformationAtom)
    outStream << *mediaAtom.mediaInformationAtom;
  return (outStream);
}

/*
 * EditAtom
 */

EditAtom::EditAtom(AtomTag& atomTag)
        : ContainerAtom(atomTag)
{
}

LONG EditAtom::loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag)
{
  if      (!memcmp(&atomTag.type, "elst", ATOM_TYPE_SIZE))
  {
    bytesRead += mmioRead(fileHandle, (PCHAR)&editListAtom,
                          sizeof(EditListAtom) - sizeof(EditListAtom::editListTable));
    reverseBytes(&editListAtom.numberOfEntries);
    for (int i = editListAtom.numberOfEntries; i > 0; i--)
    {
      EditList editList;
      bytesRead += mmioRead(fileHandle, (PCHAR)&editList, sizeof(EditList));
      editListAtom.editListTable.add(editList);
    }
    return(MMIO_SUCCESS);
  }
  else
    return(MMIO_ERROR);
}

EditAtom::~EditAtom()
{
}

ostream& operator<<(ostream& outStream, const EditAtom& editAtom)
{
  outStream << "Edit [edts]\n" <<
               editAtom.editListAtom;
  return (outStream);
}

/*
 * TrackAtom
 */

TrackAtom::TrackAtom()
         : ContainerAtom()
{
  editAtom = 0;
  mediaAtom = 0;
}

TrackAtom::TrackAtom(AtomTag& atomTag)
         : ContainerAtom(atomTag)
{
  editAtom = 0;
  mediaAtom = 0;
}

LONG TrackAtom::loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag)
{
  if      (!memcmp(&atomTag.type, "tkhd", ATOM_TYPE_SIZE))
  {
    bytesRead += mmioRead(fileHandle, (PCHAR)&trackHeaderAtom, sizeof(TrackHeaderAtom));
    reverseBytes(&trackHeaderAtom.track);
    reverseBytes(&trackHeaderAtom.width);
    reverseBytes(&trackHeaderAtom.height);
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "edts", ATOM_TYPE_SIZE))
  {
    editAtom = new EditAtom(atomTag);
    editAtom->load(fileHandle);
    bytesRead += atomTag.size - sizeof(AtomTag);
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "mdia", ATOM_TYPE_SIZE))
  {
    mediaAtom = new MediaAtom(atomTag);
    mediaAtom->load(fileHandle);
    bytesRead += atomTag.size - sizeof(AtomTag);
    return(MMIO_SUCCESS);
  }
  else
    return(MMIO_ERROR);
}

TrackAtom::~TrackAtom()
{
  delete editAtom;
  delete mediaAtom;
}

ostream& operator<<(ostream& outStream, const TrackAtom& trackAtom)
{
  outStream << "Track [trak]\n" <<
               trackAtom.trackHeaderAtom;
  if (trackAtom.editAtom)
    outStream << *trackAtom.editAtom;
  if (trackAtom.mediaAtom)
    outStream << *trackAtom.mediaAtom;
  return (outStream);
}


/*
 * MovieAtom
 */

MovieAtom::MovieAtom()
         : ContainerAtom()
{
  userDataAtom = 0;
}

MovieAtom::MovieAtom(AtomTag& atomTag)
         : ContainerAtom(atomTag)
{
  userDataAtom = 0;
  hasVideo_p = isMusicOnly_p = isSoundOnly_p = false;
}

LONG MovieAtom::loadAtom(HMMIO fileHandle, ULONG& bytesRead, AtomTag& atomTag)
{
  if      (!memcmp(&atomTag.type, "mvhd", ATOM_TYPE_SIZE))
  {
    bytesRead += mmioRead(fileHandle, (PCHAR)&movieHeaderAtom, sizeof(MovieHeaderAtom));
    reverseBytes(&movieHeaderAtom.creationTime);
    reverseBytes(&movieHeaderAtom.modificationTime);
    reverseBytes(&movieHeaderAtom.timeScale);
    reverseBytes(&movieHeaderAtom.duration);
    reverseBytes(&movieHeaderAtom.preferredRate);
    reverseBytes(&movieHeaderAtom.preferredVolume);
    reverseBytes(&movieHeaderAtom.previewTime);
    reverseBytes(&movieHeaderAtom.previewDuration);
    reverseBytes(&movieHeaderAtom.posterTime);
    reverseBytes(&movieHeaderAtom.selectionTime);
    reverseBytes(&movieHeaderAtom.selectionDuration);
    reverseBytes(&movieHeaderAtom.currentTime);
    reverseBytes(&movieHeaderAtom.nextTrack);
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "trak", ATOM_TYPE_SIZE))
  {
    //KNOWN BUG:
    //This new trackatom is never deleted because it contains pointers to sub-elements, and deleting them
    //would cause the copied track object in the collection to point to invalid data.
    //Possible solutions: 1) Implement deep copy for mediaAtom and editAtom
    //                    2) Use a reference-counted version of trackAtom in the collection
    //                      3) Change collection to be over trackatom*.  Requires changes to all client code.
    TrackAtom* trackAtom = new TrackAtom(atomTag);
    trackAtom->load(fileHandle);
    bytesRead += atomTag.size - sizeof(AtomTag);
    trackList.add(*trackAtom);
    return(MMIO_SUCCESS);
  }
  else if (!memcmp(&atomTag.type, "udta", ATOM_TYPE_SIZE))
  {
    userDataAtom = new UserDataAtom(atomTag);
    userDataAtom->load(fileHandle);
    bytesRead += atomTag.size - sizeof(AtomTag);
    return(MMIO_SUCCESS);
  }
  else
    return(MMIO_ERROR);
}

Boolean MovieAtom::isWorld()
{
  return (userDataAtom && userDataAtom->controllerTypeAtom);
}

Boolean MovieAtom::hasVideo()
{
  return hasVideo_p;
}

Boolean MovieAtom::isMusicOnly()
{
  return isMusicOnly_p;
}

Boolean MovieAtom::isSoundOnly()
{
  return isSoundOnly_p;
}

MovieAtom::~MovieAtom()
{
  delete userDataAtom;
}

/**************************************************************************
 * Load Movie                                                            **
 **************************************************************************/
LONG loadMovie(MovieAtom& movieAtom, ULONG& totalDataBytes, HMMIO fileHandle)
{
  AtomTag atomTag;

  try
  {
    mmioSeek(fileHandle, 0, SEEK_SET);
    mmioRead(fileHandle, (PCHAR)&atomTag, sizeof(AtomTag));
    reverseBytes(&atomTag.size);

    if (!memcmp(&atomTag.type, "moov", ATOM_TYPE_SIZE) ||
        !memcmp(&atomTag.type, "skip", ATOM_TYPE_SIZE))
    {
      memcpy(&movieAtom.atomTag_p, &atomTag, sizeof(atomTag));
      movieAtom.load(fileHandle);
      mmioRead(fileHandle, (PCHAR)&atomTag, sizeof(AtomTag));
      reverseBytes(&atomTag.size);
      totalDataBytes = atomTag.size - sizeof(AtomTag);
    }
    else if (!memcmp(&atomTag.type, "mdat", ATOM_TYPE_SIZE))
    {
      totalDataBytes = atomTag.size - sizeof(AtomTag);
      mmioSeek(fileHandle, totalDataBytes, SEEK_CUR);
      mmioRead(fileHandle, (PCHAR)&atomTag, sizeof(AtomTag));
      reverseBytes(&atomTag.size);
      memcpy(&movieAtom.atomTag_p, &atomTag, sizeof(atomTag));
      movieAtom.load(fileHandle);
    }
    else
      return MMIO_ERROR;
  }
  catch (NoMoreData noMoreData)
  {
    return MMIO_ERROR;
  }

  // Set content flags
  ITabularSequence<TrackAtom>::Cursor* trackCursor = movieAtom.trackList.newCursor();
  for (trackCursor->setToFirst(); trackCursor->isValid(); trackCursor->setToNext())
  {
    MediaInformationAtom* mia = trackCursor->element().mediaAtom->mediaInformationAtom;
    if (mia && mia->videoMediaInfoHeaderAtom)
    {
      movieAtom.hasVideo_p = true;
      movieAtom.isMusicOnly_p = false;
      movieAtom.isSoundOnly_p = false;
    }
    else if (!movieAtom.hasVideo_p && mia && mia->soundMediaInfoHeaderAtom)
      movieAtom.isSoundOnly_p = true;
    else if (!movieAtom.hasVideo_p && mia && mia->sampleTableAtom && mia->sampleTableAtom->sampleDescriptionAtom &&
            !memcmp(&mia->sampleTableAtom->sampleDescriptionAtom->sampleDescriptionTable.elementAtPosition(1)->
                    musicDescription.codecFourcc, "musi", 4))
      movieAtom.isMusicOnly_p = true;
  }
  delete trackCursor;

  return MMIO_SUCCESS;
}

/**************************************************************************
 * Panorama Media Routines                                               **
 **************************************************************************/
PanoramaSample::PanoramaSample()
{
  hotSpotTableAtom = 0;
  linkTableAtom = 0;
  navgTableAtom = 0;
  stringTableAtom = 0;
}

void PanoramaSample::load(HMMIO fileHandle, ULONG size)
{
  LONG bytesRead = 0;
  AtomTag atomTag;

  while (size - bytesRead >= sizeof(AtomTag))
  {
    bytesRead += mmioRead(fileHandle, (PCHAR)&atomTag, sizeof(AtomTag));
    reverseBytes(&atomTag.size);

    if (!memcmp(&atomTag.type, "pHdr", ATOM_TYPE_SIZE))
    {
      bytesRead += mmioRead(fileHandle, (PCHAR)&panoSampleHeaderAtom, sizeof(PanoSampleHeaderAtom));
      fixedToFloat(&panoSampleHeaderAtom.defHPan);
      fixedToFloat(&panoSampleHeaderAtom.defVPan);
      fixedToFloat(&panoSampleHeaderAtom.defZoom);
    }
    else if (!memcmp(&atomTag.type, "pHot", ATOM_TYPE_SIZE))
    {
      hotSpotTableAtom = (HotSpotTableAtom*)(new char[atomTag.size - sizeof(AtomTag)]);
      bytesRead += mmioRead(fileHandle, (PCHAR)hotSpotTableAtom, atomTag.size - sizeof(AtomTag));
      reverseBytes((PUSHORT)&hotSpotTableAtom->numHotSpots);
      for (int j = 0; j < hotSpotTableAtom->numHotSpots; j++)
      {
        reverseBytes(&hotSpotTableAtom->hotSpots[j].hotSpotID);
        reverseBytes(&hotSpotTableAtom->hotSpots[j].typeData);
        fixedToFloat(&hotSpotTableAtom->hotSpots[j].viewHPan);
        fixedToFloat(&hotSpotTableAtom->hotSpots[j].viewVPan);
        fixedToFloat(&hotSpotTableAtom->hotSpots[j].viewZoom);
        reverseBytes((PULONG)&hotSpotTableAtom->hotSpots[j].nameStrOffset);
        reverseBytes((PULONG)&hotSpotTableAtom->hotSpots[j].commentStrOffset);
      }
    }
    else if (!memcmp(&atomTag.type, "pLnk", ATOM_TYPE_SIZE))
    {
      linkTableAtom = (LinkTableAtom*)(new char[atomTag.size - sizeof(AtomTag)]);
      bytesRead += mmioRead(fileHandle, (PCHAR)linkTableAtom, atomTag.size - sizeof(AtomTag));
      reverseBytes((PUSHORT)&linkTableAtom->numLinks);
      for (int j = 0; j < linkTableAtom->numLinks; j++)
      {
        reverseBytes(&linkTableAtom->links[j].linkID);
        reverseBytes(&linkTableAtom->links[j].toNodeID);
        fixedToFloat(&linkTableAtom->links[j].toHPan);
        fixedToFloat(&linkTableAtom->links[j].toVPan);
        fixedToFloat(&linkTableAtom->links[j].toZoom);
        reverseBytes((PULONG)&linkTableAtom->links[j].nameStrOffset);
        reverseBytes((PULONG)&linkTableAtom->links[j].commentStrOffset);
      }
    }
    else if (!memcmp(&atomTag.type, "pNav", ATOM_TYPE_SIZE))
    {
      navgTableAtom = (NavgTableAtom*)(new char[atomTag.size - sizeof(AtomTag)]);
      bytesRead += mmioRead(fileHandle, (PCHAR)navgTableAtom, atomTag.size - sizeof(AtomTag));
      reverseBytes((PUSHORT)&navgTableAtom->numObjects);
      for (int j = 0; j < navgTableAtom->numObjects; j++)
      {
        reverseBytes(&navgTableAtom->objects[j].objID);
        fixedToFloat(&navgTableAtom->objects[j].navgHPan);
        fixedToFloat(&navgTableAtom->objects[j].navgVPan);
        fixedToFloat(&navgTableAtom->objects[j].navgZoom);
        reverseBytes((PULONG)&navgTableAtom->objects[j].nameStrOffset);
        reverseBytes((PULONG)&navgTableAtom->objects[j].commentStrOffset);
      }
    }
    else if (!memcmp(&atomTag.type, "strT", ATOM_TYPE_SIZE))
    {
      stringTableAtom = new char[atomTag.size - sizeof(AtomTag)];
      bytesRead += mmioRead(fileHandle, (PCHAR)stringTableAtom, atomTag.size - sizeof(AtomTag));
    }
  }
}

PanoramaSample::~PanoramaSample()
{
  delete[] (char*)hotSpotTableAtom;
  delete[] (char*)linkTableAtom;
  delete[] (char*)navgTableAtom;
  delete[] stringTableAtom;
}

/**************************************************************************
 * Output Routines                                                       **
 **************************************************************************/
ostream& operator<<(ostream& outStream, const MovieAtom& movieAtom)
{
  outStream << "Movie [moov]\n" <<
               movieAtom.movieHeaderAtom;
  ITabularSequence<TrackAtom>::Cursor* trackCursor = movieAtom.trackList.newCursor();
  for (trackCursor->setToFirst(); trackCursor->isValid(); trackCursor->setToNext())
  {
    outStream << trackCursor->element();
  }
  delete trackCursor;
  if (movieAtom.userDataAtom)
    outStream << *movieAtom.userDataAtom;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const MediaHeaderAtom& mediaHeaderAtom)
{
  outStream << "Media Header [mdhd]\n" <<
               "version = " << mediaHeaderAtom.version << '\n' <<
               "flags = " << mediaHeaderAtom.flags << '\n' <<
               "creationTime = " << mediaHeaderAtom.creationTime << '\n' <<
               "modificationTime = " << mediaHeaderAtom.modificationTime << '\n' <<
               "timeScale = " << mediaHeaderAtom.timeScale << '\n' <<
               "duration = " << mediaHeaderAtom.duration << '\n' <<
               "language = " << mediaHeaderAtom.language << '\n' <<
               "quality = " << mediaHeaderAtom.quality << '\n' << endl;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const TrackHeaderAtom& trackHeaderAtom)
{
  outStream << "Track Header [tkhd]\n" <<
               "version = " << trackHeaderAtom.version << '\n' <<
               "flags = " << trackHeaderAtom.flags << '\n' <<
               "creationTime = " << trackHeaderAtom.creationTime << '\n' <<
               "modificationTime = " << trackHeaderAtom.modificationTime << '\n' <<
               "track = " << trackHeaderAtom.track << '\n' <<
               "duration = " << trackHeaderAtom.duration << '\n' <<
               "layer = " << trackHeaderAtom.layer << '\n' <<
               "alternateGroup = " << trackHeaderAtom.alternateGroup << '\n' <<
               "volume = " << trackHeaderAtom.volume << '\n' <<
               "matrix = " << trackHeaderAtom.matrix << '\n' <<
               "width = " << trackHeaderAtom.width << '\n' <<
               "height = " << trackHeaderAtom.height << '\n' << endl;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const EditListAtom& editListAtom)
{
  outStream << "Edit List [elst]\n" <<
               "version = " << editListAtom.version << '\n' <<
               "flags = " << editListAtom.flags << '\n' <<
               "numberOfEntries = " << editListAtom.numberOfEntries << '\n';
  ITabularSequence<EditList>::Cursor* editListCursor = editListAtom.editListTable.newCursor();
  for (editListCursor->setToFirst(); editListCursor->isValid(); editListCursor->setToNext())
  {
    outStream << editListCursor->element().duration << '\t' <<
                 editListCursor->element().mediaTime << '\t' <<
                 editListCursor->element().rate << '\n';
  }
  outStream << endl;
  delete editListCursor;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const MovieHeaderAtom& movieHeaderAtom)
{
  outStream << "Movie Header [mvhd]\n" <<
               "version = " << movieHeaderAtom.version << '\n' <<
               "flags = " << movieHeaderAtom.flags << '\n' <<
               "creationTime = " << movieHeaderAtom.creationTime << '\n' <<
               "modificationTime = " << movieHeaderAtom.modificationTime << '\n' <<
               "timeScale = " << movieHeaderAtom.timeScale << '\n' <<
               "duration = " << movieHeaderAtom.duration << '\n' <<
               "preferredRate = " << movieHeaderAtom.preferredRate << '\n' <<
               "preferredVolume = " << movieHeaderAtom.preferredVolume << '\n' <<
               "matrix = " << movieHeaderAtom.matrix << '\n' <<
               "previewTime = " << movieHeaderAtom.previewTime << '\n' <<
               "previewDuration = " << movieHeaderAtom.previewDuration << '\n' <<
               "posterTime = " << movieHeaderAtom.posterTime << '\n' <<
               "selectionTime = " << movieHeaderAtom.selectionTime << '\n' <<
               "selectionDuration = " << movieHeaderAtom.selectionDuration << '\n' <<
               "currentTime = " << movieHeaderAtom.currentTime << '\n' <<
               "nextTrack = " << movieHeaderAtom.nextTrack << '\n' << endl;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const HandlerReferenceAtom& handlerReferenceAtom)
{
  outStream << "Handler Reference [hdlr]\n" <<
               "version = " << handlerReferenceAtom.version << '\n' <<
               "flags = " << handlerReferenceAtom.flags << '\n' <<
               "componentType = " << handlerReferenceAtom.componentType << '\n' <<
               "componentSubType = " << handlerReferenceAtom.componentSubType << '\n' <<
               "componentManufacturer = " << handlerReferenceAtom.componentManufacturer << '\n' <<
               "componentFlags = " << handlerReferenceAtom.componentFlags << '\n' <<
               "componentFlagsMask = " << handlerReferenceAtom.componentFlagsMask << '\n' <<
               "componentName = " << handlerReferenceAtom.componentName << '\n' << endl;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const VideoMediaInfoHeaderAtom& videoMediaInfoHeaderAtom)
{
  outStream << "Video Media Info Header [vmhd]\n" <<
               "version = " << videoMediaInfoHeaderAtom.version << '\n' <<
               "flags = " << videoMediaInfoHeaderAtom.flags << '\n' <<
               "graphicsMode = " << videoMediaInfoHeaderAtom.graphicsMode << '\n' <<
               "opcolor = " << videoMediaInfoHeaderAtom.opcolor << '\n' << endl;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const SoundMediaInfoHeaderAtom& soundMediaInfoHeaderAtom)
{
  outStream << "Sound Media Info Header [smhd]\n" <<
               "version = " << soundMediaInfoHeaderAtom.version << '\n' <<
               "flags = " << soundMediaInfoHeaderAtom.flags << '\n' <<
               "balance = " << soundMediaInfoHeaderAtom.balance << '\n' << endl;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const BaseMediaInfoAtom& baseMediaInfoAtom)
{
  outStream << "Base Media Info [gmin]\n" <<
               "version = " << baseMediaInfoAtom.version << '\n' <<
               "flags = " << baseMediaInfoAtom.flags << '\n' <<
               "graphicsMode = " << baseMediaInfoAtom.graphicsMode << '\n' <<
               "opcolor = " << baseMediaInfoAtom.opcolor << '\n' <<
               "balance = " << baseMediaInfoAtom.balance << '\n' << endl;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const SampleDescriptionAtom& sampleDescriptionAtom)
{
  outStream << "Sample Description [stsd]\n" <<
               "version = " << sampleDescriptionAtom.version << '\n' <<
               "flags = " << sampleDescriptionAtom.flags << '\n' <<
               "numberOfEntries = " << sampleDescriptionAtom.numberOfEntries << '\n';
  ITabularSequence<SampleDescription*>::Cursor* sampleDescriptionCursor = sampleDescriptionAtom.sampleDescriptionTable.newCursor();
  for (sampleDescriptionCursor->setToFirst(); sampleDescriptionCursor->isValid(); sampleDescriptionCursor->setToNext())
  {
    char *fourcc = (char*)&(sampleDescriptionCursor->element()->imageDescription.codecFourcc);
    outStream << "codecFourcc = " << fourcc[0] << fourcc[1] << fourcc[2] << fourcc[3] << '\n';
    if      (!memcmp(fourcc, "musi", 4))
    {
      outStream << endl;
    } else
    if      (!memcmp(fourcc, "pano", 4))
    {
      const PanoramaDescription& pd = sampleDescriptionCursor->element()->panoramaDescription;
      outStream <<
                   "majorVersion = " << pd.majorVersion << '\n' <<
                   "minorVersion = " << pd.minorVersion << '\n' <<
                   "sceneTrackID = " << pd.sceneTrackID << '\n' <<
                   "loResSceneTrackID = " << pd.loResSceneTrackID << '\n' <<
                   "hotSpotTrackID = " << pd.hotSpotTrackID << '\n' <<
                   "hPanStart = " << pd.hPanStart << '\n' <<
                   "hPanEnd = " << pd.hPanEnd << '\n' <<
                   "vPanTop = " << pd.vPanTop << '\n' <<
                   "vPanBottom = " << pd.vPanBottom << '\n' <<
                   "minimumZoom = " << pd.minimumZoom << '\n' <<
                   "maximumZoom = " << pd.maximumZoom << '\n' <<
                   "sceneSizeX = " << pd.sceneSizeX << '\n' <<
                   "sceneSizeY = " << pd.sceneSizeY << '\n' <<
                   "numFrames = " << pd.numFrames << '\n' <<
                   "sceneNumFramesX = " << pd.sceneNumFramesX << '\n' <<
                   "sceneNumFramesY = " << pd.sceneNumFramesY << '\n' <<
                   "sceneColorDepth = " << pd.sceneColorDepth << '\n' <<
                   "hotSpotSizeX = " << pd.hotSpotSizeX << '\n' <<
                   "hotSpotSizeY = " << pd.hotSpotSizeY << '\n' <<
                   "hotSpotNumFramesX = " << pd.hotSpotNumFramesX << '\n' <<
                   "hotSpotNumFramesY = " << pd.hotSpotNumFramesY << '\n' <<
                   "hotSpotColorDepth = " << pd.hotSpotColorDepth << '\n' << endl;
    } else
    {
      char *vendor = (char*)&(sampleDescriptionCursor->element()->imageDescription.vendor);
      outStream <<
                   "dataReference = " << sampleDescriptionCursor->element()->imageDescription.dataReference << '\n' <<
                   "version = " << sampleDescriptionCursor->element()->imageDescription.version << '\n' <<
                   "revisionLevel = " << sampleDescriptionCursor->element()->imageDescription.revisionLevel << '\n' <<
                   "vendor = " << vendor[0] << vendor[1] << vendor[2] << vendor[3]  << '\n' <<
                   "temporalQuality = " << sampleDescriptionCursor->element()->imageDescription.temporalQuality << '\n' <<
                   "spatialQuality = " << sampleDescriptionCursor->element()->imageDescription.spatialQuality << '\n' <<
                   "width = " << sampleDescriptionCursor->element()->imageDescription.width << '\n' <<
                   "height = " << sampleDescriptionCursor->element()->imageDescription.height << '\n' <<
                   "hResolution = " << sampleDescriptionCursor->element()->imageDescription.hResolution << '\n' <<
                   "vResolution = " << sampleDescriptionCursor->element()->imageDescription.vResolution << '\n' <<
                   "dataSize = " << sampleDescriptionCursor->element()->imageDescription.dataSize << '\n' <<
                   "frameCount = " << sampleDescriptionCursor->element()->imageDescription.frameCount << '\n' <<
                   "nameLength = " << (USHORT)sampleDescriptionCursor->element()->imageDescription.nameLength << '\n' <<
                   "name = " << sampleDescriptionCursor->element()->imageDescription.name << '\n' <<
                   "depth = " << sampleDescriptionCursor->element()->imageDescription.depth << '\n' <<
                   "colorTable = " << sampleDescriptionCursor->element()->imageDescription.colorTable << '\n' << endl;
    }
  }
  delete sampleDescriptionCursor;
  return (outStream);
}

/*
ostream& printSampleAsSound(ostream& outStream, const SampleDescriptionAtom& sampleDescriptionAtom)
{
 // outStream << sampleDescriptionAtom.atomTag <<
    //           "version = " << sampleDescriptionAtom.version << '\n' <<
    //           "flags = " << sampleDescriptionAtom.flags << '\n' <<
    //           "numberOfEntries = " << sampleDescriptionAtom.numberOfEntries << '\n';
  ISequence<SampleDescription>::Cursor* sampleDescriptionCursor = sampleDescriptionAtom.sampleDescriptionTable.newCursor();
  for (sampleDescriptionCursor->setToFirst(); sampleDescriptionCursor->isValid(); sampleDescriptionCursor->setToNext())
  {
    char *fourcc = (char*)&(sampleDescriptionCursor->element().codecFourcc);
    char *vendor = (char*)&(sampleDescriptionCursor->element().vendor);
    short channels = HIUSHORT(sampleDescriptionCursor->element().temporalQuality);
    outStream << "codecFourcc = " << fourcc[0] << fourcc[1] << fourcc[2] << fourcc[3] << '\n' <<
    //             "dataReference = " << sampleDescriptionCursor->element().dataReference << '\n' <<
    //             "version = " << sampleDescriptionCursor->element().version << '\n' <<
    //             "revisionLevel = " << sampleDescriptionCursor->element().revisionLevel << '\n' <<
    //             "vendor = " << vendor[0] << vendor[1] << vendor[2] << vendor[3]  << '\n' <<
                 "channels = " << channels << '\n' <<
    //             "spatialQuality = " << sampleDescriptionCursor->element().spatialQuality << '\n' <<
                 "samplingRate = " << sampleDescriptionCursor->element().width << '\n';
    //             "height = " << sampleDescriptionCursor->element().height << '\n' <<
    //             "hResolution = " << sampleDescriptionCursor->element().hResolution << '\n' <<
    //             "vResolution = " << sampleDescriptionCursor->element().vResolution << '\n' <<
    //             "dataSize = " << sampleDescriptionCursor->element().dataSize << '\n' <<
    //             "frameCount = " << sampleDescriptionCursor->element().frameCount << '\n' <<
    //             "nameLength = " << (USHORT)sampleDescriptionCursor->element().nameLength << '\n' <<
    //             "name = " << sampleDescriptionCursor->element().name << '\n' <<
    //             "depth = " << sampleDescriptionCursor->element().depth << '\n' << endl;
    //             "colorTable = " << sampleDescriptionCursor->element().colorTable << '\n' << endl;
  }
  delete sampleDescriptionCursor;
  return (outStream);
}       */

ostream& operator<<(ostream& outStream, const TimeToSampleAtom& timeToSampleAtom)
{
  outStream << "Time To Sample [stts]\n" <<
               "version = " << timeToSampleAtom.version << '\n' <<
               "flags = " << timeToSampleAtom.flags << '\n' <<
               "numberOfEntries = " << timeToSampleAtom.numberOfEntries << '\n';
  ITabularSequence<TimeToSample>::Cursor* timeToSampleCursor = timeToSampleAtom.timeToSampleTable.newCursor();
  for (timeToSampleCursor->setToFirst(); timeToSampleCursor->isValid(); timeToSampleCursor->setToNext())
  {
    outStream << timeToSampleCursor->element().count << '\t' <<
                 timeToSampleCursor->element().duration << '\n';
  }
  outStream << endl;
  delete timeToSampleCursor;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const SyncSampleAtom& syncSampleAtom)
{
  outStream << "Sync Sample [stss]\n" <<
               "version = " << syncSampleAtom.version << '\n' <<
               "flags = " << syncSampleAtom.flags << '\n' <<
               "numberOfEntries = " << syncSampleAtom.numberOfEntries << '\n';
  IKeySortedSet<ULONG, ULONG>::Cursor* syncSampleCursor = syncSampleAtom.syncSampleTable.newCursor();
  for (syncSampleCursor->setToFirst(); syncSampleCursor->isValid(); syncSampleCursor->setToNext())
  {
    outStream << syncSampleCursor->element() << '\t';
  }
  outStream << '\n' << endl;
  delete syncSampleCursor;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const SampleToChunkAtom& sampleToChunkAtom)
{
  outStream << "Sample To Chunk [stsc]\n" <<
               "version = " << sampleToChunkAtom.version << '\n' <<
               "flags = " << sampleToChunkAtom.flags << '\n' <<
               "numberOfEntries = " << sampleToChunkAtom.numberOfEntries << '\n';
  ITabularSequence<SampleToChunk>::Cursor* sampleToChunkCursor = sampleToChunkAtom.sampleToChunkTable.newCursor();
  for (sampleToChunkCursor->setToFirst(); sampleToChunkCursor->isValid(); sampleToChunkCursor->setToNext())
  {
    outStream << sampleToChunkCursor->element().firstChunk << '\t' <<
                 sampleToChunkCursor->element().samplesPerChunk << '\t' <<
                 sampleToChunkCursor->element().sampleDescription << '\n';
  }
  outStream << endl;
  delete sampleToChunkCursor;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const SampleSizeAtom& sampleSizeAtom)
{
  outStream << "Sample Size [stsz]\n" <<
               "version = " << sampleSizeAtom.version << '\n' <<
               "flags = " << sampleSizeAtom.flags << '\n' <<
               "sampleSize = " << sampleSizeAtom.sampleSize << '\n' <<
               "numberOfEntries = " << sampleSizeAtom.numberOfEntries << '\n';
  ITabularSequence<ULONG>::Cursor* sampleSizeCursor = sampleSizeAtom.sampleSizeTable.newCursor();
  for (sampleSizeCursor->setToFirst(); sampleSizeCursor->isValid(); sampleSizeCursor->setToNext())
  {
    outStream << sampleSizeCursor->element() << '\t';
  }
  outStream << '\n' << endl;
  delete sampleSizeCursor;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const ChunkOffsetAtom& chunkOffsetAtom)
{
  outStream << "Chunk Offset [stco]\n" <<
               "version = " << chunkOffsetAtom.version << '\n' <<
               "flags = " << chunkOffsetAtom.flags << '\n' <<
               "numberOfEntries = " << chunkOffsetAtom.numberOfEntries << '\n';
  ITabularSequence<ULONG>::Cursor* chunkOffsetCursor = chunkOffsetAtom.chunkOffsetTable.newCursor();
  for (chunkOffsetCursor->setToFirst(); chunkOffsetCursor->isValid(); chunkOffsetCursor->setToNext())
  {
    outStream << chunkOffsetCursor->element() << '\t';
  }
  outStream << '\n' << endl;
  delete chunkOffsetCursor;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const NavigationAtom& navigationAtom)
{
  outStream << "Navigation [NAVG]\n" <<
               "version = " << navigationAtom.version << '\n' <<
               "numberOfColumns = " << navigationAtom.numberOfColumns << '\n' <<
               "numberOfRows = " << navigationAtom.numberOfRows << '\n' <<
               "reserved1 = " << navigationAtom.reserved1 << '\n' <<
               "loopSize = " << navigationAtom.loopSize << '\n' <<
               "frameDuration = " << navigationAtom.frameDuration << '\n' <<
               "movieType = " << navigationAtom.movieType << '\n' <<
               "loopTicks = " << navigationAtom.loopTicks << '\n' <<
               "fieldOfView = " << navigationAtom.fieldOfView << '\n' <<
               "startHPan = " << navigationAtom.startHPan << '\n' <<
               "endHPan = " << navigationAtom.endHPan << '\n' <<
               "startVPan = " << navigationAtom.startVPan << '\n' <<
               "endVPan = " << navigationAtom.endVPan << '\n' <<
               "initialHPan = " << navigationAtom.initialHPan << '\n' <<
               "initialVPan = " << navigationAtom.initialVPan << '\n' <<
               "reserved2 = " << navigationAtom.reserved2 << '\n' << endl;
  return (outStream);
}

ostream& operator<<(ostream& outStream, const ControllerTypeAtom& controllerTypeAtom)
{
  char *fourcc = (char*)&(controllerTypeAtom.controllerFourcc);
  outStream << "Controller Type [ctyp]\n" <<
               "controllerFourcc = " << fourcc[0] << fourcc[1] << fourcc[2] << fourcc[3] << endl;
  return (outStream);
}

