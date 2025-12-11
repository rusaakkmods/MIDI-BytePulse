# BytePulse Unit Tests

This directory contains automated unit tests for the BytePulse MIDI clock router and sync converter.

**Total Coverage: 21 tests, 100% pass rate**

## Running Tests

### Run all tests:
```bash
pio test -e native
```

### Run specific test suite:
```bash
pio test -e native -f test_clock_priority
pio test -e native -f test_sync_rate
```

### Expected Results:
- **test_clock_priority**: 7 tests, 0 failures
- **test_sync_rate**: 14 tests, 0 failures

## Test Suites

### 1. test_clock_priority ✅ Active (7 tests)
Tests automatic clock source priority system.

**Purpose:** Validates SYNC_IN > USB > DIN priority hierarchy

**Coverage:**
- Priority blocking (SYNC_IN blocks USB/DIN, USB blocks DIN)
- Graceful fallback when higher priority stops
- Initial state handling (accepts any source when idle)
- Complete priority chain validation

**Status:** All 7 tests passing

### 2. test_sync_rate ✅ Active (14 tests)
Tests bidirectional PPQN rate conversion.

**Purpose:** Validates SYNC_IN multiplication and SYNC_OUT division

**Coverage:**
- SYNC_IN multipliers (1, 2, 4, 6, 24 PPQN → 24 PPQN MIDI)
- SYNC_OUT divisors (24 PPQN MIDI → 1, 2, 4, 24 PPQN)
- Real-world scenarios (Volca, BeatStep Pro, DAW routing)
- Bidirectional consistency verification

**Status:** All 14 tests passing

---

## Framework

These tests use the **Unity Test Framework** (ThrowTheSwitch).
- Tests run natively on your computer (not embedded device)
- Fast execution (~4 seconds for all 21 tests)
- No hardware required for validation
- Ideal for CI/CD integration

## Test Execution Details

**Platform:** native (x86/x64)
**Compiler:** GCC/MinGW (auto-installed by PlatformIO)
**Framework:** Unity 2.6.0
**Toolchain:** toolchain-gccmingw32 @ 1.50100.0

## Adding New Tests

1. Create new directory: `test/test_<feature>/`
2. Add test file: `test_<feature>.cpp`
3. Include Unity: `#include <unity.h>`
4. Write test functions: `void test_something() { TEST_ASSERT_EQUAL(...); }`
5. Register in main(): `RUN_TEST(test_something);`
6. Run with: `pio test -e native -f test_<feature>`

## Continuous Integration

Unit tests should be run:
- ✅ Before every commit
- ✅ Before hardware testing
- ✅ In CI/CD pipeline
- ✅ After any core logic changes
- ✅ When modifying clock priority or sync rate conversion

## Troubleshooting

**GCC Toolchain Missing:**
```bash
pio pkg install --tool "toolchain-gccmingw32" --platform native
```

**Tests fail on Windows:**
- Ensure MinGW GCC is used (not MSVC)
- Update PlatformIO: `pio upgrade`

## See Also

- [TESTING_GUIDE.md](TESTING_GUIDE.md) - Detailed test documentation with scenarios
- [Unity Framework Docs](https://github.com/ThrowTheSwitch/Unity)
- [BytePulse Summary](../documents/BytePulseSummary.md) - Feature overview

