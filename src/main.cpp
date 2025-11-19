/**
 * MIDI BytePulse - Pro Micro USB MIDI Sync Box
 * 
 * Universal MIDI clock follower/converter with dual clock sources (USB + DIN MIDI).
 * Features USB MIDI, DIN MIDI IN, analog clock sync output, and MIDI CC controls.
 * 
 * Hardware: SparkFun Pro Micro (ATmega32U4, 5V, 16MHz)
 * See documents/pro_micro_usb_midi_sync_box_breadboard_v_1.md for full specs
 */

#include <Arduino.h>
#include "config.h"
#include "MidiHandler.h"
#include "ClockSync.h"
#include "Controls.h"
#if ENABLE_DISPLAY
#include "Display.h"
#endif
#include "Settings.h"

// ============================================================================
// Global Module Instances
// ============================================================================

MidiHandler midiHandler;
ClockSync clockSync;
Controls controls;
#if ENABLE_DISPLAY
Display display;
#endif
Settings settings;

// ============================================================================
// Setup
// ============================================================================

void setup() {
  #if SERIAL_DEBUG
  Serial.begin(DEBUG_BAUD_RATE);
  delay(1000);  // Wait for serial connection
  DEBUG_PRINTLN("=== MIDI BytePulse v1.0 ===");
  DEBUG_PRINTLN("Universal MIDI Clock Converter");
  #endif
  
  #if MIDI_FORWARD_ONLY
  // MIDI Forward Only Mode: Initialize MIDI handler
  midiHandler.begin();
  DEBUG_PRINTLN("MIDI Forward Only Mode - Ready");
  return;
  #endif
  
  // Initialize settings from EEPROM
  settings.begin();
  
  #if !TEST_MODE
  // Initialize clock sync module
  clockSync.begin();
  clockSync.setPPQN(settings.getPPQN());
  
  // Initialize MIDI handler (USB + DIN) and link with clock sync
  midiHandler.begin();
  midiHandler.setClockSync(&clockSync);
  midiHandler.setClockSource(settings.getClockSource());
  
  // Initialize controls and link with MIDI handler
  controls.begin();
  controls.setMidiHandler(&midiHandler);
  #else
  // Test mode: Initialize MIDI handler (for CC messages) and controls
  midiHandler.begin();
  controls.begin();
  controls.setMidiHandler(&midiHandler);
  #endif
  
  #if ENABLE_DISPLAY
  // Initialize 7-segment display
  display.begin();
  display.showBPM(120); // Show default BPM
  #endif
  
  DEBUG_PRINTLN("System initialized");
  #if SERIAL_DEBUG
  DEBUG_PRINT("Clock source: ");
  ClockSource src = settings.getClockSource();
  DEBUG_PRINTLN(src == CLOCK_AUTO ? "AUTO" : (src == CLOCK_FORCE_USB ? "FORCE_USB" : "FORCE_DIN"));
  #endif
}

// ============================================================================
// Test Mode Loop
// ============================================================================

#if TEST_MODE
void testModeLoop() {
  static bool playWasPressed = false;
  static bool stopWasPressed = false;
  static bool isPlaying = false;
  static uint8_t panValue = 64;
  static bool encoderButtonWasPressed = false;
  
  controls.update();
  
  // Handle encoder for pan control (CC10)
  int8_t encoderDelta = controls.getEncoderDelta();
  if (encoderDelta != 0) {
    int16_t newPan = panValue + encoderDelta;
    if (newPan < 0) newPan = 0;
    if (newPan > 127) newPan = 127;
    
    panValue = (uint8_t)newPan;
    midiHandler.sendCC(CC_PAN, panValue);
    
    DEBUG_PRINT("Pan: ");
    DEBUG_PRINTLN(panValue);
  }
  
  // Handle encoder button for Note On/Off test
  bool encoderButtonPressed = controls.encoderPressed();
  if (encoderButtonPressed && !encoderButtonWasPressed) {
    midiHandler.sendNoteOn(48, 100);
    DEBUG_PRINTLN("Note On C3");
  } else if (!encoderButtonPressed && encoderButtonWasPressed) {
    midiHandler.sendNoteOff(48, 0);
    DEBUG_PRINTLN("Note Off C3");
  }
  encoderButtonWasPressed = encoderButtonPressed;
  
  // Transport buttons
  bool playPressed = controls.playPressed();
  bool stopPressed = controls.stopPressed();
  
  if (playPressed && !playWasPressed) {
    if (isPlaying) {
      midiHandler.sendStop();
      isPlaying = false;
    } else {
      midiHandler.sendStart();
      isPlaying = true;
    }
  }
  playWasPressed = playPressed;
  
  if (stopPressed && !stopWasPressed) {
    midiHandler.sendStop();
    isPlaying = false;
  }
  stopWasPressed = stopPressed;
}
#endif

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
  #if MIDI_FORWARD_ONLY
  // MIDI Forward Only Mode: Use MIDI Library for efficient parsing
  midiHandler.update();
  return;
  #endif
  
  #if TEST_MODE
  testModeLoop();
  return;
  #endif
  
  // Update MIDI handler first for low latency
  midiHandler.update();
  
  // Update clock sync
  clockSync.update();
  
  // Update controls periodically
  static unsigned long lastControlUpdate = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastControlUpdate >= 5) {  // Every 5ms
    controls.update();
    lastControlUpdate = currentTime;
  }
  
  #if ENABLE_DISPLAY
  // Update BPM display every second
  static unsigned long lastDisplayUpdate = 0;
  if (currentTime - lastDisplayUpdate >= 1000) {
    display.showBPM(midiHandler.getBPM());
    lastDisplayUpdate = currentTime;
  }
  #endif
  
  // Process MIDI again to catch any messages during other processing
  midiHandler.update();
  
  // Handle transport buttons
  static bool playWasPressed = false;
  static bool stopWasPressed = false;
  static bool localIsPlaying = false;
  
  bool playPressed = controls.playPressed();
  bool stopPressed = controls.stopPressed();
  
  // Play/Pause button
  if (playPressed && !playWasPressed) {
    if (localIsPlaying) {
      midiHandler.sendStop();
      localIsPlaying = false;
    } else {
      midiHandler.sendStart();
      localIsPlaying = true;
    }
  }
  playWasPressed = playPressed;
  
  // Stop button
  if (stopPressed && !stopWasPressed) {
    midiHandler.sendStop();
    localIsPlaying = false;
  }
  stopWasPressed = stopPressed;
}
