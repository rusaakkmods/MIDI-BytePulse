# BytePulse Unit Tests

This directory contains automated unit tests for the BytePulse MIDI clock sync device.

## Running Tests

### Run all tests:
```bash
pio test -e native
```

### Run specific test suite:
```bash
pio test -e native -f test_bpm_calculation
pio test -e native -f test_clock_priority  
pio test -e native -f test_display_format
```

### Expected Results:
- **test_bpm_calculation**: 12 tests, 0 failures
- **test_clock_priority**: 7 tests, 0 failures
- **test_display_format**: 11 tests, 0 failures

**Total: 30 unit tests**

## Test Suites

### 1. test_bpm_calculation
Tests BPM calculation algorithm (240,000 / interval).

**Coverage:**
- Standard tempos (60, 90, 120, 128, 140, 180, 240 BPM)
- Edge cases (30 BPM minimum, 300 BPM maximum)
- Zero interval handling
- Rounding behavior

### 2. test_clock_priority
Tests clock source priority logic (Sync In > USB > DIN).

**Coverage:**
- Priority blocking (SYNC_IN blocks USB/DIN, USB blocks DIN)
- Fallback behavior when higher priority stops
- Initial state handling
- Complete priority chain

### 3. test_display_format
Tests 7-segment display formatting and character conversion.

**Coverage:**
- Digit conversion (0-9)
- Letter conversion (I, d, L, e, t, etc.)
- Special characters (-, _)
- BPM formatting (5-999 BPM range)
- Case insensitivity

## Framework

These tests use the **Unity Test Framework** (ThrowTheSwitch).
- Tests run natively on your computer (not embedded device)
- Fast execution, no hardware required
- Ideal for CI/CD integration

## Adding New Tests

1. Create new directory: `test/test_<feature>/`
2. Add test file: `test_<feature>.cpp`
3. Include Unity: `#include <unity.h>`
4. Write test functions: `void test_something() { ... }`
5. Run with: `pio test -e native -f test_<feature>`

## Continuous Integration

Unit tests should be run:
- ✅ Before every commit
- ✅ Before hardware testing
- ✅ In CI/CD pipeline
- ✅ After any core logic changes

## See Also

- [TESTING_GUIDE.md](TESTING_GUIDE.md) - Complete testing documentation
- [Unity Framework Docs](https://github.com/ThrowTheSwitch/Unity)
