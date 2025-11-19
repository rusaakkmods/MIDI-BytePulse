/**
 * MIDI BytePulse - Transport Control Implementation
 */

#include "TransportControl.h"
#include "config.h"
#include <MIDIUSB.h>

#define DEBOUNCE_MS 50

void TransportControl::begin() {
  pinMode(BTN_PLAY_PIN, INPUT_PULLUP);
  pinMode(BTN_STOP_PIN, INPUT_PULLUP);
  state = TRANSPORT_STOP;
}

void TransportControl::update() {
  handlePlayButton();
  handleStopButton();
}

void TransportControl::handlePlayButton() {
  bool reading = digitalRead(BTN_PLAY_PIN);
  
  if (reading != lastPlayState) {
    lastPlayDebounce = millis();
  }
  
  if ((millis() - lastPlayDebounce) > DEBOUNCE_MS) {
    if (reading == LOW && !playPressed) {
      playPressed = true;
      switch (state) {
        case TRANSPORT_STOP:
          state = TRANSPORT_PLAY;
          sendStart();
          break;
        case TRANSPORT_PLAY:
          state = TRANSPORT_PAUSE;
          sendStop();
          break;
        case TRANSPORT_PAUSE:
          state = TRANSPORT_PLAY;
          sendContinue();
          break;
      }
    } else if (reading == HIGH) {
      playPressed = false;
    }
  }
  
  lastPlayState = reading;
}

void TransportControl::handleStopButton() {
  bool reading = digitalRead(BTN_STOP_PIN);
  
  if (reading != lastStopState) {
    lastStopDebounce = millis();
  }
  
  if ((millis() - lastStopDebounce) > DEBOUNCE_MS) {
    if (reading == LOW && !stopPressed) {
      stopPressed = true;
      state = TRANSPORT_STOP;
      sendStop();
    } else if (reading == HIGH) {
      stopPressed = false;
    }
  }
  
  lastStopState = reading;
}

void TransportControl::sendStart() {
  midiEventPacket_t event = {0x0F, 0xFA, 0, 0};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void TransportControl::sendContinue() {
  midiEventPacket_t event = {0x0F, 0xFB, 0, 0};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void TransportControl::sendStop() {
  midiEventPacket_t event = {0x0F, 0xFC, 0, 0};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}
