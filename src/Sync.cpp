#include "Sync.h"
#include "config.h"
#include <MIDIUSB.h>
#include <MIDI.h>

extern midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> MIDI_DIN;

void Sync::begin() {
  pinMode(SYNC_OUT_PIN, OUTPUT);
  pinMode(DISPLAY_CLK_PIN, OUTPUT);
  pinMode(SYNC_IN_PIN, INPUT_PULLUP);
  pinMode(SYNC_IN_DETECT_PIN, INPUT_PULLUP);
  pinMode(LED_PULSE_PIN, OUTPUT);
  
  pinMode(SYNC_RATE_PIN_1, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_2, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_3, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_4, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_5, INPUT_PULLUP);
  
  syncRate = readSyncInRate();
  lastSwitchReadTime = millis();
  
  digitalWrite(SYNC_OUT_PIN, LOW);
  digitalWrite(DISPLAY_CLK_PIN, LOW);
  digitalWrite(LED_PULSE_PIN, LOW);
  
  ppqnCounter = 0;
  isPlaying = false;
  usbIsPlaying = false;
  syncInIsPlaying = false;
  clockState = false;
  ledState = false;
  activeSource = CLOCK_SOURCE_NONE;
  lastUSBClockTime = 0;
}

void Sync::handleSyncInPulse() {
  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  
  // Debounce: ignore interrupts within 5ms of previous
  if (interruptTime - lastInterruptTime < 5) {
    return;
  }
  
  lastInterruptTime = interruptTime;
  
  if (!isSyncInConnected()) return;
  syncInPulseTime = interruptTime;
}

void Sync::handleClock(ClockSource source) {
  unsigned long now = millis();
  
  // Priority: SYNC_IN > USB > DIN
  // Reject lower priority sources completely when higher priority is active
  if (source == CLOCK_SOURCE_DIN && (activeSource == CLOCK_SOURCE_USB || activeSource == CLOCK_SOURCE_SYNC_IN)) {
    return;  // Ignore DIN if USB or SYNC_IN is active
  }
  if (source == CLOCK_SOURCE_USB && activeSource == CLOCK_SOURCE_SYNC_IN) {
    return;  // Ignore USB if SYNC_IN is active
  }
  
  // Handle first clock from each source
  if (source == CLOCK_SOURCE_USB && !usbIsPlaying) {
    usbIsPlaying = true;
    isPlaying = true;
    activeSource = CLOCK_SOURCE_USB;
    ppqnCounter = 0;
    lastUSBClockTime = now;
  }
  
  if (source == CLOCK_SOURCE_DIN && !isPlaying) {
    isPlaying = true;
    activeSource = CLOCK_SOURCE_DIN;
    ppqnCounter = 0;
    lastDINClockTime = now;
  }
  
  // USB can override DIN after it's started
  if (source == CLOCK_SOURCE_USB && activeSource == CLOCK_SOURCE_DIN) {
    activeSource = CLOCK_SOURCE_USB;
    usbIsPlaying = true;
    ppqnCounter = 0;  // Reset counter on source switch
    lastUSBClockTime = now;
  }
  
  // Update last clock time for active source
  if (source == CLOCK_SOURCE_USB && activeSource == CLOCK_SOURCE_USB) {
    lastUSBClockTime = now;
  }
  if (source == CLOCK_SOURCE_DIN && activeSource == CLOCK_SOURCE_DIN) {
    lastDINClockTime = now;
  }
  
  if (!isPlaying) return;
  
  // Forward clock to MIDI DIN OUT (only for USB/SYNC_IN, DIN already forwards itself)
  if (source == CLOCK_SOURCE_USB || source == CLOCK_SOURCE_SYNC_IN) {
    MIDI_DIN.sendRealTime(midi::Clock);
  }
  
  if (ppqnCounter == 0) {
    digitalWrite(DISPLAY_CLK_PIN, HIGH);
    displayClkState = true;
    displayClkPulseTime = millis();
  }
  
  uint8_t divisor = getSyncOutDivisor();
  if (ppqnCounter % divisor == 0) {
    unsigned long now = millis();
    
    digitalWrite(SYNC_OUT_PIN, HIGH);
    clockState = true;
    lastPulseTime = micros();
    
    if (!ledState) {
      digitalWrite(LED_PULSE_PIN, HIGH);
      ledState = true;
      ledPulseTime = now;
    }
  }
  
  ppqnCounter++;
  if (ppqnCounter >= PPQN) {
    ppqnCounter = 0;
  }
}

