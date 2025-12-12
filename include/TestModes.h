#ifndef TEST_MODES_H
#define TEST_MODES_H

#include <Arduino.h>
#include "config.h"

class Sync;

class TestModes {
public:
  void setup(Sync* syncPtr);
  void loop(Sync* syncPtr);

private:
#if TEST_MODE_CLOCK
  unsigned long lastTestPulse = 0;
  unsigned long currentTestInterval = 500;
  unsigned long clockPulseStartTime = 0;
  bool clockPulseActive = false;
  unsigned long ledPulseStartTime = 0;
  bool ledState = false;
  
  void updateTestInterval();
#endif

#if TEST_MODE_SYNC_IN
  unsigned long lastSyncInPulse = 0;
  unsigned long syncInPulseStartTime = 0;
  bool syncInPulseActive = false;
#endif

#if TEST_MODE_MIDI_IN
  unsigned long lastMidiClock = 0;
  unsigned long midiClockCount = 0;
  bool midiClockRunning = false;
  unsigned long midiStartTime = 0;
#endif
};

#endif // TEST_MODES_H
