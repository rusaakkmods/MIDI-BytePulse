/**
 * MIDI BytePulse - Sync Output Implementation
 */

#include "SyncOut.h"
#include "config.h"

#define PULSE_WIDTH_US 5000   // 5ms pulse width for faster response
#define PPQN 24               // Pulses per quarter note

void SyncOut::begin() {
  pinMode(CLOCK_OUT_PIN, OUTPUT);
  pinMode(LED_BEAT_PIN, OUTPUT);
  digitalWrite(CLOCK_OUT_PIN, LOW);
  digitalWrite(LED_BEAT_PIN, LOW);
  
  ppqnCounter = 0;
  isPlaying = false;
  clockState = false;
  ledState = false;
}

void SyncOut::handleClock() {
  if (!isPlaying) return;
  
  // Immediate pulse output - no processing delay
  digitalWrite(CLOCK_OUT_PIN, HIGH);
  clockState = true;
  lastPulseTime = micros();
  
  // Beat LED pulses every quarter note (every 24 clocks)
  if (ppqnCounter == 0) {
    digitalWrite(LED_BEAT_PIN, HIGH);
    ledState = true;
  }
  
  ppqnCounter++;
  if (ppqnCounter >= PPQN) {
    ppqnCounter = 0;
  }
}

void SyncOut::handleStart() {
  isPlaying = true;
  ppqnCounter = 0;
  
  // Immediate outputs
  digitalWrite(CLOCK_OUT_PIN, HIGH);
  digitalWrite(LED_BEAT_PIN, HIGH);
  clockState = true;
  ledState = true;
  lastPulseTime = micros();
}

void SyncOut::handleStop() {
  isPlaying = false;
  ppqnCounter = 0;
  
  // Immediate turn off
  digitalWrite(CLOCK_OUT_PIN, LOW);
  digitalWrite(LED_BEAT_PIN, LOW);
  clockState = false;
  ledState = false;
}

void SyncOut::update() {
  unsigned long currentTime = micros();
  
  // Turn off clock pulse after pulse width
  if (clockState && (currentTime - lastPulseTime >= PULSE_WIDTH_US)) {
    digitalWrite(CLOCK_OUT_PIN, LOW);
    clockState = false;
  }
  
  // Turn off LED pulse after pulse width
  if (ledState && (currentTime - lastPulseTime >= PULSE_WIDTH_US)) {
    digitalWrite(LED_BEAT_PIN, LOW);
    ledState = false;
  }
}

void SyncOut::pulseClock() {
  digitalWrite(CLOCK_OUT_PIN, HIGH);
  clockState = true;
  lastPulseTime = micros();
}

void SyncOut::pulseLED() {
  digitalWrite(LED_BEAT_PIN, HIGH);
  ledState = true;
}
