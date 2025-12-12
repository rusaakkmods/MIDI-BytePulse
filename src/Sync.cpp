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
        
        if (onClockStop) {
          onClockStop();
        }
      }
    }
  }
  
  checkUSBTimeout();
  
  if (currentMillis - lastSwitchReadTime >= 100) {
    SyncInRate newRate = readSyncInRate();
    if (newRate != syncRate) {
      syncRate = newRate;
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
  if (digitalRead(SYNC_RATE_PIN_1) == LOW) return SYNC_IN_1_PPQN;
  if (digitalRead(SYNC_RATE_PIN_2) == LOW) return SYNC_IN_2_PPQN;
  if (digitalRead(SYNC_RATE_PIN_3) == LOW) return SYNC_IN_4_PPQN;
  if (digitalRead(SYNC_RATE_PIN_4) == LOW) return SYNC_IN_6_PPQN;
  if (digitalRead(SYNC_RATE_PIN_5) == LOW) return SYNC_IN_24_PPQN;
  return SYNC_IN_2_PPQN;
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