void Sync::handleStart(ClockSource source) {
  if (source == CLOCK_SOURCE_USB) {
    usbIsPlaying = true;
    activeSource = CLOCK_SOURCE_USB;
    lastUSBClockTime = millis();
    isPlaying = true;
    ppqnCounter = 0;
    
    // USB is master: forward Start to MIDI OUT
    MIDI_DIN.sendRealTime(midi::Start);
    
    if (onClockStart) {
      onClockStart();
    }
    
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && usbIsPlaying) {
    return;  // DIN blocked by USB
  }
  
  if (source == CLOCK_SOURCE_DIN && !usbIsPlaying) {
    activeSource = CLOCK_SOURCE_DIN;
    isPlaying = true;
    ppqnCounter = 0;
    
    // DIN is master: MidiHandler already forwarded to both USB and MIDI OUT
    
    if (onClockStart) {
      onClockStart();
    }
  }
}

void Sync::handleStop(ClockSource source) {
  if (source == CLOCK_SOURCE_USB) {
    usbIsPlaying = false;
    isPlaying = false;
    activeSource = CLOCK_SOURCE_NONE;
    ppqnCounter = 0;
    
    // USB is master: forward Stop to MIDI OUT
    MIDI_DIN.sendRealTime(midi::Stop);
    
    if (onClockStop) {
      onClockStop();
    }
    
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && usbIsPlaying) {
    return;  // DIN blocked by USB
  }
  
  if (source == CLOCK_SOURCE_DIN && !usbIsPlaying) {
    activeSource = CLOCK_SOURCE_NONE;
    isPlaying = false;
    ppqnCounter = 0;
    
    digitalWrite(SYNC_OUT_PIN, LOW);
    digitalWrite(DISPLAY_CLK_PIN, LOW);
    digitalWrite(LED_PULSE_PIN, LOW);
    clockState = false;
    displayClkState = false;
    ledState = false;
    
    // DIN is master: MidiHandler already forwarded to both USB and MIDI OUT
    
    if (onClockStop) {
      onClockStop();
    }
  }
}

void Sync::update() {
  unsigned long currentTime = micros();
  unsigned long currentMillis = millis();
  
  if (syncInPulseTime > 0) {
    unsigned long pulseTime = syncInPulseTime;
    syncInPulseTime = 0;
    
    if (!syncInIsPlaying) {
      syncInIsPlaying = true;
      isPlaying = true;
      activeSource = CLOCK_SOURCE_SYNC_IN;
      ppqnCounter = 0;
      lastSyncInTime = pulseTime;
      
      // SYNC_IN does NOT send Start message - only clocks
      
      if (onClockStart) {
        onClockStart();
      }
    }
    
    lastSyncInTime = pulseTime;  // Update for timeout detection
    
    uint8_t multiplier = getSyncInMultiplier();
    uint8_t divisor = getSyncOutDivisor();
    
    // Now send MIDI clocks and update counter
    // Pulse DISPLAY_CLK once per quarter note (when ppqnCounter wraps to 0)
    // Pulse SYNC_OUT based on divisor setting
    for (uint8_t i = 0; i < multiplier; i++) {
      sendMIDIClock();
      
      // Check if we should pulse SYNC_OUT based on divisor before incrementing
      if (ppqnCounter % divisor == 0) {
        digitalWrite(SYNC_OUT_PIN, HIGH);
        clockState = true;
        lastPulseTime = currentTime;
        
        if (!ledState) {
          digitalWrite(LED_PULSE_PIN, HIGH);
          ledState = true;
          ledPulseTime = currentMillis;
        }
      }
      
      ppqnCounter++;
      if (ppqnCounter >= PPQN) {
        ppqnCounter = 0;
        // Pulse DISPLAY_CLK once per beat using actual SYNC_IN pulse arrival time
        digitalWrite(DISPLAY_CLK_PIN, HIGH);
        displayClkState = true;
        displayClkPulseTime = pulseTime;
      }
    }
  }
  
  if (syncInIsPlaying) {
    if (!isSyncInConnected()) {
      syncInIsPlaying = false;
      if (activeSource == CLOCK_SOURCE_SYNC_IN) {
        activeSource = CLOCK_SOURCE_NONE;
        isPlaying = false;
        ppqnCounter = 0;
        
        // SYNC_IN does NOT send Stop message - only stops clocks
        
        if (onClockStop) {
          onClockStop();
        }
      }
    }
    else if ((millis() - lastSyncInTime) > 3000) {
      syncInIsPlaying = false;
      if (activeSource == CLOCK_SOURCE_SYNC_IN) {
        activeSource = CLOCK_SOURCE_NONE;
        isPlaying = false;
        ppqnCounter = 0;
        
        // SYNC_IN does NOT send Stop message - only stops clocks;
        
        if (onClockStop) {
          onClockStop();
        }
      }
    }
  }
  
  checkUSBTimeout();
  
  // Debounced switch reading - require 3 consecutive stable readings
  if (currentMillis - lastSwitchReadTime >= 50) {
    SyncInRate newRate = readSyncInRate();
    
    static SyncInRate pendingRate = SYNC_IN_2_PPQN;
    static uint8_t stableCount = 0;
    
    if (newRate == pendingRate) {
      stableCount++;
      if (stableCount >= 3 && newRate != syncRate) {
        syncRate = newRate;
        stableCount = 0;
      }
    } else {
      pendingRate = newRate;
      stableCount = 1;
    }
    
    lastSwitchReadTime = currentMillis;
  }
  
  if (clockState && (currentTime - lastPulseTime >= CLOCK_PULSE_WIDTH_US)) {
    digitalWrite(SYNC_OUT_PIN, LOW);
    clockState = false;
  }
  
  if (displayClkState && (currentMillis - displayClkPulseTime >= 5)) {
    digitalWrite(DISPLAY_CLK_PIN, LOW);
    displayClkState = false;
  }
  
  if (ledState && (currentMillis - ledPulseTime >= LED_PULSE_WIDTH_MS)) {
    digitalWrite(LED_PULSE_PIN, LOW);
    ledState = false;
  }
}

