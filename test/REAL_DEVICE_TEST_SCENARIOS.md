# Real Device Test Scenarios

## Table of Contents

### Setup & Configuration
- [Equipment](#equipment)
- [Test Setup Configuration](#test-setup-configuration)
- [Standard Connections](#standard-connections)

### Test Scenarios
- [Scenario 1: Ableton as Master (USB MIDI)](#scenario-1-ableton-as-master-usb-midi)
- [Scenario 2: Beatstep Pro as Master (SYNC_IN)](#scenario-2-beatstep-pro-as-master-sync_in)
- [Scenario 3: TR-08 as Master (DIN MIDI)](#scenario-3-tr-08-as-master-din-midi)
- [Scenario 4: Clock Priority Test](#scenario-4-clock-priority-test)
- [Scenario 5: SYNC_OUT to Analog Device](#scenario-5-sync_out-to-analog-device)
- [Scenario 6: MIDI THRU (USB → DIN)](#scenario-6-midi-thru-test-usb--din-forwarding)
- [Scenario 7: MIDI THRU (DIN → USB)](#scenario-7-midi-thru-test-din--usb-forwarding)
- [Scenario 8: Bidirectional MIDI THRU](#scenario-8-bidirectional-midi-thru-test)
- [Scenario 9: Clock Forwarding (All Directions)](#scenario-9-clock-forwarding-test-all-directions)
- [Scenario 10: Clock + Notes Simultaneous](#scenario-10-clock--notes-simultaneous-test)
- [Scenario 11: Full Chain Test](#scenario-11-full-chain-test)
- [Scenario 12: Tempo Stability Test](#scenario-12-tempo-stability-test)
- [Scenario 13: Timeout and Recovery](#scenario-13-timeout-and-recovery)
- [Scenario 14: All Outputs Test (Hub Mode)](#scenario-14-all-outputs-test-hub-mode)
- [Scenario 15: MIDI Message Stress Test](#scenario-15-midi-message-stress-test)

### Reference & Tools
- [Port Testing Checklist](#port-testing-checklist)
- [Complete Port Matrix](#complete-port-matrix)
- [Troubleshooting Guide](#troubleshooting-guide)
- [Quick Reference: Rotary Switch Settings](#quick-reference-rotary-switch-settings)
- [Success Criteria](#success-criteria)

---

## Equipment
- **BytePulse**: MIDI BytePulse (USB MIDI sync box)
- **TinyPulse**: BPM monitor display (connected to DISPLAY_CLK)
- **Ableton Lite**: DAW running on computer (USB MIDI)
- **Beatstep Pro**: CV/Gate sequencer with sync out
- **TR-08**: Roland drum machine (DIN MIDI)
- **JU-06A**: Roland Boutique synth with external clock input (DIN MIDI)

---

## Test Setup Configuration

### Disable All Test Modes First
In `config.h`, set:
```cpp
#define TEST_MODE_CLOCK     false
#define TEST_MODE_SYNC_IN   false
#define TEST_MODE_MIDI_IN   false
```

### Standard Connections
- **BytePulse USB** → Computer (for Ableton MIDI clock)
- **BytePulse DIN MIDI IN** → TR-08 MIDI OUT
- **BytePulse DIN MIDI OUT** → TR-08 MIDI IN (for forwarding)
- **BytePulse SYNC_IN** → Beatstep Pro SYNC OUT
- **BytePulse SYNC_OUT** → (optional) Volca or other analog device
- **BytePulse DISPLAY_CLK** → TinyPulse clock input
- **BytePulse LED** → Visual pulse indicator

---

## Scenario 1: Ableton as Master (USB MIDI)
**Goal**: Test USB MIDI clock from DAW

### Setup
1. Connect BytePulse USB to computer
2. Set Ableton tempo to **120 BPM**
3. Rotary switch Position 2 (2 PPQN for SYNC_OUT)

### Steps
1. Open Ableton Lite
2. Go to Preferences → Link/Tempo/MIDI
3. Enable **MIDI Sync** output to BytePulse device
4. Create empty MIDI track
5. Press **Play** in Ableton

### Expected Results
- ✅ TinyPulse shows **120 BPM** (from DISPLAY_CLK at 1 PPQN)
- ✅ BytePulse LED pulses at beat rate
- ✅ SYNC_OUT pulses at 2 PPQN (250ms intervals)
- ✅ DIN MIDI OUT forwards clock to TR-08
- ✅ If TR-08 set to external sync, it follows Ableton tempo

### Test Variations
- Change Ableton tempo to **80 BPM** → TinyPulse should show **80 BPM**
- Change Ableton tempo to **140 BPM** → TinyPulse should show **140 BPM**
- Stop Ableton → LED stops, TinyPulse shows "----"
- Restart Ableton → Everything syncs again

---

## Scenario 2: Beatstep Pro as Master (SYNC_IN)
**Goal**: Test analog sync input multiplication

### Setup
1. Connect Beatstep Pro SYNC OUT → BytePulse SYNC_IN
2. Set Beatstep tempo to **120 BPM**
3. **Critical**: Beatstep Pro outputs **1 PPQN** (1 pulse per quarter note)
4. Rotary switch Position 1 (tell BytePulse to expect 1 PPQN)

### Steps
1. Set Beatstep sequencer tempo to 120 BPM
2. Press **Play** on Beatstep Pro
3. Observe TinyPulse and BytePulse LED

### Expected Results
- ✅ TinyPulse shows **120 BPM**
- ✅ BytePulse LED pulses at beat rate
- ✅ SYNC_OUT pulses at 1 PPQN (matches input)
- ✅ USB MIDI OUT sends 24 PPQN clock to Ableton
- ✅ DIN MIDI OUT sends 24 PPQN clock to TR-08

### Rotary Switch Test (PPQN Interpretation)
Keep Beatstep at 120 BPM, 1 PPQN output:

| Position | PPQN Setting | Expected TinyPulse BPM | Notes |
|----------|--------------|------------------------|-------|
| 1 | 1 PPQN | **120** ✅ | Correct interpretation |
| 2 | 2 PPQN | **60** ❌ | Interprets 1 pulse as 2 PPQN |
| 3 | 4 PPQN | **30** ❌ | Interprets 1 pulse as 4 PPQN |
| 4 | 6 PPQN | **20** ❌ | Interprets 1 pulse as 6 PPQN |
| 5 | 24 PPQN | **5** ❌ | Interprets 1 pulse as 24 PPQN |

**Conclusion**: Position 1 is correct for Beatstep Pro (1 PPQN)

---

## Scenario 3: TR-08 as Master (DIN MIDI)
**Goal**: Test DIN MIDI input clock

### Setup
1. Connect TR-08 MIDI OUT → BytePulse DIN MIDI IN
2. Set TR-08 tempo to **120 BPM**
3. TR-08 sends standard 24 PPQN MIDI clock
4. Rotary switch Position 2 (2 PPQN for SYNC_OUT)

### Steps
1. Set TR-08 to internal clock mode
2. Set tempo to 120 BPM
3. Press **Play/Stop** button on TR-08

### Expected Results
- ✅ TinyPulse shows **120 BPM**
- ✅ BytePulse LED pulses at beat rate
- ✅ SYNC_OUT pulses at 2 PPQN
- ✅ USB MIDI OUT forwards clock to Ableton (can sync Ableton to TR-08)

### Test Variations
- Change TR-08 tempo to **100 BPM** → TinyPulse follows
- Stop TR-08 → LED stops, TinyPulse shows "----"

---

## Scenario 4: Clock Priority Test
**Goal**: Verify SYNC_IN has highest priority, USB blocks DIN

### Test 4A: SYNC_IN Blocks USB
1. Start **Beatstep Pro** playing at 100 BPM (SYNC_IN active)
2. TinyPulse shows **100 BPM**
3. Start **Ableton** playing at 120 BPM (USB MIDI)
4. **Expected**: TinyPulse stays at **100 BPM** (SYNC_IN priority)
5. Stop Beatstep Pro
6. **Expected**: TinyPulse switches to **120 BPM** (falls back to USB)

### Test 4B: SYNC_IN Blocks DIN
1. Start **Beatstep Pro** at 100 BPM
2. Start **TR-08** at 120 BPM (DIN MIDI IN)
3. **Expected**: TinyPulse shows **100 BPM** (SYNC_IN priority)

### Test 4C: USB Blocks DIN
1. Make sure Beatstep is **stopped** (no SYNC_IN)
2. Start **Ableton** at 110 BPM (USB)
3. Start **TR-08** at 120 BPM (DIN)
4. **Expected**: TinyPulse shows **110 BPM** (USB blocks DIN)
5. Stop Ableton
6. **Expected**: TinyPulse switches to **120 BPM** (falls back to DIN)

---

## Scenario 5: SYNC_OUT to Analog Device
**Goal**: Test PPQN division for analog devices

### Setup Option A: Using SYNC_OUT (Analog Sync)
For devices with dedicated sync input accepting PPQN signals:
1. Ableton as master at **120 BPM**
2. Connect BytePulse SYNC_OUT → Device SYNC IN
3. Set rotary to match device's expected PPQN rate

### Setup Option B: Using DIN MIDI (JU-06A)
JU-06A uses standard DIN MIDI clock, not analog sync:
1. Ableton as master at **120 BPM**
2. Connect BytePulse DIN MIDI OUT → JU-06A MIDI IN
3. Set JU-06A to external clock mode (CLOCK → EXT in menu)
4. **Rotary switch position doesn't matter** (MIDI is always 24 PPQN)

### Expected Results (JU-06A)
- ✅ JU-06A syncs to **120 BPM** via MIDI clock
- ✅ Arpeggiator/sequencer follows Ableton tempo
- ✅ Change Ableton to 80 BPM → JU-06A follows
- ✅ Works at any rotary position (MIDI path independent of SYNC settings)

### SYNC_OUT Test Example (if analog device available)
| Position | SYNC_OUT PPQN | Device at 120 BPM Source |
|----------|---------------|----------------|
| 1 | 1 PPQN | Works for 1 PPQN devices |
| 2 | 2 PPQN | Works for 2 PPQN devices (Korg Volca, PO) |
| 3 | 4 PPQN | Works for 4 PPQN devices (Roland DIN Sync) |
| 5 | 24 PPQN | Full MIDI rate passthrough |

**Note**: JU-06A doesn't use SYNC_OUT - it uses standard MIDI clock via DIN connector

---

## Scenario 6: MIDI THRU Test (USB → DIN Forwarding)
**Goal**: Verify MIDI messages pass through from USB to DIN

### Setup
```
Ableton (USB MIDI)
    ↓
BytePulse USB IN
    ↓
BytePulse DIN MIDI OUT → TR-08 MIDI IN
```

### Test 6A: MIDI Note Forwarding
1. Connect as shown above
2. Create MIDI track in Ableton
3. Set MIDI output to BytePulse device
4. Set TR-08 to receive on MIDI channel 10 (drum channel)
5. Play notes on MIDI keyboard or click notes in Ableton

### Expected Results
- ✅ Notes played in Ableton trigger sounds on TR-08
- ✅ Velocity information is preserved
- ✅ Note timing is accurate (no noticeable delay)
- ✅ All 16 MIDI channels work correctly

### Test 6B: MIDI CC Forwarding
1. In Ableton, send CC messages (e.g., CC7 = volume)
2. Adjust CC values with MIDI controller or automation

### Expected Results
- ✅ CC messages control TR-08 parameters
- ✅ All CC values 0-127 work correctly
- ✅ Multiple CCs can be sent simultaneously

### Test 6C: Program Change Forwarding
1. Send Program Change messages from Ableton
2. TR-08 should change patterns/kits

### Expected Results
- ✅ Program changes are forwarded correctly
- ✅ TR-08 responds to program changes

---

## Scenario 7: MIDI THRU Test (DIN → USB Forwarding)
**Goal**: Verify MIDI messages pass through from DIN to USB

### Setup
```
TR-08 MIDI OUT
    ↓
BytePulse DIN MIDI IN
    ↓
BytePulse USB OUT → Ableton (or other USB MIDI device)
```

### Test 7A: Recording from TR-08
1. Connect TR-08 MIDI OUT → BytePulse DIN MIDI IN
2. In Ableton, create MIDI track with BytePulse as input
3. Arm track for recording
4. Play pattern on TR-08

### Expected Results
- ✅ Ableton receives MIDI notes from TR-08
- ✅ Timing is accurate
- ✅ Velocity information is preserved
- ✅ Can record TR-08 performance into Ableton

### Test 7B: MIDI Monitor Check
1. Use MIDI monitor software (MIDI-OX on Windows, MIDI Monitor on Mac)
2. Play notes on TR-08
3. Watch incoming MIDI messages

### Expected Results
- ✅ All note on/off messages appear
- ✅ Timing messages (clock) appear
- ✅ No dropped messages
- ✅ No MIDI data corruption

---

## Scenario 8: Bidirectional MIDI THRU Test
**Goal**: Verify simultaneous bidirectional MIDI forwarding

### Setup
```
Ableton USB ←→ BytePulse ←→ TR-08 DIN
```

### Test Steps
1. Send notes from Ableton to TR-08 (USB → DIN)
2. Simultaneously play pattern on TR-08 sending back to Ableton (DIN → USB)
3. Record TR-08 in Ableton while sending clock/notes to it

### Expected Results
- ✅ Both directions work simultaneously
- ✅ No MIDI feedback loops
- ✅ No message conflicts or corruption
- ✅ Timing remains stable in both directions

---

## Scenario 9: Clock Forwarding Test (All Directions)
**Goal**: Verify MIDI clock is forwarded correctly in all paths

### Test 9A: USB Clock → DIN Clock
**Setup**: Ableton (120 BPM) → BytePulse USB → BytePulse DIN OUT → TR-08

1. Start Ableton at 120 BPM
2. Set TR-08 to external sync
3. TR-08 should run at 120 BPM

**Expected**:
- ✅ TR-08 syncs perfectly to Ableton tempo
- ✅ Tempo changes in Ableton are followed by TR-08
- ✅ Start/Stop messages are forwarded

### Test 9B: DIN Clock → USB Clock
**Setup**: TR-08 (110 BPM) → BytePulse DIN IN → BytePulse USB → Ableton

1. Set TR-08 to internal clock at 110 BPM
2. Set Ableton to external sync from BytePulse
3. Start TR-08

**Expected**:
- ✅ Ableton syncs to TR-08 tempo (110 BPM)
- ✅ Ableton follows TR-08 start/stop
- ✅ Tempo is stable

### Test 9C: SYNC_IN → All Outputs
**Setup**: Beatstep Pro → BytePulse SYNC_IN → MIDI (USB + DIN) + SYNC_OUT

1. Start Beatstep at 100 BPM (1 PPQN sync)
2. Set rotary to Position 1
3. Connect TR-08 to DIN OUT, Volca to SYNC_OUT

**Expected**:
- ✅ TinyPulse shows 100 BPM
- ✅ TR-08 syncs to 100 BPM (receives 24 PPQN MIDI clock)
- ✅ Ableton syncs to 100 BPM (receives 24 PPQN USB MIDI clock)
- ✅ Volca syncs to 100 BPM (if rotary set to Volca's PPQN)
- ✅ All devices run in perfect sync

---

## Scenario 10: Clock + Notes Simultaneous Test
**Goal**: Verify clock and note data don't interfere

### Setup
```
Ableton (120 BPM, playing MIDI notes)
    ↓
BytePulse
    ↓
TR-08 (external sync, receiving notes)
```

### Test Steps
1. Set Ableton to 120 BPM with clock sync enabled
2. Create MIDI track sending notes to BytePulse → TR-08
3. Create a melody/drum pattern in Ableton
4. Set TR-08 to external sync
5. Start playback

### Expected Results
- ✅ TR-08 receives both clock AND note data
- ✅ TR-08 plays in sync with Ableton
- ✅ Notes trigger correctly on TR-08
- ✅ Timing is tight (no MIDI jitter)
- ✅ No dropped notes or clock messages

---

## Scenario 11: Full Chain Test
**Goal**: Verify complete signal flow

### Setup
```
Ableton (120 BPM, USB MIDI)
    ↓
BytePulse
    ├→ DISPLAY_CLK → TinyPulse (should show 120)
    ├→ LED (visual pulse)
    ├→ DIN MIDI OUT → TR-08 (external sync, follows 120 BPM)
    └→ DIN MIDI OUT (chain) → JU-06A (external sync, follows 120 BPM)
```

### Steps
1. Set rotary to Position 2
2. Start Ableton at 120 BPM
3. Set TR-08 to external sync mode
4. Observe all devices

### Expected Results
- ✅ TinyPulse: **120 BPM**
- ✅ TR-08: Plays pattern at **120 BPM** in sync with Ableton
- ✅ JU-06A: Arpeggiator runs at **120 BPM** in sync
- ✅ BytePulse LED: Pulses at beat rate
- ✅ Change Ableton to 100 BPM → **all devices follow**

**Note**: To sync both TR-08 and JU-06A via DIN MIDI, use MIDI THRU daisy-chain: BytePulse → TR-08 MIDI THRU → JU-06A

---

## Scenario 12: Tempo Stability Test
**Goal**: Verify BPM calculation accuracy

### Test at Different Tempos
| Ableton BPM | Expected TinyPulse | Tolerance |
|-------------|-------------------|-----------|
| 30 | 30 ±1 | Very slow |
| 60 | 60 ±1 | Slow |
| 80 | 80 ±1 | Medium slow |
| 100 | 100 ±1 | Medium |
| 120 | 120 ±1 | Standard |
| 140 | 140 ±1 | Fast |
| 180 | 180 ±2 | Very fast |
| 200 | 200 ±2 | Extreme |

**Note**: TinyPulse has 103% calibration, so actual reading might be ~103 for 100 BPM input

---

## Scenario 13: Timeout and Recovery
**Goal**: Test clock source timeout and fallback

### Test 8A: USB Timeout
1. Start Ableton at 120 BPM
2. TinyPulse shows 120 BPM
3. **Stop** Ableton (MIDI Stop message sent)
4. **Expected**: TinyPulse shows "----" within 3 seconds
5. LED stops pulsing

### Test 8B: SYNC_IN Timeout
1. Start Beatstep at 120 BPM
2. TinyPulse shows 120 BPM
3. **Stop** Beatstep (no more sync pulses)
4. **Expected**: After 3 seconds of no pulses, TinyPulse shows "----"

### Test 8C: Fallback Chain
1. Start **Beatstep** (SYNC_IN) at 100 BPM
2. Start **Ableton** (USB) at 120 BPM
3. TinyPulse shows **100 BPM** (SYNC_IN priority)
4. **Stop Beatstep**
5. **Expected**: Within 3 seconds, TinyPulse switches to **120 BPM** (USB fallback)

---

## Scenario 14: All Outputs Test (Hub Mode)
**Goal**: Verify BytePulse as central MIDI/sync hub

### Ultimate Setup
```
                    ┌─ USB MIDI → Ableton (DAW sync)
                    │
Beatstep Pro ──→ BytePulse ─┼─ DIN MIDI OUT → TR-08 → JU-06A (chain)
(SYNC_IN master)            │
                    ├─ SYNC_OUT → (reserved for analog device)
                    │
                    └─ DISPLAY_CLK → TinyPulse (BPM monitor)
```

### Test Steps
1. Set Beatstep Pro to 128 BPM, 1 PPQN output
2. Set BytePulse rotary to Position 1 (1 PPQN)
3. JU-06A receives MIDI clock via DIN MIDI (no SYNC_OUT needed)
   - Chain: BytePulse DIN OUT → TR-08 MIDI THRU → JU-06A MIDI IN
   - Rotary position doesn't affect MIDI clock (always 24 PPQN)
4. Start Beatstep sequencer
5. Set TR-08 to external sync
6. Set Ableton to external sync from BytePulse

### Expected Results
- ✅ TinyPulse shows **128 BPM**
- ✅ Ableton syncs to **128 BPM** (receives USB MIDI clock)
- ✅ TR-08 syncs to **128 BPM** (receives DIN MIDI clock)
- ✅ JU-06A syncs to **128 BPM** (receives DIN MIDI clock via THRU)
- ✅ SYNC_OUT provides clock to analog devices (if connected)
- ✅ All devices run in perfect lock-step
- ✅ LED pulses at beat rate

### Notes on JU-06A Connection
JU-06A uses standard MIDI clock (24 PPQN), not analog sync:
- **Connect via**: DIN MIDI OUT → JU-06A MIDI IN (or chain via TR-08 MIDI THRU)
- **Rotary switch**: Doesn't affect MIDI clock output (always 24 PPQN)
- **External clock mode**: Set on JU-06A via menu (CLOCK → EXT)

### Notes on Rotary Switch
The rotary switch controls **both** SYNC_IN interpretation **and** SYNC_OUT division:
- MIDI outputs (USB/DIN) are always 24 PPQN regardless of rotary position
- SYNC_OUT follows rotary switch for analog device compatibility

---

## Scenario 15: MIDI Message Stress Test
**Goal**: Test high-throughput MIDI data handling

### Test 15A: Dense Note Data
1. Create Ableton track with very dense MIDI notes (32nd notes at 180 BPM)
2. Send to TR-08 via BytePulse
3. Play for 1+ minute

**Expected**:
- ✅ No dropped notes
- ✅ Timing remains accurate
- ✅ No MIDI buffer overflow
- ✅ Clock stays stable

### Test 15B: Multiple CC Automation
1. Automate 4+ CC parameters simultaneously in Ableton
2. Send to TR-08 while clock is running

**Expected**:
- ✅ All CC changes are forwarded
- ✅ Clock doesn't skip
- ✅ No data corruption

### Test 15C: SysEx Transfer
1. Send SysEx data (pattern backup, etc.) through BytePulse
2. TR-08 should receive complete SysEx message

**Expected**:
- ✅ SysEx messages pass through completely
- ✅ No truncation or corruption
- ✅ Large SysEx messages work (tested in MIDIHandler)

---

## Port Testing Checklist

### Input Ports
- [ ] **USB MIDI IN**: Receives clock from Ableton ✓
- [ ] **USB MIDI IN**: Receives notes from Ableton ✓
- [ ] **USB MIDI IN**: Receives CC from Ableton ✓
- [ ] **DIN MIDI IN**: Receives clock from TR-08 ✓
- [ ] **DIN MIDI IN**: Receives notes from TR-08 ✓
- [ ] **SYNC_IN**: Receives 1 PPQN from Beatstep Pro ✓
- [ ] **SYNC_IN**: Multiplies to 24 PPQN correctly ✓

### Output Ports
- [ ] **USB MIDI OUT**: Forwards DIN clock to Ableton ✓
- [ ] **USB MIDI OUT**: Forwards DIN notes to Ableton ✓
- [ ] **USB MIDI OUT**: Outputs clock from SYNC_IN to Ableton ✓
- [ ] **DIN MIDI OUT**: Forwards USB clock to TR-08 ✓
- [ ] **DIN MIDI OUT**: Forwards USB notes to TR-08 ✓
- [ ] **DIN MIDI OUT**: Outputs clock from SYNC_IN to TR-08 ✓
- [ ] **SYNC_OUT**: Outputs divided clock to Volca ✓
- [ ] **SYNC_OUT**: Rate controlled by rotary switch ✓
- [ ] **DISPLAY_CLK**: Outputs 1 PPQN to TinyPulse ✓
- [ ] **DISPLAY_CLK**: Independent of rotary switch ✓
- [ ] **LED**: Pulses at SYNC_OUT rate ✓

### Bidirectional Tests
- [ ] USB → DIN while DIN → USB (simultaneous) ✓
- [ ] Clock + Notes simultaneously ✓
- [ ] Multiple MIDI channels work ✓
- [ ] SysEx pass-through works ✓

---

## Troubleshooting Guide

### TinyPulse Shows Wrong BPM
- ✅ Check rotary switch position matches device PPQN rate
- ✅ Beatstep Pro = Position 1 (1 PPQN)
- ✅ Volca, Pocket Operator = Position 2 (2 PPQN)
- ✅ MIDI devices = any position (MIDI is always 24 PPQN internally)

### TinyPulse Shows "----"
- ✅ Check clock source is playing/running
- ✅ Check cables are connected
- ✅ Wait 2-3 beats for initial calculation

### Unstable BPM Reading (Jumping Values)
- ✅ Check for loose cables
- ✅ Verify clean clock signal from source
- ✅ Wait 4-8 beats for progressive refinement to stabilize

### BytePulse LED Not Pulsing
- ✅ Check clock source is sending data
- ✅ Verify BytePulse is receiving clock (check serial debug)
- ✅ LED pulses at SYNC_OUT rate, not MIDI clock rate

### Device Not Syncing to BytePulse
- ✅ Check device is set to **external sync** mode
- ✅ Verify MIDI/SYNC cable is connected
- ✅ For SYNC_OUT: verify correct rotary position for device PPQN

### MIDI Notes Not Playing Through
- ✅ Check MIDI channel settings (transmit channel matches receive channel)
- ✅ Verify BytePulse is selected as MIDI device in DAW
- ✅ Check cables are in correct IN/OUT ports
- ✅ Enable SERIAL_DEBUG in config.h to monitor MIDI traffic

### Clock Forwarding Not Working
- ✅ Verify DAW has MIDI clock output enabled
- ✅ Check receiving device is set to external sync
- ✅ MIDI Start message must be sent (press Play in DAW)
- ✅ Some devices need both Clock + Start/Stop messages

### SYNC_OUT and SYNC_IN Conflict
- ✅ Rotary switch controls **both** directions simultaneously
- ✅ If input device is 1 PPQN but output device needs 2 PPQN:
  - Use MIDI outputs (USB/DIN) instead of SYNC_OUT
  - MIDI clock is always 24 PPQN regardless of rotary
- ✅ For mixed PPQN requirements, use dedicated clock converter

---

## Quick Reference: Rotary Switch Settings

### For SYNC_IN (Receiving from analog device)
- **Position 1**: Device sends 1 PPQN (Beatstep Pro, some modular)
- **Position 2**: Device sends 2 PPQN (Volca, PO, Korg SQ-1)
- **Position 3**: Device sends 4 PPQN (rare)
- **Position 4**: Device sends 6 PPQN (very rare)
- **Position 5**: Device sends 24 PPQN (converts to standard MIDI)

### For SYNC_OUT (Sending to analog device)
- **Position 1**: Device expects 1 PPQN
- **Position 2**: Device expects 2 PPQN (Volca, PO)
- **Position 3**: Device expects 4 PPQN
- **Position 4**: Device expects 6 PPQN
- **Position 5**: Device expects 24 PPQN (full MIDI rate)

**Note**: Rotary switch controls **both** SYNC_IN interpretation **and** SYNC_OUT division rate simultaneously.

---

## Complete Port Matrix

| Source | Destination | Data Type | Test Scenario | Status |
|--------|-------------|-----------|---------------|--------|
| Ableton USB | DIN OUT | Clock | Scenario 1, 9A | ⬜ |
| Ableton USB | DIN OUT | Notes | Scenario 6A | ⬜ |
| Ableton USB | DIN OUT | CC | Scenario 6B | ⬜ |
| Ableton USB | DISPLAY_CLK | 1 PPQN | Scenario 1 | ⬜ |
| Ableton USB | SYNC_OUT | Divided Clock | Scenario 5 | ⬜ |
| TR-08 DIN | USB OUT | Clock | Scenario 3, 9B | ⬜ |
| TR-08 DIN | USB OUT | Notes | Scenario 7A | ⬜ |
| TR-08 DIN | DISPLAY_CLK | 1 PPQN | Scenario 3 | ⬜ |
| Beatstep SYNC | USB OUT | Clock (24 PPQN) | Scenario 2, 9C | ⬜ |
| Beatstep SYNC | DIN OUT | Clock (24 PPQN) | Scenario 2, 9C | ⬜ |
| Beatstep SYNC | DISPLAY_CLK | 1 PPQN | Scenario 2 | ⬜ |
| Beatstep SYNC | SYNC_OUT | Divided Clock | Scenario 2 | ⬜ |
| Any Source | LED | Visual Pulse | All Scenarios | ⬜ |

Use this matrix to track your testing progress!

---

## Success Criteria

All scenarios should pass with:
- ✅ Accurate BPM display on TinyPulse (within ±1-2 BPM)
- ✅ Stable readings after 4-8 beats
- ✅ Correct clock priority enforcement
- ✅ Proper timeout and fallback behavior
- ✅ SYNC_OUT devices run at correct tempo with proper rotary setting
- ✅ MIDI forwarding works (DIN OUT mirrors USB IN and vice versa)
- ✅ Visual LED pulse confirmation
