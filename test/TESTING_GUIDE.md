# rMODS MIDI BytePulse - Testing Guide

## 1. Overview

This document provides a systematic testing guide for the **rMODS MIDI BytePulse** device. It includes:
- **Unit Tests** - Automated tests for core logic (BPM calculation, clock priority, display formatting)
- **Integration Tests** - Manual hardware tests with real MIDI equipment
- **Edge Case Tests** - Stress tests and boundary condition validation

Testers should start with unit tests to verify core logic, then proceed to hardware integration testing.

---

## 2. Unit Tests (Automated)

### 2.1. Running Unit Tests

Unit tests run on your computer (not the device) and test core logic without hardware dependencies.

**Prerequisites:**
- PlatformIO installed (via VS Code or CLI)

**Run all unit tests:**
```bash
pio test -e native
```

**Run specific test:**
```bash
pio test -e native -f test_clock_priority
```

### 2.2. Available Unit Tests

#### Test Suite 1: Clock Source Priority (`test_clock_priority`)
Tests the priority logic: Sync In > USB > DIN.

**What it tests:**
- SYNC_IN blocks USB and DIN
- USB blocks DIN
- Fallback when higher priority stops
- Initial state accepts any source
- Complete priority chain

**Expected result:** All 7 tests pass

#### Test Suite 3: Display Formatting (`test_display_format`)
Tests 7-segment character conversion and BPM formatting.

**What it tests:**
- Digit conversion (0-9)
- Letter conversion (I, d, L, e for "IdLE")
- Special characters (-, _)
- BPM digit extraction (5, 30, 60, 120, 300, 999 BPM)
- Case insensitivity

**Expected result:** All 11 tests pass

### 2.3. Interpreting Unit Test Results
**Expected result:** All 7 tests pass

---

## 3. Device Features

| Feature | Description |
|---------|-------------|
| **Bidirectional MIDI Routing** | Forwards all MIDI messages between USB and DIN (IN/OUT) in both directions. |
| **MIDI Clock Forwarding** | USB and DIN MIDI clock, start, stop, and continue messages are forwarded in real time. |
| **Beat LED Indicator** | Beat LED flashes on every quarter note for visual sync confirmation. |
| **MIDI Thru Output** | DIN MIDI IN is optically isolated and can be routed to MIDI OUT for true THRU functionality. |
| **Sync In/Out** | Accepts and generates 24 PPQN clock pulses for external sync. |
| **Display Extension Port** | 6-pin DISPLAY_EXT header provides CLK_EXT (1 PPQN) and MIDI_EXT (31250 baud) for TinyPulse Display module. |
| **Robust Error Handling** | Serial buffer and UART error checks for reliable MIDI input. |
| **Open Source Hardware** | Designed for SparkFun Pro Micro (ATmega32U4) and compatible clones. |

---

## 3. Test Equipment Required

### Required Equipment
- Computer with DAW software (e.g., Ableton Live, FL Studio, Reaper)
- USB cable (micro-USB)
- DIN MIDI cables (5-pin, standard)
- MIDI monitor software (e.g., MIDI-OX, MIDI Monitor, or DAW MIDI monitor)

### Optional Equipment
- Hardware MIDI sequencer or drum machine (for DIN MIDI testing)
- Analog clock source with Sync Out (e.g., modular synth, Arturia Beatstep Pro)
- 3.5mm mono patch cables (for analog sync testing)
- Oscilloscope or logic analyzer (for sync pulse verification)
- Hardware synthesizer or sound module (for MIDI message testing)

---

## 4. Core Functionality Tests (Hardware Integration)

These tests require physical hardware and real MIDI equipment. **Run unit tests first** to verify core logic before hardware testing.

### Test Case 1: USB MIDI Clock Master

**Objective:** Verify USB MIDI clock is forwarded to DIN MIDI OUT and Sync OUT.

