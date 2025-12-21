#ifndef SYNC_H
#define SYNC_H

#include <Arduino.h>

enum ClockSource {
  CLOCK_SOURCE_NONE,
  CLOCK_SOURCE_SYNC_IN,
  CLOCK_SOURCE_DIN,
  CLOCK_SOURCE_USB
};

enum SyncInRate {
  SYNC_IN_1_PPQN = 1,
  SYNC_IN_2_PPQN = 2,
  SYNC_IN_4_PPQN = 4,
  SYNC_IN_6_PPQN = 6,
  SYNC_IN_24_PPQN = 24
};

class Sync {
public:
  void begin();
  void handleClock(ClockSource source);
  void handleStart(ClockSource source);
  void handleStop(ClockSource source);
  void handleSyncInPulse();
  void update();
  bool isBeatActive() const { return ledState; }
  bool isClockRunning() const { return isPlaying; }
  ClockSource getActiveSource() const { return activeSource; }
  
  SyncInRate readSyncInRate();     // Read rotary switch position
  uint8_t getSyncInMultiplier();   // Get PPQN multiplier for SYNC_IN → MIDI
  uint8_t getSyncOutDivisor();     // Get PPQN divisor for MIDI → SYNC_OUT
  
  void (*onClockStop)() = nullptr;
  void (*onClockStart)() = nullptr;

private:
  void checkUSBTimeout();
  bool isSyncInConnected();
  void sendMIDIClock();
  
  unsigned long lastPulseTime = 0;
  unsigned long ledPulseTime = 0;
  unsigned long lastUSBClockTime = 0;
  unsigned long lastDINClockTime = 0;
  unsigned long lastSyncInTime = 0;
  volatile unsigned long syncInPulseTime = 0;
  unsigned long syncOutPulseTime = 0;
  unsigned long displayClkPulseTime = 0;
  bool clockState = false;
  bool displayClkState = false;
  bool ledState = false;
  byte ppqnCounter = 0;
  bool isPlaying = false;
  bool usbIsPlaying = false;
  bool syncInIsPlaying = false;
  ClockSource activeSource = CLOCK_SOURCE_NONE;
  
  SyncInRate syncRate = SYNC_IN_2_PPQN;  // Switch setting (controls both IN and OUT)
  uint8_t syncInPulseCounter = 0;        // Counter for SYNC_IN PPQN multiplication
  uint8_t syncOutPulseCounter = 0;       // Counter for SYNC_OUT PPQN division
  unsigned long lastSwitchReadTime = 0;  // For non-blocking switch debouncing
};

#endif
