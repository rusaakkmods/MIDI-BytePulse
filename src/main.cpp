#include <Arduino.h>
#include <MIDIUSB.h>
#include "config.h"
#include "MIDIHandler.h"
#include "Sync.h"
#include "TestModes.h"

MIDIHandler midiHandler;
Sync sync;
TestModes testModes;

void syncInInterrupt() {
  sync.handleSyncInPulse();
}

void processUSBMIDI() {
  while (true) {
    midiEventPacket_t rx = MidiUSB.read();
    
    if (rx.header == 0) break;

    midiHandler.forwardUSBtoDIN(rx);

    if (rx.header == 0x0F) {
      switch (rx.byte1) {
        case 0xF8: 
          sync.handleClock(CLOCK_SOURCE_USB);
          break;
        case 0xFA:
          sync.handleStart(CLOCK_SOURCE_USB);
          break;
        case 0xFB: 
          sync.handleStart(CLOCK_SOURCE_USB);
          break;
        case 0xFC: 
          sync.handleStop(CLOCK_SOURCE_USB);
          break;
      }
    }
  }
}

void setup() {
  #if SERIAL_DEBUG
  Serial.begin(DEBUG_BAUD_RATE);
  while (!Serial && millis() < 3000);
  DEBUG_PRINTLN("MIDI BytePulse - Debug Mode");
  DEBUG_PRINTLN("BPM monitoring active (change threshold: >2 BPM)");
  DEBUG_PRINTLN("Checking Serial1 (MIDI DIN)...");
  DEBUG_PRINT("Serial1 TX Pin: ");
  DEBUG_PRINTLN(1);
  DEBUG_PRINT("Serial1 RX Pin: ");
  DEBUG_PRINTLN(0);
  #endif
  
  sync.begin();
  midiHandler.setSync(&sync);
  midiHandler.begin();
  
  testModes.setup(&sync);
  
  attachInterrupt(digitalPinToInterrupt(SYNC_IN_PIN), syncInInterrupt, RISING);
}

void loop() {
#if TEST_MODE_CLOCK || TEST_MODE_SYNC_IN || TEST_MODE_MIDI_IN
  testModes.loop(&sync);
  
  #if TEST_MODE_CLOCK
  return;
  #endif
#endif
  // Normal operation
  midiHandler.update();
  processUSBMIDI();
  sync.update();
  midiHandler.flushBuffer();
}
