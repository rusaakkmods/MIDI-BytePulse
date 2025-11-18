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
#include "Display.h"
#include "Settings.h"

// ============================================================================
// Global Module Instances
// ============================================================================

MidiHandler midiHandler;
ClockSync clockSync;
Controls controls;
Display display;
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
  
  // Initialize settings from EEPROM
  settings.begin();
  
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
  
  // Initialize display and link with other modules
  display.begin();
  display.setMidiHandler(&midiHandler);
  display.setClockSync(&clockSync);
  display.setControls(&controls);
  
  DEBUG_PRINTLN("System initialized");
  DEBUG_PRINT("Clock source: ");
  ClockSource src = settings.getClockSource();
  DEBUG_PRINTLN(src == CLOCK_AUTO ? "AUTO" : (src == CLOCK_FORCE_USB ? "FORCE_USB" : "FORCE_DIN"));
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
  // Update all modules
  midiHandler.update();  // Handles both USB and DIN MIDI
  clockSync.update();
  controls.update();
  display.update();
  
  // Handle transport buttons
  static bool playWasPressed = false;
  static bool stopWasPressed = false;
  
  bool playPressed = controls.playPressed();
  bool stopPressed = controls.stopPressed();
  
  // Play/Pause button (toggle behavior)
  if (playPressed && !playWasPressed) {
    if (midiHandler.isPlaying()) {
      midiHandler.sendStop();
    } else {
      if (midiHandler.getClockCount() == 0) {
        midiHandler.sendStart();
      } else {
        midiHandler.sendContinue();
      }
    }
  }
  playWasPressed = playPressed;
  
  // Stop button
  if (stopPressed && !stopWasPressed) {
    midiHandler.sendStop();
  }
  stopWasPressed = stopPressed;
  
  // Function button - enter/exit menu
  static bool functionWasPressed = false;
  bool functionPressed = controls.functionPressed();
  
  if (functionPressed && !functionWasPressed) {
    if (display.isInMenu()) {
      display.exitMenu();
    } else {
      display.enterMenu();
    }
  }
  functionWasPressed = functionPressed;
  
  // Handle encoder input
  int8_t encoderDelta = controls.getEncoderDelta();
  if (encoderDelta != 0 && display.isInMenu()) {
    display.handleEncoderDelta(encoderDelta);
  }
  
  // Handle encoder button press
  static bool encoderWasPressed = false;
  bool encoderPressed = controls.encoderPressed();
  
  if (encoderPressed && !encoderWasPressed) {
    if (display.isInMenu()) {
      display.handleEncoderPress();
      
      // Save settings if changed
      bool settingsChanged = false;
      
      if (clockSync.getPPQN() != settings.getPPQN()) {
        settings.setPPQN(clockSync.getPPQN());
        settingsChanged = true;
      }
      
      if (midiHandler.getClockSource() != settings.getClockSource()) {
        settings.setClockSource(midiHandler.getClockSource());
        settingsChanged = true;
      }
      
      if (settingsChanged) {
        settings.save();
        DEBUG_PRINTLN("Settings saved to EEPROM");
      }
    }
  }
  encoderWasPressed = encoderPressed;
}
