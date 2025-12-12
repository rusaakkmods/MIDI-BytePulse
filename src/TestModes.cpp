#include "TestModes.h"
#include "Sync.h"
#include "config.h"

void TestModes::setup(Sync* syncPtr) {
#if TEST_MODE_CLOCK
  pinMode(DISPLAY_CLK_PIN, OUTPUT);
  digitalWrite(DISPLAY_CLK_PIN, LOW);
  pinMode(LED_PULSE_PIN, OUTPUT);
  digitalWrite(LED_PULSE_PIN, LOW);
  
  pinMode(SYNC_RATE_PIN_1, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_2, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_3, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_4, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_5, INPUT_PULLUP);
  
  updateTestInterval();
  
  #if SERIAL_DEBUG
  DEBUG_PRINTLN("TEST MODE: Rotary switch controls BPM");
  DEBUG_PRINTLN("Pos1=30, Pos2=140, Pos3=230, Pos4=320, Pos5=400");
  #endif
#endif

#if TEST_MODE_SYNC_IN
  pinMode(SYNC_IN_PIN, OUTPUT);
  digitalWrite(SYNC_IN_PIN, LOW);
  
  pinMode(SYNC_RATE_PIN_1, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_2, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_3, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_4, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_5, INPUT_PULLUP);
  
  #if SERIAL_DEBUG
  DEBUG_PRINTLN("TEST MODE: Sync In test - Simulated Volca @ 2 PPQN, 120 BPM");
  DEBUG_PRINTLN("Switch position tells BytePulse how to INTERPRET the signal");
  DEBUG_PRINTLN("Correct = Pos2 (120 BPM), Wrong positions show incorrect BPM");
  #endif
#endif

#if TEST_MODE_MIDI_IN
  pinMode(SYNC_RATE_PIN_1, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_2, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_3, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_4, INPUT_PULLUP);
  pinMode(SYNC_RATE_PIN_5, INPUT_PULLUP);
  
  #if SERIAL_DEBUG
  DEBUG_PRINTLN("TEST MODE: MIDI In test - Simulated sequencer @ 24 PPQN, 120 BPM");
  DEBUG_PRINTLN("Rotary controls SYNC_OUT: Pos1=1PPQN, Pos2=2PPQN, Pos3=4PPQN, Pos5=24PPQN");
  DEBUG_PRINTLN("TinyPulse should always show 120 BPM from DISPLAY_CLK");
  #endif
  
  midiStartTime = millis();
#endif
}

void TestModes::loop(Sync* syncPtr) {
#if TEST_MODE_CLOCK
  unsigned long now = millis();
  
  updateTestInterval();
  
  if (currentTestInterval == 0) {
    digitalWrite(DISPLAY_CLK_PIN, LOW);
    digitalWrite(LED_PULSE_PIN, LOW);
    clockPulseActive = false;
    ledState = false;
    return;
  }
  
  if (now - lastTestPulse >= currentTestInterval) {
    lastTestPulse = now;
    
    digitalWrite(DISPLAY_CLK_PIN, HIGH);
    clockPulseActive = true;
    clockPulseStartTime = now;
    
    digitalWrite(LED_PULSE_PIN, HIGH);
    ledState = true;
    ledPulseStartTime = now;
  }
  
  if (clockPulseActive && (now - clockPulseStartTime >= 5)) {
    digitalWrite(DISPLAY_CLK_PIN, LOW);
    clockPulseActive = false;
  }
  
  if (ledState && (now - ledPulseStartTime >= LED_PULSE_WIDTH_MS)) {
    digitalWrite(LED_PULSE_PIN, LOW);
    ledState = false;
  }
#endif

#if TEST_MODE_SYNC_IN
  unsigned long now = millis();
  
  if (now - lastSyncInPulse >= 250) {  // VOLCA_INTERVAL = 250ms
    lastSyncInPulse = now;
    
    digitalWrite(SYNC_IN_PIN, HIGH);
    syncInPulseActive = true;
    syncInPulseStartTime = now;
    
    syncPtr->handleSyncInPulse();
  }
  
  if (syncInPulseActive && (now - syncInPulseStartTime >= 5)) {
    digitalWrite(SYNC_IN_PIN, LOW);
    syncInPulseActive = false;
  }
#endif

#if TEST_MODE_MIDI_IN
  unsigned long now = millis();
  
  if (!midiClockRunning && (now - midiStartTime >= 1000)) {
    syncPtr->handleStart(CLOCK_SOURCE_USB);
    midiClockRunning = true;
    midiClockCount = 0;
    lastMidiClock = now;
    
    #if SERIAL_DEBUG
    DEBUG_PRINTLN("MIDI Start sent");
    #endif
  }
  
  if (midiClockRunning) {
    unsigned long targetTime = lastMidiClock + ((midiClockCount * 60000UL) / (120 * 24));
    
    if (now >= targetTime) {
      midiClockCount++;
      syncPtr->handleClock(CLOCK_SOURCE_USB);
    }
  }
#endif
}

#if TEST_MODE_CLOCK
void TestModes::updateTestInterval() {
  if (digitalRead(SYNC_RATE_PIN_1) == LOW) {
    currentTestInterval = 2000;  // 30 BPM
  } else if (digitalRead(SYNC_RATE_PIN_2) == LOW) {
    currentTestInterval = 429;   // 140 BPM
  } else if (digitalRead(SYNC_RATE_PIN_3) == LOW) {
    currentTestInterval = 261;   // 230 BPM
  } else if (digitalRead(SYNC_RATE_PIN_4) == LOW) {
    currentTestInterval = 188;   // 320 BPM
  } else if (digitalRead(SYNC_RATE_PIN_5) == LOW) {
    currentTestInterval = 150;   // 400 BPM
  } else {
    currentTestInterval = 0;
  }
}
#endif
