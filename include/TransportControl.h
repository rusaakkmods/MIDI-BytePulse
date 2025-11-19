/**
 * MIDI BytePulse - Transport Control
 */

#ifndef TRANSPORT_CONTROL_H
#define TRANSPORT_CONTROL_H

#include <Arduino.h>

enum TransportState {
  TRANSPORT_STOP,
  TRANSPORT_PLAY,
  TRANSPORT_PAUSE
};

class TransportControl {
public:
  void begin();
  void update();
  TransportState getState() { return state; }

private:
  void handlePlayButton();
  void handleStopButton();
  void sendStart();
  void sendContinue();
  void sendStop();
  
  TransportState state = TRANSPORT_STOP;
  bool lastPlayState = HIGH;
  bool lastStopState = HIGH;
  bool playPressed = false;
  bool stopPressed = false;
  unsigned long lastPlayDebounce = 0;
  unsigned long lastStopDebounce = 0;
};

#endif  // TRANSPORT_CONTROL_H
