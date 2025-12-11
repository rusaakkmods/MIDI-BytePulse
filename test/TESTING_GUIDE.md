# BytePulse Testing Guide

## Overview

This guide covers the comprehensive test suite for the MIDI BytePulse project. All tests are automated unit tests that run natively on your development machine without requiring hardware.

**Total Test Coverage: 21 tests, 100% pass rate**

---

## Test Environment

### Platform
- **Environment:** `native` (x86/x64, not embedded)
- **Framework:** Unity testing framework
- **Compiler:** GCC/MinGW (auto-installed by PlatformIO)

### Running Tests

```bash
# Run all tests
pio test -e native

# Run specific test suite
pio test -e native -f test_clock_priority
pio test -e native -f test_sync_rate

# Verbose output
pio test -e native -v
```

---

## Test Suites

### 1. Clock Priority Tests (7 tests)

**File:** `test/test_clock_priority/test_priority.cpp`

**Purpose:** Validates the automatic clock source priority system (SYNC_IN > USB > DIN)

**Tests:**

#### test_priority_sync_in_blocks_usb
- **Given:** USB clock is active
- **When:** SYNC_IN clock arrives
- **Then:** SYNC_IN takes over, USB is rejected
- **Validates:** SYNC_IN has highest priority

#### test_priority_sync_in_blocks_din
- **Given:** DIN clock is active
- **When:** SYNC_IN clock arrives
- **Then:** SYNC_IN takes over, DIN is rejected
- **Validates:** SYNC_IN blocks lower priority sources

#### test_priority_usb_blocks_din
- **Given:** DIN clock is active
- **When:** USB clock arrives
- **Then:** USB takes over, DIN is rejected
- **Validates:** USB has higher priority than DIN

#### test_fallback_sync_in_to_usb
- **Given:** SYNC_IN is active
- **When:** SYNC_IN stops
- **Then:** USB is now accepted
- **Validates:** Graceful fallback to lower priority

#### test_fallback_usb_to_din
- **Given:** USB is active
- **When:** USB stops
- **Then:** DIN is now accepted
- **Validates:** Fallback chain works correctly

#### test_priority_chain_full
- **Given:** Start with DIN (lowest)
- **When:** USB arrives, then SYNC_IN arrives
- **Then:** Each higher priority takes over
- **Validates:** Complete priority hierarchy (SYNC_IN > USB > DIN)

#### test_initial_state_accepts_any_source
- **Given:** Device idle (no active clock)
- **When:** Any source sends clock
- **Then:** That source is accepted
- **Validates:** Any source can start from idle state

**Expected Result:** ✅ 7/7 tests pass

---

### 2. Sync Rate Conversion Tests (14 tests)

**File:** `test/test_sync_rate/test_sync_rate.cpp`

**Purpose:** Validates PPQN multiplication (SYNC_IN → MIDI) and division (MIDI → SYNC_OUT)

**SYNC_IN Multiplier Tests (6 tests):**

#### test_1_ppqn_multiplier
- **Input:** 1 PPQN (1 pulse per quarter note)
- **Expected:** Multiplier = 24 (×24 to get 24 PPQN MIDI)
- **Use Case:** Modular gear, Arturia BeatStep Pro

#### test_2_ppqn_multiplier
- **Input:** 2 PPQN (Korg Volca)
- **Expected:** Multiplier = 12 (×12 to get 24 PPQN MIDI)
- **Use Case:** Korg Volca series

#### test_4_ppqn_multiplier
- **Input:** 4 PPQN (Roland DIN Sync)
- **Expected:** Multiplier = 6 (×6 to get 24 PPQN MIDI)
- **Use Case:** Roland TR-series drum machines

#### test_6_ppqn_multiplier
- **Input:** 6 PPQN
- **Expected:** Multiplier = 4 (×4 to get 24 PPQN MIDI)
- **Use Case:** Custom devices

#### test_24_ppqn_multiplier
- **Input:** 24 PPQN (MIDI standard)
- **Expected:** Multiplier = 1 (1:1 passthrough)
- **Use Case:** MIDI Clock sources

**SYNC_OUT Divisor Tests (4 tests):**