void Sync::checkUSBTimeout() {
  if (!usbIsPlaying) return;
  
  unsigned long now = millis();
  if ((now - lastUSBClockTime) > 3000) {
    usbIsPlaying = false;
    isPlaying = false;
    activeSource = CLOCK_SOURCE_NONE;
    ppqnCounter = 0;
    
    if (onClockStop) {
      onClockStop();
    }
  }
}

bool Sync::isSyncInConnected() {
  return digitalRead(SYNC_IN_DETECT_PIN) == HIGH;
}

SyncInRate Sync::readSyncInRate() {
  // Read all pins
  bool pin1 = (digitalRead(SYNC_RATE_PIN_1) == LOW);
  bool pin2 = (digitalRead(SYNC_RATE_PIN_2) == LOW);
  bool pin3 = (digitalRead(SYNC_RATE_PIN_3) == LOW);
  bool pin4 = (digitalRead(SYNC_RATE_PIN_4) == LOW);
  bool pin5 = (digitalRead(SYNC_RATE_PIN_5) == LOW);
  
  // Count how many pins are active
  uint8_t activeCount = pin1 + pin2 + pin3 + pin4 + pin5;
  
  #if SERIAL_DEBUG
  static unsigned long lastDebugTime = 0;
  if (millis() - lastDebugTime > 2000) {
    DEBUG_PRINT("Switch: P1=");
    DEBUG_PRINT(pin1);
    DEBUG_PRINT(" P2=");
    DEBUG_PRINT(pin2);
    DEBUG_PRINT(" P3=");
    DEBUG_PRINT(pin3);
    DEBUG_PRINT(" P4=");
    DEBUG_PRINT(pin4);
    DEBUG_PRINT(" P5=");
    DEBUG_PRINT(pin5);
    DEBUG_PRINT(" Active=");
    DEBUG_PRINT(activeCount);
    DEBUG_PRINT(" Rate=");
    DEBUG_PRINTLN((int)syncRate);
    lastDebugTime = millis();
  }
  #endif
  
  // If no pins active, keep current setting
  if (activeCount == 0) {
    return syncRate;
  }
  
  // Special case: If P1 and P2 are both active (hardware issue), treat as position 1
  if (pin1 && pin2 && !pin3 && !pin4 && !pin5) {
    return SYNC_IN_1_PPQN;
  }
  
  // Priority order: 5, 4, 3, 2, 1 (highest value wins if multiple active)
  if (pin5) return SYNC_IN_24_PPQN;
  if (pin4) return SYNC_IN_6_PPQN;
  if (pin3) return SYNC_IN_4_PPQN;
  if (pin2) return SYNC_IN_2_PPQN;
  if (pin1) return SYNC_IN_1_PPQN;
  
  return syncRate;
}

uint8_t Sync::getSyncInMultiplier() {
  return 24 / (uint8_t)syncRate;
}

uint8_t Sync::getSyncOutDivisor() {
  return 24 / (uint8_t)syncRate;
}

void Sync::sendMIDIClock() {
  midiEventPacket_t clockEvent = {0x0F, 0xF8, 0, 0};
  MidiUSB.sendMIDI(clockEvent);
  MIDI_DIN.sendRealTime(midi::Clock);
}