**Setup:**
1. Connect the device to a computer via USB.
2. Connect DIN MIDI OUT to a MIDI monitor or external gear.
3. Start MIDI clock from the DAW at 120 BPM.

**Test Steps:**
1. Send MIDI clock messages from the DAW.
2. Observe DIN MIDI OUT on the monitor.
3. Check Sync OUT pulse with oscilloscope (if available).
4. Verify BPM display shows "120".

**Expected Results:**
- ✅ MIDI clock messages appear on DIN MIDI OUT.
- ✅ Sync OUT pulses at 24 PPQN.
- ✅ BPM display shows correct tempo.
- ✅ Beat LED flashes on each quarter note.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: USB MIDI Clock Forwarding
  Scenario: Forward USB MIDI clock to DIN and Sync OUT
    Given the device is connected to a computer via USB
    And the computer is sending MIDI clock messages
    When the device receives MIDI clock over USB
    Then the device forwards MIDI clock to DIN MIDI OUT
    And the device outputs Sync pulses
    And the BPM display shows the correct tempo
```

---

### Test Case 2: DIN MIDI Clock Master

**Objective:** Verify DIN MIDI clock is forwarded to USB MIDI OUT and Sync OUT.

**Setup:**
1. Connect a hardware sequencer to DIN MIDI IN.
2. Connect the device to a computer via USB.
3. Start MIDI clock from the sequencer at 140 BPM.

**Test Steps:**
1. Send MIDI clock from the hardware sequencer.
2. Monitor USB MIDI on the computer with MIDI monitoring software.
3. Check Sync OUT pulse (if available).
4. Verify BPM display shows "140".

**Expected Results:**
- ✅ MIDI clock messages appear on USB MIDI OUT.
- ✅ Sync OUT pulses at 24 PPQN.
- ✅ BPM display shows correct tempo.
- ✅ Beat LED flashes on each quarter note.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: DIN MIDI Clock Forwarding
  Scenario: Forward DIN MIDI clock to USB and Sync OUT
    Given a hardware sequencer is connected to DIN MIDI IN
    And the sequencer is sending MIDI clock messages
    When the device receives MIDI clock over DIN
    Then the device forwards MIDI clock to USB MIDI OUT
    And the device outputs Sync pulses
    And the BPM display shows the correct tempo
```

---

### Test Case 3: USB ↔ DIN MIDI Bridge

**Objective:** Verify all MIDI messages are forwarded bidirectionally between USB and DIN.

**Setup:**
1. Connect the device to a computer via USB.
2. Connect DIN MIDI IN and OUT to MIDI monitoring devices or external gear.

**Test Steps:**
1. **USB to DIN:**
   - Send MIDI notes (C3, D3, E3) from the DAW over USB.
   - Verify notes appear on DIN MIDI OUT.
   - Send CC messages (e.g., CC1, CC7, CC11).
   - Send pitch bend and aftertouch messages.
   - Send program change messages.
2. **DIN to USB:**
   - Send MIDI notes from DIN MIDI IN.
   - Verify notes appear on USB MIDI in the DAW or monitoring software.
   - Send CC, pitch bend, and other channel messages.
3. **Real-time messages:**
   - Verify Start, Stop, Continue, and Clock messages are forwarded in both directions.

**Expected Results:**
- ✅ All MIDI channel messages from USB appear on DIN MIDI OUT.
- ✅ All MIDI channel messages from DIN appear on USB MIDI OUT.
- ✅ All real-time clock messages (Start, Stop, Continue, Clock) are forwarded.
- ✅ No data loss or corruption.
- ✅ Low latency (<1ms typical).

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Bidirectional MIDI Bridge
  Scenario: Forward all MIDI messages between USB and DIN
    Given the device is connected to both USB and DIN MIDI
    When any MIDI message is received on USB
    Then the message is forwarded to DIN MIDI OUT
    When any MIDI message is received on DIN
    Then the message is forwarded to USB MIDI OUT
