#include "Sync.h"
#include "config.h"
#include <MIDIUSB.h>
#include <MIDI.h>

extern midi::MidiInterface<midi::SerialMIDI<HardwareSerial>> MIDI_DIN;

#define CLOCK_PULSE_WIDTH_US 5000  // SYNC_OUT pulse width (5ms)
#define LED_PULSE_WIDTH_MS 50      // Pulse LED width (50ms) - safe up to 400 BPM
#define PPQN 24

void Sync::begin() {
  pinMode(SYNC_OUT_PIN, OUTPUT);
  pinMode(DISPLAY_CLK_PIN, OUTPUT);
  pinMode(SYNC_IN_PIN, INPUT_PULLUP);
  pinMode(SYNC_IN_DETECT_PIN, INPUT_PULLUP);
  pinMode(LED_PULSE_PIN, OUTPUT);
  
  // Rotary switch pins (with pullups) - one per position
  pinMode(SYNC_RATE_PIN_1, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_2, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_3, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_4, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_5, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_6, INPUT_PULLUP);
  
  // Read sync rate from rotary switch (controls both SYNC_IN and SYNC_OUT)
  syncRate = readSyncInRate();
  
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
  if (!isSyncInConnected()) return;
  syncInPulseTime = millis();
}

void Sync::handleClock(ClockSource source) {
  unsigned long now = millis();
  
  if (source == CLOCK_SOURCE_DIN && (activeSource == CLOCK_SOURCE_USB || activeSource == CLOCK_SOURCE_SYNC_IN)) {
    return;
  }
  if (source == CLOCK_SOURCE_USB && activeSource == CLOCK_SOURCE_SYNC_IN) {
    return;
  }
  
  if (source == CLOCK_SOURCE_USB && usbIsPlaying) {
    lastUSBClockTime = now;
    activeSource = CLOCK_SOURCE_USB;
  }
  
  if (source == CLOCK_SOURCE_DIN && activeSource != CLOCK_SOURCE_USB) {
    lastDINClockTime = now;
    activeSource = CLOCK_SOURCE_DIN;
  }
  
  if (source == CLOCK_SOURCE_USB && !usbIsPlaying) {
    usbIsPlaying = true;
    isPlaying = true;
    activeSource = CLOCK_SOURCE_USB;
    ppqnCounter = 0;
    lastUSBClockTime = millis();
  }
  
  if (source == CLOCK_SOURCE_DIN && activeSource == CLOCK_SOURCE_USB) {
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && activeSource != CLOCK_SOURCE_USB) {
    if (!isPlaying) {
      isPlaying = true;
      activeSource = CLOCK_SOURCE_DIN;
      ppqnCounter = 0;
      lastDINClockTime = millis();
    }
  }
  
  if (!isPlaying) return;
  
  // DISPLAY_CLK: Always output at 1 PPQN for TinyPulse Display (once per beat)
  if (ppqnCounter == 0) {
    digitalWrite(DISPLAY_CLK_PIN, HIGH);
    displayClkState = true;
    displayClkPulseTime = millis();
  }
  
  // SYNC_OUT: Output pulse at selected PPQN rate (configurable via switch)
  // Use modulo to divide 24 PPQN down to target rate
  uint8_t divisor = getSyncOutDivisor();
  if (ppqnCounter % divisor == 0) {
    unsigned long now = millis();
    
    // Send SYNC_OUT pulse at configured rate
    digitalWrite(SYNC_OUT_PIN, HIGH);
    clockState = true;
    lastPulseTime = micros();
    
    if (!ledState) {
      digitalWrite(LED_PULSE_PIN, HIGH);
      ledState = true;
      lastPulseTime = now;
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
    
    MIDI_DIN.sendRealTime(midi::Start);
    
    if (onClockStart) {
      onClockStart();
    }
    
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && usbIsPlaying) {
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && !usbIsPlaying) {
    activeSource = CLOCK_SOURCE_DIN;
    isPlaying = true;
    ppqnCounter = 0;
    
    MIDI_DIN.sendRealTime(midi::Start);
    
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
    
    MIDI_DIN.sendRealTime(midi::Stop);
    
    if (onClockStop) {
      onClockStop();
    }
    
    return;
  }
  
  if (source == CLOCK_SOURCE_DIN && usbIsPlaying) {
    return;
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
    
    MIDI_DIN.sendRealTime(midi::Stop);
    
    if (onClockStop) {
      onClockStop();
    }
  }
}

void Sync::update() {
  unsigned long currentTime = micros();
  
  if (syncInPulseTime > 0) {
    unsigned long pulseTime = syncInPulseTime;
    syncInPulseTime = 0;
    
    if (!syncInIsPlaying) {
      syncInIsPlaying = true;
      isPlaying = true;
      activeSource = CLOCK_SOURCE_SYNC_IN;
      ppqnCounter = 0;
      lastSyncInTime = pulseTime;
      
      if (onClockStart) {
        onClockStart();
      }
    }
    
    // Send multiple MIDI Clocks based on SYNC_IN rate
    // E.g., if SYNC_IN is 2 PPQN, send 12 MIDI Clocks per pulse (24/2=12)
    uint8_t multiplier = getSyncInMultiplier();
    for (uint8_t i = 0; i < multiplier; i++) {
      sendMIDIClock();
    }
    
    if (syncInIsPlaying) {
      // DISPLAY_CLK: Always 1 PPQN for TinyPulse
      digitalWrite(DISPLAY_CLK_PIN, HIGH);
      displayClkState = true;
      
      // SYNC_OUT: Variable PPQN based on switch
      digitalWrite(SYNC_OUT_PIN, HIGH);
      clockState = true;
      lastPulseTime = currentTime;
      
      digitalWrite(LED_PULSE_PIN, HIGH);
      ledState = true;
    }
  }
  
  if (syncInIsPlaying) {
    if (!isSyncInConnected()) {
      syncInIsPlaying = false;
      if (activeSource == CLOCK_SOURCE_SYNC_IN) {
        activeSource = CLOCK_SOURCE_NONE;
        isPlaying = false;
        ppqnCounter = 0;
        
        if (onClockStop) {
          onClockStop();
        }
      }
    }
    else if ((millis() - lastSyncInTime) > 3000) {  // 3 second timeout
      syncInIsPlaying = false;
      if (activeSource == CLOCK_SOURCE_SYNC_IN) {
        activeSource = CLOCK_SOURCE_NONE;
        isPlaying = false;
        ppqnCounter = 0;
        
        if (onClockStop) {
          onClockStop();
        }
      }
    }
  }
  
  checkUSBTimeout();
  
  // Turn off SYNC_OUT pulse after 5ms
  if (clockState && (currentTime - lastPulseTime >= CLOCK_PULSE_WIDTH_US)) {
    digitalWrite(SYNC_OUT_PIN, LOW);
    clockState = false;
  }
  
  // Turn off DISPLAY_CLK pulse after 5ms
  unsigned long currentMillis = millis();
  if (displayClkState && (currentMillis - displayClkPulseTime >= 5)) {
    digitalWrite(DISPLAY_CLK_PIN, LOW);
    displayClkState = false;
  }
  
  // Pulse LED (fixed 50ms width)
  if (ledState && (currentMillis - lastPulseTime >= LED_PULSE_WIDTH_MS)) {
    digitalWrite(LED_PULSE_PIN, LOW);
    ledState = false;
  }
}

void Sync::checkUSBTimeout() {
  if (!usbIsPlaying) return;
  
  unsigned long now = millis();
  if ((now - lastUSBClockTime) > 3000) {  // 3 second timeout
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
  // Simple direct reading - one pin per position, no encoding
  // Delay a bit for mechanical settling, then read
  delay(5);
  
  // Check each pin - whichever is LOW (connected to GND) determines rate
  if (digitalRead(SYNC_RATE_PIN_1) == LOW) return SYNC_IN_1_PPQN;
  if (digitalRead(SYNC_RATE_PIN_2) == LOW) return SYNC_IN_2_PPQN;
  if (digitalRead(SYNC_RATE_PIN_3) == LOW) return SYNC_IN_4_PPQN;
  if (digitalRead(SYNC_RATE_PIN_4) == LOW) return SYNC_IN_6_PPQN;
  if (digitalRead(SYNC_RATE_PIN_5) == LOW) return SYNC_IN_24_PPQN;
  if (digitalRead(SYNC_RATE_PIN_6) == LOW) return SYNC_IN_48_PPQN;
  
  // Default if no pin is grounded (shouldn't happen with proper switch)
  return SYNC_IN_2_PPQN;
}

uint8_t Sync::getSyncInMultiplier() {
  // Calculate how many MIDI Clocks to send per SYNC_IN pulse
  // MIDI Clock is always 24 PPQN, so multiplier = 24 / syncRate
  return 24 / (uint8_t)syncRate;
}

uint8_t Sync::getSyncOutDivisor() {
  // Calculate SYNC_OUT divisor: how many MIDI Clocks per SYNC_OUT pulse
  // SYNC_OUT pulses at the selected PPQN rate
  // For example: if syncRate = 2 PPQN, output 1 pulse every 12 MIDI Clocks (24/2)
  return 24 / (uint8_t)syncRate;
}

void Sync::sendMIDIClock() {
  midiEventPacket_t clockEvent = {0x0F, 0xF8, 0, 0};
  MidiUSB.sendMIDI(clockEvent);
  MIDI_DIN.sendRealTime(midi::Clock);
}
