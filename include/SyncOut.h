/**
 * MIDI BytePulse - Sync Output Handler
 * Generates clock sync pulses and beat LED indicator
 */

#ifndef SYNC_OUT_H
#define SYNC_OUT_H

#include <Arduino.h>

class SyncOut {
public:
  void begin();
  void handleClock();
  void handleStart();
  void handleStop();
  void update();

private:
  void pulseClock();
  void pulseLED();
  
  unsigned long lastPulseTime = 0;
  bool clockState = false;
  bool ledState = false;
  byte ppqnCounter = 0;
  bool isPlaying = false;
};

#endif  // SYNC_OUT_H