```

---

### Test Case 4: MIDI Thru Splitter

**Objective:** Verify DIN MIDI IN is forwarded to DIN MIDI OUT and Sync OUT.

**Setup:**
1. Connect a MIDI source to DIN MIDI IN.
2. Connect DIN MIDI OUT to a MIDI monitor.
3. Connect Sync OUT to an oscilloscope (optional).

**Test Steps:**
1. Send MIDI messages from the source to DIN MIDI IN.
2. Verify all messages appear on DIN MIDI OUT.
3. Send MIDI clock and verify Sync OUT pulses.

**Expected Results:**
- ✅ All MIDI messages are forwarded to DIN MIDI OUT.
- ✅ Sync OUT pulses when clock is received.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: MIDI Thru Functionality
  Scenario: Forward DIN MIDI IN to DIN OUT and Sync OUT
    Given a MIDI source is connected to DIN MIDI IN
    When the device receives any MIDI message on DIN IN
    Then the message is forwarded to DIN MIDI OUT
    And the device outputs Sync pulses if clock is received
```

---

### Test Case 5: Standalone BPM Display

**Objective:** Verify BPM display shows tempo from any clock source.

**Setup:**
1. Connect a clock source (USB, DIN, or Sync In).
2. Power on the device.

**Test Steps:**
1. Send MIDI clock at 90 BPM.
2. Press and hold the button.
3. Verify display shows "t.90" (or "t. 90").
4. Release button and verify display returns to clock animation.
5. Change tempo to 180 BPM.
6. Press and hold button again.
7. Verify display updates to "t.180".
8. Stop the clock source.
9. Press button and verify display shows "IdLE".

**Expected Results:**
- ✅ Display shows correct BPM from any clock source when button is held.
- ✅ Display format is "t.###" (e.g., "t.120").
- ✅ Display updates in real time as tempo changes.
- ✅ Display shows "IdLE" when no clock is present.
- ✅ Display returns to clock animation when button is released.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: BPM Display
  Scenario: Show BPM from any clock source
    Given the device is powered on
    And a clock source (USB, DIN, or Sync In) is active
    When the user presses and holds the button
    Then the BPM display shows the current tempo in "t.###" format
    When the button is released
    Then the display returns to the clock animation
```

---

### Test Case 6: Sync In to MIDI Out

**Objective:** Verify Sync In pulses are converted to MIDI clock on USB and DIN OUT.

**Setup:**
1. Connect an analog clock source to Sync In.
2. Connect DIN MIDI OUT to a MIDI monitor or external gear.
3. Connect USB to computer and open MIDI monitoring software.

**Test Steps:**
1. Send 24 PPQN pulses from the analog clock at 100 BPM.
2. Verify MIDI clock appears on **both USB MIDI and DIN MIDI OUT**.
3. Press button and verify BPM display shows "t.100".
4. Change analog clock to 120 BPM.
5. Verify MIDI clock updates on both USB and DIN outputs.
6. Verify BPM display updates to "t.120".

**Expected Results:**
- ✅ MIDI clock is generated on **USB MIDI OUT**.
- ✅ MIDI clock is generated on **DIN MIDI OUT**.
- ✅ BPM display shows correct tempo.
- ✅ All outputs remain synchronized to Sync In.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Analog Sync to MIDI Clock
  Scenario: Convert Sync In pulses to MIDI clock
    Given an analog clock source is connected to Sync In
    When the device receives Sync In pulses
    Then the device generates MIDI clock on USB MIDI OUT
    And the device generates MIDI clock on DIN MIDI OUT
    And the BPM display shows the correct tempo
```

---

### Test Case 7: Button-Controlled Display

**Objective:** Verify display shows BPM when button is pressed.

**Setup:**
1. Power on the device.
2. Connect a clock source and start playback at 120 BPM.

