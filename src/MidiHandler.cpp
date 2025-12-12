#include "MIDIHandler.h"
#include "Sync.h"
#include <MIDI.h>
#include <MIDIUSB.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI_DIN);

Sync* MIDIHandler::sync = nullptr;

void MIDIHandler::sendMessage(const midiEventPacket_t& event) {
  MidiUSB.sendMIDI(event);
}

void MIDIHandler::flushBuffer() {
  MidiUSB.flush();
}

void MIDIHandler::begin() {
  MIDI_DIN.begin(MIDI_CHANNEL_OMNI);
  #if FORWARD_MIDI_IN_TO_MIDI_OUT == false
  // MIDI IN not forwarded to USB MIDI OUT (only mirrored to MIDI THRU PORT)
  MIDI_DIN.turnThruOff();
  #endif
  #if SERIAL_DEBUG
  DEBUG_PRINTLN("MIDI DIN initialized on Serial1");
  DEBUG_PRINTLN("Listening on all channels (OMNI mode)");
  #endif
  
  MIDI_DIN.setHandleNoteOn(handleNoteOn);
  MIDI_DIN.setHandleNoteOff(handleNoteOff);
  MIDI_DIN.setHandleAfterTouchPoly(handleAfterTouchPoly);
  MIDI_DIN.setHandleControlChange(handleControlChange);
  MIDI_DIN.setHandleProgramChange(handleProgramChange);
  MIDI_DIN.setHandleAfterTouchChannel(handleAfterTouchChannel);
  MIDI_DIN.setHandlePitchBend(handlePitchBend);
  MIDI_DIN.setHandleSystemExclusive(handleSystemExclusive);
  MIDI_DIN.setHandleClock(handleClock);
  MIDI_DIN.setHandleStart(handleStart);
  MIDI_DIN.setHandleContinue(handleContinue);
  MIDI_DIN.setHandleStop(handleStop);
  MIDI_DIN.setHandleActiveSensing(handleActiveSensing);
  MIDI_DIN.setHandleSystemReset(handleSystemReset);
}

void MIDIHandler::update() {
  #if SERIAL_DEBUG
  // Check for raw Serial1 data
  if (Serial1.available()) {
    static unsigned long lastRawDebug = 0;
    if (millis() - lastRawDebug > 500) {
      DEBUG_PRINT("Raw Serial1 bytes available: ");
      DEBUG_PRINTLN(Serial1.available());
      lastRawDebug = millis();
    }
  }
  #endif
  
  MIDI_DIN.read();
}

void MIDIHandler::setSync(Sync* s) {
  sync = s;
}

void MIDIHandler::forwardDINtoUSB(byte channel, byte type, byte data1, byte data2) {
  midiEventPacket_t event;
  event.header = type >> 4;
  event.byte1 = type | (channel - 1);
  event.byte2 = data1;
  event.byte3 = data2;
  
  sendMessage(event);
}

void MIDIHandler::forwardUSBtoDIN(const midiEventPacket_t& event) {
  byte status = event.byte1;
  byte type = status & 0xF0;
  byte channel = (status & 0x0F) + 1;
  
  switch (type) {
    case 0x80:
      MIDI_DIN.sendNoteOff(event.byte2, event.byte3, channel);
      break;
    case 0x90:
      MIDI_DIN.sendNoteOn(event.byte2, event.byte3, channel);
      break;
    case 0xA0:
      MIDI_DIN.sendAfterTouch(event.byte2, event.byte3, channel);
      break;
    case 0xB0:
      MIDI_DIN.sendControlChange(event.byte2, event.byte3, channel);
      break;
    case 0xC0:
      MIDI_DIN.sendProgramChange(event.byte2, channel);
      break;
    case 0xD0:
      MIDI_DIN.sendAfterTouch(event.byte2, channel);
      break;
    case 0xE0: {
      int pitchBend = event.byte2 | (event.byte3 << 7);
      MIDI_DIN.sendPitchBend(pitchBend, channel);
      break;
    }
  }
}