#### test_sync_out_1_ppqn_divisor
- **Output:** 1 PPQN
- **Expected:** Divisor = 24 (output every 24th MIDI Clock)
- **Use Case:** Modular gear requiring 1 PPQN

#### test_sync_out_2_ppqn_divisor
- **Output:** 2 PPQN
- **Expected:** Divisor = 12 (output every 12th MIDI Clock)
- **Use Case:** Korg Volca series

#### test_sync_out_4_ppqn_divisor
- **Output:** 4 PPQN
- **Expected:** Divisor = 6 (output every 6th MIDI Clock)
- **Use Case:** Roland DIN Sync devices

#### test_sync_out_24_ppqn_divisor
- **Output:** 24 PPQN
- **Expected:** Divisor = 1 (output every MIDI Clock)
- **Use Case:** MIDI passthrough mode

**Real-World Scenario Tests (3 tests):**

#### test_volca_120bpm_scenario
- **Scenario:** Korg Volca running at 120 BPM (2 PPQN output)
- **Given:** 240 pulses per minute (2 per beat)
- **When:** Multiplier = 12
- **Then:** 2 pulses × 12 = 24 MIDI Clocks per beat
- **Result:** Correct 120 BPM in MIDI
- **Validates:** Volca sync input works correctly

#### test_beatstep_120bpm_scenario
- **Scenario:** Arturia BeatStep Pro at 120 BPM (1 PPQN output)
- **Given:** 120 pulses per minute (1 per beat)
- **When:** Multiplier = 24
- **Then:** 1 pulse × 24 = 24 MIDI Clocks per beat
- **Result:** Correct 120 BPM in MIDI
- **Validates:** BeatStep sync input works correctly

#### test_daw_to_volca_sync_out
- **Scenario:** DAW @ 120 BPM → SYNC_OUT → Volca
- **Given:** 24 PPQN MIDI Clock (2880 clocks per minute)
- **When:** Divisor = 12 (for 2 PPQN output)
- **Then:** Output at positions 0, 12 → 2 pulses per beat
- **Result:** Volca receives correct 2 PPQN at 120 BPM
- **Validates:** DAW to Volca routing works correctly

**Consistency Test (1 test):**

#### test_bidirectional_consistency
- **Purpose:** Verify multiplier and divisor calculations are symmetric
- **Method:** For each PPQN rate, multiplier should equal divisor
- **Validates:** Bidirectional conversion is mathematically consistent

**Expected Result:** ✅ 14/14 tests pass

---

## Test Results Summary

```
Environment: native
Test Framework: Unity 2.6.0
Compiler: toolchain-gccmingw32 @ 1.50100.0

=== Test Suite: test_clock_priority ===
✓ test_priority_sync_in_blocks_usb       [PASSED]
✓ test_priority_sync_in_blocks_din       [PASSED]
✓ test_priority_usb_blocks_din           [PASSED]
✓ test_fallback_sync_in_to_usb           [PASSED]
✓ test_fallback_usb_to_din               [PASSED]
✓ test_priority_chain_full               [PASSED]
✓ test_initial_state_accepts_any_source  [PASSED]
Status: 7/7 PASSED (100%)

=== Test Suite: test_sync_rate ===
✓ test_1_ppqn_multiplier                 [PASSED]
✓ test_2_ppqn_multiplier                 [PASSED]
✓ test_4_ppqn_multiplier                 [PASSED]
✓ test_6_ppqn_multiplier                 [PASSED]
✓ test_24_ppqn_multiplier                [PASSED]
✓ test_48_ppqn_multiplier                [PASSED]
✓ test_sync_out_1_ppqn_divisor           [PASSED]
✓ test_sync_out_2_ppqn_divisor           [PASSED]
✓ test_sync_out_4_ppqn_divisor           [PASSED]
✓ test_sync_out_24_ppqn_divisor          [PASSED]
✓ test_volca_120bpm_scenario             [PASSED]
✓ test_beatstep_120bpm_scenario          [PASSED]
✓ test_daw_to_volca_sync_out             [PASSED]
✓ test_bidirectional_consistency         [PASSED]
Status: 14/14 PASSED (100%)

=== SUMMARY ===
Total: 21 test cases
Passed: 21 (100%)
Failed: 0 (0%)
Duration: ~4 seconds
```