**Test Steps:**
1. Observe the default display (should show rotating clock animation).
2. Press and hold the button.
3. Verify display changes to "t.120" format.
4. Release the button.
5. Verify display returns to clock animation.
6. Stop the clock source.
7. Press and hold the button.
8. Verify display shows "IdLE".

**Expected Results:**
- ✅ Display shows clock animation during normal operation.
- ✅ Display shows "t.###" format when button is pressed during clock playback.
- ✅ Display shows "IdLE" when button is pressed and no clock is present.
- ✅ Display returns to normal animation when button is released.
- ✅ Button responds reliably without bounce issues.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Display Mode Button
  Scenario: Toggle display modes with button
    Given the device is powered on
    When the user presses the display button
    Then the display shows current BPM in "t.###" format
    When the button is released
    Then the display returns to clock animation
```

---

### Test Case 8: High-Tempo Performance

**Objective:** Verify LED and display stability at high BPM.

**Setup:**
1. Connect a clock source.
2. Set tempo to 250 BPM.

**Test Steps:**
1. Send MIDI clock at 250 BPM.
2. Observe beat LED pulse (if visible).
3. Observe clock animation on display.
4. Press button to view BPM.
5. Verify display shows "t.250".
6. Test at 300 BPM (upper limit).
7. Verify all outputs remain stable.

**Expected Results:**
- ✅ Beat LED flashes clearly without flickering (pulse width clamped to 10-100ms).
- ✅ Clock animation remains visible and synchronized.
- ✅ BPM display shows correct tempo when button is pressed.
- ✅ All MIDI clock messages are forwarded without drops.
- ✅ Sync OUT pulses remain accurate.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Stable LED and Display at High BPM
  Scenario: Maintain stable LED and display at high tempo
    Given the device is running at high BPM (e.g., >200)
    When clock messages are received at high frequency
    Then the beat LED remains visible and stable
    And the BPM display updates smoothly
    And all clock messages are forwarded without drops
```

---

### Test Case 8.1: Clock Animation and Beat Position

**Objective:** Verify clock animation and beat position indicators work correctly.

**Setup:**
1. Connect a clock source.
2. Start playback at 120 BPM.

**Test Steps:**
1. Observe the 4-digit display during clock playback.
2. Verify a rotating animation is visible (16-step rotation).
3. Observe the decimal points on the display.
4. Verify decimal points indicate beat positions (1.2.3.4.).
5. Change tempo to 60 BPM and 180 BPM.
6. Verify animation remains synchronized to clock at all tempos.

**Expected Results:**
- ✅ Rotating animation is visible and synchronized to MIDI clock.
- ✅ Decimal points show beat positions (1, 2, 3, 4).
- ✅ Animation speed adjusts with tempo changes.
- ✅ Animation is smooth and non-flickering.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Clock Animation Display
  Scenario: Show synchronized clock animation
    Given the device is receiving MIDI clock
    When clock messages are being processed
    Then the display shows a rotating animation synchronized to the clock
    And decimal points indicate beat positions 1-4
```

---

### Test Case 8.2: Idle Display Behavior

**Objective:** Verify display shows idle animation when no clock is present.

**Setup:**
1. Power on the device with no clock source connected.

**Test Steps:**
1. Observe the display after power-on.
2. Verify display shows an animated pattern (rotating segments).
3. Press and hold the button.
4. Verify display changes to show "IdLE" text.
5. Release the button.
6. Verify display returns to animated pattern.
7. Start a clock source.
8. Verify display changes to clock animation.
9. Stop the clock source.
10. Verify display returns to idle animated pattern.

**Expected Results:**
- ✅ Display shows animated pattern when no clock is detected (not pressing button).
- ✅ Display shows "IdLE" text when button is pressed during idle state.
- ✅ Display transitions to clock animation when clock starts.
- ✅ Display returns to idle animation when clock stops.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Idle Display
  Scenario: Show idle animation when no clock is present
    Given the device is powered on
    When no clock source is active
    Then the display shows an animated pattern
    When the user presses the button during idle
    Then the display shows "IdLE" text
    When the button is released
    Then the display returns to idle animation
    When a clock source becomes active
    Then the display shows clock animation
```