void MIDIHandler::handleNoteOn(byte channel, byte note, byte velocity) {
  #if SERIAL_DEBUG
  DEBUG_PRINT("DIN MIDI: Note On - Ch:");
  DEBUG_PRINT(channel);
  DEBUG_PRINT(" Note:");
  DEBUG_PRINT(note);
  DEBUG_PRINT(" Vel:");
  DEBUG_PRINTLN(velocity);
  #endif
  
  forwardDINtoUSB(channel, 0x90, note, velocity);
}

void MIDIHandler::handleNoteOff(byte channel, byte note, byte velocity) {
  forwardDINtoUSB(channel, 0x80, note, velocity);
}

void MIDIHandler::handleAfterTouchPoly(byte channel, byte note, byte pressure) {
  forwardDINtoUSB(channel, 0xA0, note, pressure);
}

void MIDIHandler::handleControlChange(byte channel, byte controller, byte value) {
  forwardDINtoUSB(channel, 0xB0, controller, value);
}

void MIDIHandler::handleProgramChange(byte channel, byte program) {
  forwardDINtoUSB(channel, 0xC0, program, 0);
}

void MIDIHandler::handleAfterTouchChannel(byte channel, byte pressure) {
  forwardDINtoUSB(channel, 0xD0, pressure, 0);
}

void MIDIHandler::handlePitchBend(byte channel, int bend) {
  byte lsb = bend & 0x7F;
  byte msb = (bend >> 7) & 0x7F;
  forwardDINtoUSB(channel, 0xE0, lsb, msb);
}

void MIDIHandler::handleSystemExclusive(byte* data, unsigned size) {
  midiEventPacket_t event;
  event.header = 0x04;
  
  for (unsigned i = 0; i < size; i += 3) {
    event.byte1 = (i < size) ? data[i] : 0;
    event.byte2 = (i + 1 < size) ? data[i + 1] : 0;
    event.byte3 = (i + 2 < size) ? data[i + 2] : 0;
    
    if (i + 3 >= size) {
      if (i + 1 >= size) event.header = 0x05;
      else if (i + 2 >= size) event.header = 0x06;
      else event.header = 0x07;
    }
    
    sendMessage(event);
  }
  MidiUSB.flush();
}

void MIDIHandler::handleClock() {
  midiEventPacket_t event = {0x0F, 0xF8, 0, 0};
  MidiUSB.sendMIDI(event);
  
  MIDI_DIN.sendRealTime(midi::Clock);
  
  static uint8_t clockCounter = 0;
  if (++clockCounter >= 6) {
    clockCounter = 0;
  }
  
  if (sync) {
    sync->handleClock(CLOCK_SOURCE_DIN);
  }
}

void MIDIHandler::handleStart() {
  midiEventPacket_t event = {0x0F, 0xFA, 0, 0};
  MidiUSB.sendMIDI(event);
  
  MIDI_DIN.sendRealTime(midi::Start);
  
  if (sync) {
    sync->handleStart(CLOCK_SOURCE_DIN);
  }
}

void MIDIHandler::handleContinue() {
  midiEventPacket_t event = {0x0F, 0xFB, 0, 0};
  MidiUSB.sendMIDI(event);
  
  MIDI_DIN.sendRealTime(midi::Continue);
  
  if (sync) {
    sync->handleStart(CLOCK_SOURCE_DIN);
  }
}

void MIDIHandler::handleStop() {
  midiEventPacket_t event = {0x0F, 0xFC, 0, 0};
  MidiUSB.sendMIDI(event);
  
  MIDI_DIN.sendRealTime(midi::Stop);
  
  if (sync) {
    sync->handleStop(CLOCK_SOURCE_DIN);
  }
}

void MIDIHandler::handleActiveSensing() {
  midiEventPacket_t event = {0x0F, 0xFE, 0, 0};
  MidiUSB.sendMIDI(event);
}

void MIDIHandler::handleSystemReset() {
  midiEventPacket_t event = {0x0F, 0xFF, 0, 0};
  MidiUSB.sendMIDI(event);
}
