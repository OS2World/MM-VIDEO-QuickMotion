#ifndef _IMMDEV_
   #define _IMMDEV_
/*******************************************************************************
* FILE NAME:  immdev.hpp                                                       *
*                                                                              *
* DESCRIPTION:                                                                 *
*   Declaration of the classes:                                                *
*     IMMDevice                                                                *
*                                                                              *
* COPYRIGHT:                                                                   *
*   IBM Open Class Library                                                     *
*   (C) Copyright International Business Machines Corporation 1992, 1995       *
*   Licensed Material - Program-Property of IBM - All Rights Reserved.         *
*   US Government Users Restricted Rights - Use, duplication, or disclosure    *
*   restricted by GSA ADP Schedule Contract with IBM Corp.                     *
*                                                                              *
*******************************************************************************/
#include <istdntfy.hpp>
#include <ipoint.hpp>
#include <immtime.hpp>
#include <immevt.hpp>
#include <immspeed.hpp>
#include <inotify.hpp>
#include <istring.hpp>

class IMMDeviceHandler;
class IMMDeviceNotifyHandler;
class IMMDeviceData;
class IMMMessageWindow;

#pragma pack(4)


class IMMDevice : public IStandardNotifier {
typedef IStandardNotifier
  Inherited;
public:
/*-------------------------------- Constructors ------------------------------*/
virtual
  ~IMMDevice();

/*-------------------------------- Call Type ---------------------------------*/
enum CallType
{
  nowait = 0X00000001,
  wait   = 0x00000002
};

/*-------------------------------- Device Type and Description ---------------*/
static const unsigned long
  cdxa,
  audioCD,
  dat,
  audioTape,
  waveAudio,
  sequencer,
  ampMixer,
  videoTape,
  digitalVideo,
  videoDisc,
  overlay,
  animation,
  speaker,
  headphone,
  microphone,
  monitor,
  other,
  allDevices;

unsigned long
  deviceType    (CallType call = wait) const;
IString
  description        (CallType call = wait) const;

/*-------------------------------- Preroll Type ------------------------------*/
enum PrerollType
{
   variable = 1,
   fixed,
   none
};
PrerollType
  prerollType       (CallType call = wait) const;
IMMTime
  prerollTime       (CallType call = wait) const;

/*------------------------------- Capabilities -------------------------------*/
Boolean
  supportsEject          (CallType call = wait) const,
  supportsDisableEject   (CallType call = wait) const,
  supportsPlay           (CallType call = wait) const,
  supportsDigitalTransfer(CallType call = wait) const,
  supportsRecord         (CallType call = wait) const,
  supportsRecordInsertion(CallType call = wait) const,
  supportsSave           (CallType call = wait) const,
  supportsVolumeAdjustment(CallType call = wait) const,
  supportsStreaming      (CallType call = wait) const,
  supportsAudio          (CallType call = wait) const,
  supportsVideo          (CallType call = wait) const,
  supportsCommand        (IMMNotifyEvent::Command command,
                          CallType call = wait) const,
  requiresFiles          (CallType call = wait) const;

/*-------------------------------- Share Mode & Acquire ----------------------*/
enum ShareMode
{
  shareable,
  isolatedExclusive = 0x00000100,
  exclusive         = 0x00000200
};
virtual IMMDevice
  &acquire   (ShareMode acquire = shareable,
              Boolean queuedForResources = false,
              CallType call = wait),
  &release   (CallType call = wait);
Boolean
  isAcquired (CallType call = wait) const;

/*-------------------------------- Event Type and DeleteEvents ---------------*/
enum EventType
{
  allEvents,
  notifyEvent = 0x0500,
  passDeviceEvent,
  positionChangeEvent,
  cuePointEvent,
  deviceEvent,
  synchEvent
};
virtual IMMDevice
  &deletePendingEvents (EventType event = allEvents);

/*-------------------------------- Open and Close ----------------------------*/
virtual IMMDevice
  &close             (CallType call = wait),
  &open              (const IString& fileOrDevice = IString(),
                      Boolean shareable = true,
                      CallType call = wait),
  &openOnThread      (const IString& fileOrDevice = IString(),
                      Boolean shareable = true,
                      CallType call = wait);
IMMDevice
  &setCloseOnDestroy (Boolean close = false);
Boolean
  isOpen             () const,
  isCloseOnDestroy   () const;

/*-------------------------------- Mode --------------------------------------*/
enum Mode
{
  notReady  = 1,
  paused,
  playing,
  stopped,
  recording,
  seeking
};
virtual Mode
  mode  (CallType call = wait) const;

/*-------------------------------- Audio -------------------------------------*/
enum AudioChannel
{
  all,
  left,
  right
};
virtual IMMDevice
  &setVolume   (unsigned long volume,
                AudioChannel channel = all,
                const IMMMillisecondTime& over = IMMMillisecondTime(),
                CallType call = wait),
  &enableAudio (Boolean enable= true,
                AudioChannel channel = all,
                const IMMMillisecondTime& over = IMMMillisecondTime(),
                CallType call = wait),
  &disableAudio(AudioChannel channel = all,
                const IMMMillisecondTime& over = IMMMillisecondTime(),
                CallType call = wait);
virtual unsigned long
  volume       (AudioChannel channel = left,
                CallType call = wait) const;
Boolean
  isAudioEnabled(AudioChannel channel = all,
                 CallType call = wait);

/*-------------------------------- Speed and Time ----------------------------*/
IMMSpeed::Format
  speedFormat     (CallType call = wait) const;
virtual IMMDevice
  &setSpeedFormat (IMMSpeed::Format format, CallType call = wait),
  &setTimeFormat  (IMMTime::Format format, CallType call = wait);
IMMTime::Format
  timeFormat      (CallType call = wait) const;

/*-------------------------------- Connector Type ----------------------------*/
enum ConnectorType
{
  midiStream = 0x00000001,
  cdStream   = 0x00000002,
  waveStream = 0x00000003,
  ampStream  = 0x00000004,
  xaStream   = 0x00000005,
  headphones = 0x00000006,
  speakers   = 0x00000007,
  microphones= 0x00000008,
  lineIn     = 0x00000009,
  lineOut    = 0x0000000a,
  videoIn    = 0x0000000b,
  videoOut   = 0x0000000c,
  phoneSet   = 0x0000000d,
  phoneLine  = 0x0000000e,
  audioIn    = 0x0000000f,
  audioOut   = 0x00000010,
  universal  = 0x00000011
};
virtual IMMDevice
  &enableConnector      (ConnectorType type,
                         Boolean enable=true,
                         CallType call = wait),
  &disableConnector     (ConnectorType type,
                         CallType call = wait);
Boolean
  isConnectionSupported (ConnectorType type,
                         CallType call = wait) const,
  isConnectorEnabled    (ConnectorType type,
                         CallType call = wait) const;
unsigned long
  connectedDeviceId     (ConnectorType type,
                         CallType call = wait) const;

/*---------------------------- Device Information ----------------------------*/
unsigned long
  deviceId        () const;
IString
  aliasName       () const,
  deviceName      () const;

/*----------------------- Notification Event Descriptions --------------------*/
static INotificationId const
  commandNotifyId,
  passDeviceId,
  cuePointId,
  positionChangeId,
  deviceEventId;

/*----------------------- Observer Notification ------------------------------*/
virtual IMMDevice
 &enableNotification (Boolean enabled = true);

protected:
/*----------------------------- Constructors ---------------------------------*/
  IMMDevice ( const IString& fileOrDevice );
  IMMDevice ( unsigned long deviceIdentifier,
              const IString& newAlias = IString());

/*------------------- Observer Notification Implementation -------------------*/
IMMDevice
 &setNotificationHandler ( IMMDeviceNotifyHandler* notifyHandler );

IMMDeviceNotifyHandler
 *notificationHandler    ( ) const;


/*-------------------------------- Open Status -------------------------------*/
enum OpenStatus
{
  openNotAttempted,
  openSuccessful,
  openFailed
};

OpenStatus
  openStatus     () const;
IMMDevice
  &setOpenStatus (OpenStatus status);

/*-------------------- Implementation ----------------------------------------*/
unsigned long
  itemStatus              (unsigned long item,
                           unsigned long value = 0,
                           CallType call = wait) const,
  itemCapability          (unsigned long item, CallType call) const;
IString
   sendCommand            (const IString& command),
   sendCommand            (const IString& command,unsigned short userParm);
IMMDevice
  &sendCommand            (unsigned short message, unsigned long param1,
                           void *param2),
  &sendCommand            (unsigned short message, unsigned long param1,
                           void *param2, unsigned short userParm),
  &setPassDeviceRequested (Boolean sawPassDevice),
  &setUserParameter       (unsigned short userParm = 0);
IMMMessageWindow
  &deviceWindow() const;
Boolean
  wasPassDeviceRequested  () const;
unsigned short
  userParameter           () const;

virtual Boolean
  isOpenStringValid       (const IString& deviceName) const;

virtual IMMDevice
  &open                   (unsigned long instanceNumber,
                           Boolean shareable = true,
                           CallType call = nowait),
  &openOnThread           (unsigned long instanceNumber,
                           Boolean shareable = true);

/*---------------------------- Error Information -----------------------------*/
unsigned long
  lastError      () const;
IMMDevice
  &setLastError  (unsigned long errorId);

private:
/*--------------------------------- Private ----------------------------------*/
void
  establishAlias (),
  releaseAlias   (const IString& alias);
IString
  uniqueAlias    ();

IMMDeviceData
  *fDeviceData;

friend class IMMDeviceHandler;
friend class IMMDeviceData;
friend class MMDeviceData; /****************** HELLO ***********************/
friend class QFMotionPlayer; /****************** HELLO ***********************/
}; // IMMDevice


#pragma pack()

#endif /* _IMMDEV_ */