---

### Test Case 9: Clock Source Priority

**Objective:** Verify device correctly prioritizes clock sources.

**Setup:**
1. Connect multiple clock sources: Sync In, DIN MIDI, USB MIDI.

**Test Steps:**
1. Start DIN MIDI clock at 140 BPM.
2. Verify device syncs to DIN MIDI clock.
3. Start USB MIDI clock at 120 BPM (while DIN is still running).
4. Verify device switches to USB MIDI clock.
5. Start Sync In at 100 BPM (while both USB and DIN are running).
6. Verify device switches to Sync In clock (highest priority).
7. Stop Sync In.
8. Verify device falls back to USB MIDI clock.
9. Stop USB MIDI.
10. Verify device falls back to DIN MIDI clock.

**Expected Results:**
- ✅ Sync In has highest priority.
- ✅ USB MIDI has second priority.
- ✅ DIN MIDI has lowest priority.
- ✅ Device automatically switches to highest priority active source.
- ✅ No jitter or erratic output during source switching.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Clock Source Priority
  Scenario: Prioritize clock sources correctly
    Given multiple clock sources are active
    When Sync In is active
    Then the device uses Sync In as the clock source
    When Sync In stops and USB MIDI is active
    Then the device switches to USB MIDI
    When USB MIDI stops and DIN MIDI is active
    Then the device switches to DIN MIDI
```

---

### Test Case 10: USB Active Sensing and Timeout

**Objective:** Verify USB clock timeout handling.

**Setup:**
1. Connect device to computer via USB.
2. Start MIDI clock from DAW at 120 BPM.

**Test Steps:**
1. Verify device syncs to USB clock and shows clock animation.
2. Pause the DAW (stop sending clock but keep USB connected).
3. Wait 3 seconds (USB timeout period).
4. Verify device stops clock animation and shows idle animation pattern.
5. Press and hold button.
6. Verify display shows "IdLE" text.
7. Release button.
8. Resume DAW playback.
9. Verify device resumes clock synchronization and clock animation.

**Expected Results:**
- ✅ Device detects USB clock inactivity after 3 seconds.
- ✅ Display returns to idle animation pattern (not "IdLE" text) after timeout.
- ✅ Display shows "IdLE" text only when button is pressed during idle.
- ✅ Device resumes clock animation when USB clock resumes.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: USB Clock Timeout
  Scenario: Handle USB clock inactivity
    Given the device is synced to USB MIDI clock
    When USB clock messages stop for 3 seconds
    Then the device stops the clock and shows idle animation
    When the button is pressed during idle
    Then the display shows "IdLE" text
    When USB clock messages resume
    Then the device resumes clock synchronization
```

---

### Test Case 11: Sync Cable Detection

**Objective:** Verify device automatically enables/disables sync I/O based on cable connection.

**Setup:**
1. Power on the device.
2. Connect USB or DIN MIDI clock source at 120 BPM.
3. Have 3.5mm patch cables ready.

**Test Steps:**
1. Start MIDI clock with no sync cables connected.
2. Verify device syncs to MIDI clock normally.
3. Connect a cable to Sync OUT jack.
4. Verify sync pulses appear on Sync OUT.
5. Disconnect Sync OUT cable.
6. Verify sync pulses stop on Sync OUT.
7. Connect a cable to Sync IN jack.
8. Send analog clock pulses (24 PPQN) to Sync IN at 140 BPM.
9. Verify device switches to Sync IN as clock source (highest priority).
10. Press button to verify BPM shows "t.140".
11. Disconnect Sync IN cable.
12. Verify device falls back to MIDI clock source.
13. Press button to verify BPM shows "t.120".