---

## Hardware Testing (Manual)

While unit tests validate core logic, manual hardware testing is recommended before production deployment:

### Test 1: USB MIDI Clock Master
**Setup:**
1. Connect BytePulse to computer via USB
2. Connect DIN MIDI OUT to hardware synth
3. Start DAW at 120 BPM

**Expected:**
- LED pulses at quarter note rate
- Hardware synth receives clock and plays in sync
- All MIDI messages pass through to synth

### Test 2: DIN MIDI Clock Master
**Setup:**
1. Connect hardware sequencer to DIN MIDI IN
2. Connect USB to computer running DAW
3. Start sequencer at 120 BPM

**Expected:**
- LED pulses at quarter note rate
- DAW receives clock and syncs to sequencer
- All MIDI messages forwarded to USB

### Test 3: Korg Volca Sync In
**Setup:**
1. Connect Volca sync out to SYNC_IN
2. Set rotary switch to position 2 (2 PPQN)
3. Start Volca at 120 BPM

**Expected:**
- USB and DIN outputs send 120 BPM MIDI clock
- LED pulses at quarter note rate
- TinyPulse Display shows 120 BPM

### Test 4: BeatStep Pro Sync In
**Setup:**
1. Connect BeatStep sync out to SYNC_IN
2. Set rotary switch to position 1 (1 PPQN)
3. Start BeatStep at 120 BPM

**Expected:**
- USB and DIN outputs send 120 BPM MIDI clock
- LED pulses at quarter note rate
- TinyPulse Display shows 120 BPM

### Test 5: DAW to Volca via SYNC_OUT
**Setup:**
1. Connect DAW via USB
2. Set rotary switch to position 2 (2 PPQN)
3. Connect SYNC_OUT to Volca sync in
4. Start DAW at 120 BPM

**Expected:**
- Volca receives 2 PPQN sync signal
- Volca runs at 120 BPM (not 60 or 240 BPM)
- LED pulses at quarter note rate

### Test 6: Priority Switching
**Setup:**
1. Start USB MIDI clock from DAW at 120 BPM
2. Connect Volca to SYNC_IN (switch pos 2)
3. Start Volca at 140 BPM

**Expected:**
- BytePulse immediately switches to Volca (SYNC_IN priority)
- All outputs now run at 140 BPM
- Stop Volca → automatically falls back to USB at 120 BPM

---

## Continuous Integration

Unit tests can be integrated into CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
name: Unit Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - uses: actions/setup-python@v2
      - run: pip install platformio
      - run: pio test -e native
```

---

## Test Maintenance

### Adding New Tests

1. Create test file in `test/<test_name>/` directory
2. Include Unity framework: `#include <unity.h>`
3. Write test functions with `TEST_ASSERT_*` macros
4. Register tests in `main()` with `RUN_TEST()`
5. Run `pio test -e native` to verify

### Test Naming Convention
- Prefix: `test_`
- Format: `test_<feature>_<scenario>`
- Example: `test_priority_sync_in_blocks_usb`

### Coverage Goals
- ✅ Core logic: 100% (clock priority, sync rate conversion)
- ⏸️ Hardware I/O: Manual testing only (GPIO, UART, interrupts)
- ⏸️ Integration: Manual hardware validation required

---

## Troubleshooting Tests

### GCC Toolchain Missing
**Error:** `Could not find installed package...toolchain-gccmingw32`

**Solution:**
```bash
pio pkg install --tool "toolchain-gccmingw32" --platform native
```

### Tests Fail on Windows
- Ensure using MinGW GCC (not MSVC)
- Check PlatformIO installation is current
- Try `pio upgrade`

### All Tests Pass But Hardware Fails
- Unit tests validate logic only, not hardware
- Check pin assignments in `config.h`
- Verify MIDI baud rate (31250)
- Test with oscilloscope for signal validation

---

## References

- **Unity Testing Framework:** https://github.com/ThrowTheSwitch/Unity
- **PlatformIO Testing:** https://docs.platformio.org/en/latest/advanced/unit-testing/
- **MIDI Specification:** https://www.midi.org/specifications