**Expected Results:**
- ✅ Sync OUT only generates pulses when cable is connected.
- ✅ Sync IN only accepts input when cable is connected.
- ✅ Device automatically detects cable insertion/removal.
- ✅ Clock source priority respects cable detection (unplugged = disabled).
- ✅ Display updates correctly when switching sources.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Sync Cable Detection
  Scenario: Enable sync I/O only when cables are connected
    Given the device is powered on
    When a sync cable is connected to Sync OUT
    Then sync pulses are generated on Sync OUT
    When the Sync OUT cable is disconnected
    Then sync pulses stop on Sync OUT
    When a sync cable is connected to Sync IN with active clock
    Then the device switches to Sync IN as the clock source
    When the Sync IN cable is disconnected
    Then the device falls back to the next priority clock source
```

---

## 5. Edge Case Tests

### Test Case 12: Sync Out Cable Hot Swap

**Objective:** Verify Sync OUT automatically starts/stops when cable is inserted/removed during operation.

**Setup:**
1. Connect USB MIDI clock at 120 BPM.
2. Start playback.

**Test Steps:**
1. With clock running, plug in Sync OUT cable.
2. Verify sync pulses start immediately (use LED or oscilloscope).
3. While clock is still running, unplug Sync OUT cable.
4. Verify sync pulses stop immediately.
5. Plug cable back in while clock is running.
6. Verify sync pulses resume immediately.

**Expected Results:**
- ✅ Sync OUT responds to cable changes without restarting device.
- ✅ No delay or glitches when cable is inserted/removed.
- ✅ USB and DIN MIDI clock continue unaffected.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Sync OUT Hot Swap
  Scenario: Automatically enable/disable Sync OUT during operation
    Given the device is running with active MIDI clock
    When a cable is plugged into Sync OUT
    Then sync pulses start immediately
    When the cable is unplugged from Sync OUT
    Then sync pulses stop immediately
```

---

### Test Case 13: Power Loss During Operation

**Objective:** Verify device recovers cleanly after power loss.

**Setup:**
1. Connect device and start forwarding MIDI messages.

**Test Steps:**
1. Unplug power from the device.
2. Wait 5 seconds and restore power.
3. Verify device resumes forwarding MIDI without reconfiguration.

**Expected Results:**
- ✅ Device restarts cleanly.
- ✅ MIDI forwarding resumes automatically.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Power Recovery
  Scenario: Device resumes normal operation after power loss
    Given the device is running and forwarding MIDI messages
    When power is lost and then restored
    Then the device restarts cleanly
    And resumes forwarding MIDI messages without manual intervention
```

---

### Test Case 14: Simultaneous USB and DIN Clock Sources

**Objective:** Verify device handles simultaneous clock sources without jitter.

**Setup:**
1. Connect USB and DIN clock sources at different tempos (e.g., 120 BPM USB, 140 BPM DIN).

**Test Steps:**
1. Send MIDI clock from both USB and DIN simultaneously.
2. Observe which clock source is prioritized.
3. Verify no jitter or erratic output.

**Expected Results:**
- ✅ Device prioritizes one clock source.
- ✅ No jitter or unstable output.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Clock Source Priority
  Scenario: Handle simultaneous clock input on USB and DIN
    Given both USB and DIN MIDI clock sources are active
    When both sources send clock messages at different tempos
    Then the device prioritizes one clock source (as per design)
    And does not produce jitter or erratic output
```

---

### Test Case 15: Invalid or Corrupted MIDI Data

**Objective:** Verify device ignores malformed MIDI messages and continues forwarding valid ones.

**Setup:**
1. Connect a MIDI source that can send malformed data (e.g., incomplete SysEx, invalid status bytes).

**Test Steps:**
1. Send a mix of valid and invalid MIDI messages.
2. Verify valid messages are forwarded.
3. Verify device does not crash or hang.

**Expected Results:**
- ✅ Invalid messages are ignored.
- ✅ Valid messages continue to be forwarded.
- ✅ Device remains stable.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Robust MIDI Parsing
  Scenario: Ignore invalid or corrupted MIDI messages
    Given the device receives malformed or incomplete MIDI data
    When the device processes incoming messages
    Then invalid messages are ignored
    And valid messages continue to be forwarded
```

---

### Test Case 16: Button Bounce or Rapid Presses

**Objective:** Verify button debouncing prevents erratic display behavior.

**Setup:**
1. Power on the device.

**Test Steps:**
1. Rapidly press the display button 10 times in quick succession.
2. Hold the button for 2 seconds.
3. Observe display behavior.

**Expected Results:**
- ✅ Only intentional presses are registered.
- ✅ Display does not flicker or change erratically.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Button Debouncing
  Scenario: Handle rapid or bouncing button presses
    Given the user rapidly presses or holds the display button
    When the device processes button input
    Then only intentional presses are registered
    And the display mode does not flicker or change erratically
```

---

### Test Case 17: Extreme BPM Values

**Objective:** Verify device handles very low and very high BPM values.

**Setup:**
1. Connect a clock source.

**Test Steps:**
1. Send MIDI clock at 10 BPM.
2. Verify display shows "10" and LED pulse is visible.
3. Send MIDI clock at 350 BPM.
4. Verify display shows "350" and LED pulse is clamped to safe limits.

**Expected Results:**
- ✅ Display remains readable at extreme BPM values.
- ✅ LED pulse width is clamped (10–100ms).

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: BPM Range Handling
  Scenario: Handle extremely low or high BPM values
    Given the device receives clock at very low (<20) or very high (>300) BPM
    When the device calculates and displays BPM
    Then the display remains readable
    And the LED pulse width is clamped to safe limits
```

---

### Test Case 18: MIDI Buffer Overflow

**Objective:** Verify device handles high MIDI traffic without crashing.

**Setup:**
1. Connect a MIDI source capable of sending rapid bursts of messages.

**Test Steps:**
1. Send a continuous stream of MIDI messages at maximum rate (e.g., notes, CC, clock).
2. Monitor device stability.
3. Verify as many messages as possible are forwarded.

**Expected Results:**
- ✅ Device does not crash or hang.
- ✅ Messages are forwarded without corruption.
- ✅ Overflow is handled gracefully (if it occurs).

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Buffer Management
  Scenario: Prevent data loss on high MIDI traffic
    Given the device receives a burst of MIDI messages
    When the internal buffer is nearly full
    Then the device does not crash or hang
    And as many messages as possible are forwarded
    And an overflow condition is handled gracefully
```

---

### Test Case 19: Hot Plugging USB or MIDI Cables

**Objective:** Verify device handles cable insertion/removal during operation.

**Setup:**
1. Power on device and start MIDI forwarding.

**Test Steps:**
1. Unplug USB cable during operation.
2. Plug USB cable back in.
3. Verify device resumes forwarding without reset.
4. Repeat for DIN MIDI cables.

**Expected Results:**
- ✅ Device continues operating without reset.
- ✅ MIDI forwarding resumes as soon as connection is restored.

**Acceptance Criteria (Gherkin):**
```gherkin
Feature: Hot Plug Robustness
  Scenario: Handle cable insertion/removal during operation
    Given the device is running
    When a USB or DIN MIDI cable is plugged in or removed
    Then the device continues operating without reset
    And resumes forwarding messages as soon as connection is restored
```

---

## 6. Test Execution Checklist

Use this checklist to track your testing progress:

**Core Functionality Tests:**
- [ ] Test Case 1: USB MIDI Clock Master
- [ ] Test Case 2: DIN MIDI Clock Master
- [ ] Test Case 3: USB ↔ DIN MIDI Bridge
- [ ] Test Case 4: MIDI Thru Splitter
- [ ] Test Case 5: Standalone BPM Display
- [ ] Test Case 6: Sync In to MIDI Out
- [ ] Test Case 7: Button-Controlled Display
- [ ] Test Case 8: High-Tempo Performance
- [ ] Test Case 8.1: Clock Animation and Beat Position
- [ ] Test Case 8.2: Idle Display Behavior
- [ ] Test Case 9: Clock Source Priority
- [ ] Test Case 10: USB Active Sensing and Timeout
- [ ] Test Case 11: Sync Cable Detection

**Edge Case Tests:**
- [ ] Test Case 12: Sync Out Cable Hot Swap
- [ ] Test Case 13: Power Loss During Operation
- [ ] Test Case 14: Simultaneous USB and DIN Clock Sources
- [ ] Test Case 15: Invalid or Corrupted MIDI Data
- [ ] Test Case 16: Button Bounce or Rapid Presses
- [ ] Test Case 17: Extreme BPM Values
- [ ] Test Case 18: MIDI Buffer Overflow
- [ ] Test Case 19: Hot Plugging USB or MIDI Cables

---

## 7. Reporting Issues

When reporting issues, please include:
- **Test case number and name** (e.g., "Test Case 3: USB ↔ DIN MIDI Bridge")
- **Hardware and software setup:**
  - Device firmware version
  - Computer OS and DAW software (if applicable)
  - Connected hardware (MIDI devices, cables, etc.)
- **Steps to reproduce:**
  - Detailed step-by-step instructions
  - Any specific settings or configurations
- **Expected vs. actual results:**
  - What should have happened
  - What actually happened
- **Supporting materials:**
  - Screenshots or MIDI monitor logs (if applicable)
  - Serial debug output (if SERIAL_DEBUG enabled)
  - Video recording (for timing-related issues)

---

## 8. Testing Notes and Tips

### MIDI Monitoring Tools
- **Windows:** MIDI-OX (free), MIDI Monitor
- **macOS:** MIDI Monitor (free), Snoize
- **Linux:** amidi, kmidimon
- **DAW built-in:** Most DAWs have MIDI monitor/log views

### Common Issues and Solutions

| Issue | Possible Cause | Solution |
|-------|----------------|----------|
| Display shows idle animation constantly | No clock detected | Check MIDI cables, verify source is sending clock |
| Display shows "IdLE" text without button press | Should not happen | Report as bug - "IdLE" text should only appear when button is pressed during idle |
| BPM display incorrect | Non-standard PPQN on analog sync | Verify analog source uses 24 PPQN or accept scaled BPM reading |
| MIDI messages not passing through | Wrong cable type or wiring | Use standard MIDI cables (not null-modem), check optocoupler circuit |
| USB not recognized | Driver or cable issue | Try different USB port, cable, or update USB MIDI drivers |
| Clock jitter or unstable tempo | Multiple sources active | Check clock source priority (Sync In > USB > DIN), disable unused sources |
| Display flickering or dim | Power supply issue | Use quality USB cable and reliable power source (500mA minimum) |
| Button doesn't respond | Debounce or wiring | Check button connection to pin 16 and GND, verify 50ms debounce |

### Performance Benchmarks
- **Latency:** <1ms typical for MIDI passthrough
- **Clock Accuracy:** Microsecond-precision (interrupt-based)
- **BPM Range:** 30-300 BPM (reliable detection and display)
- **Message Throughput:** Handles bursts up to MIDI spec (31.25 kbps)
- **Clock Source Priority:** Sync In (highest) > USB MIDI > DIN MIDI (lowest)
- **USB Timeout:** 3 seconds of inactivity before switching sources
- **LED Pulse Width:** Dynamic 10-100ms based on tempo

### Debug Mode
To enable detailed logging, edit `config.h`:
```cpp
#define SERIAL_DEBUG    true
#define DEBUG_BAUD_RATE 115200
```
Then open Serial Monitor at 115200 baud to view:
- BPM changes (>2 BPM threshold)
- Clock source switching
- System status messages

---
