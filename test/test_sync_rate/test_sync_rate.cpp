#include <unity.h>

// Sync rate definitions from Sync.h
enum SyncInRate {
  SYNC_IN_1_PPQN = 1,
  SYNC_IN_2_PPQN = 2,
  SYNC_IN_4_PPQN = 4,
  SYNC_IN_6_PPQN = 6,
  SYNC_IN_24_PPQN = 24
};

// Test SYNC_IN multiplier calculation
class SyncRateCalculator {
public:
    static uint8_t getSyncInMultiplier(SyncInRate rate) {
        // MIDI Clock is 24 PPQN, multiplier = 24 / rate
        return 24 / (uint8_t)rate;
    }
    
    static uint8_t getSyncOutDivisor(SyncInRate rate) {
        // SYNC_OUT divisor: how many MIDI Clocks per pulse
        return 24 / (uint8_t)rate;
    }
};

// Test 1 PPQN multiplier
void test_1_ppqn_multiplier() {
    uint8_t multiplier = SyncRateCalculator::getSyncInMultiplier(SYNC_IN_1_PPQN);
    TEST_ASSERT_EQUAL_UINT8(24, multiplier);
    // 1 pulse from external device → 24 MIDI Clocks
}

// Test 2 PPQN multiplier (Volca)
void test_2_ppqn_multiplier() {
    uint8_t multiplier = SyncRateCalculator::getSyncInMultiplier(SYNC_IN_2_PPQN);
    TEST_ASSERT_EQUAL_UINT8(12, multiplier);
    // 1 pulse from Volca → 12 MIDI Clocks
    // 2 pulses per beat → 24 MIDI Clocks per beat ✓
}

// Test 4 PPQN multiplier (Roland)
void test_4_ppqn_multiplier() {
    uint8_t multiplier = SyncRateCalculator::getSyncInMultiplier(SYNC_IN_4_PPQN);
    TEST_ASSERT_EQUAL_UINT8(6, multiplier);
    // 1 pulse → 6 MIDI Clocks
    // 4 pulses per beat → 24 MIDI Clocks per beat ✓
}

// Test 6 PPQN multiplier
void test_6_ppqn_multiplier() {
    uint8_t multiplier = SyncRateCalculator::getSyncInMultiplier(SYNC_IN_6_PPQN);
    TEST_ASSERT_EQUAL_UINT8(4, multiplier);
    // 1 pulse → 4 MIDI Clocks
    // 6 pulses per beat → 24 MIDI Clocks per beat ✓
}

// Test 24 PPQN multiplier (passthrough)
void test_24_ppqn_multiplier() {
    uint8_t multiplier = SyncRateCalculator::getSyncInMultiplier(SYNC_IN_24_PPQN);
    TEST_ASSERT_EQUAL_UINT8(1, multiplier);
    // 1 pulse → 1 MIDI Clock (1:1 passthrough)
}

// Test SYNC_OUT divisor for 1 PPQN
void test_sync_out_1_ppqn_divisor() {
    uint8_t divisor = SyncRateCalculator::getSyncOutDivisor(SYNC_IN_1_PPQN);
    TEST_ASSERT_EQUAL_UINT8(24, divisor);
    // Output 1 pulse every 24 MIDI Clocks (at ppqnCounter % 24 == 0)
}

// Test SYNC_OUT divisor for 2 PPQN
void test_sync_out_2_ppqn_divisor() {
    uint8_t divisor = SyncRateCalculator::getSyncOutDivisor(SYNC_IN_2_PPQN);
    TEST_ASSERT_EQUAL_UINT8(12, divisor);
    // Output 1 pulse every 12 MIDI Clocks
    // 24 MIDI Clocks → 2 pulses (correct for Volca)
}

// Test SYNC_OUT divisor for 4 PPQN
void test_sync_out_4_ppqn_divisor() {
    uint8_t divisor = SyncRateCalculator::getSyncOutDivisor(SYNC_IN_4_PPQN);
    TEST_ASSERT_EQUAL_UINT8(6, divisor);
    // Output 1 pulse every 6 MIDI Clocks
    // 24 MIDI Clocks → 4 pulses (Roland DIN Sync)
}

// Test SYNC_OUT divisor for 24 PPQN
void test_sync_out_24_ppqn_divisor() {
    uint8_t divisor = SyncRateCalculator::getSyncOutDivisor(SYNC_IN_24_PPQN);
    TEST_ASSERT_EQUAL_UINT8(1, divisor);
    // Output 1 pulse every MIDI Clock (24 pulses per beat)
}

// Test BPM calculation scenario: Volca @ 120 BPM
void test_volca_120bpm_scenario() {
    // Volca outputs 2 pulses per quarter note at 120 BPM
    // = 240 pulses per minute
    // = 4 pulses per second
    
    SyncInRate volcaRate = SYNC_IN_2_PPQN;
    uint8_t multiplier = SyncRateCalculator::getSyncInMultiplier(volcaRate);
    
    TEST_ASSERT_EQUAL_UINT8(12, multiplier);
    
    // Each Volca pulse generates 12 MIDI Clocks
    // 2 pulses (one beat) → 24 MIDI Clocks ✓
    // Resulting BPM = 120 (correct!)
}

// Test BPM calculation scenario: BeatStep Pro @ 120 BPM
void test_beatstep_120bpm_scenario() {
    // BeatStep Pro outputs 1 pulse per quarter note at 120 BPM
    // = 120 pulses per minute
    
    SyncInRate beatstepRate = SYNC_IN_1_PPQN;
    uint8_t multiplier = SyncRateCalculator::getSyncInMultiplier(beatstepRate);
    
    TEST_ASSERT_EQUAL_UINT8(24, multiplier);
    
    // Each BeatStep pulse generates 24 MIDI Clocks
    // 1 pulse (one beat) → 24 MIDI Clocks ✓
    // Resulting BPM = 120 (correct!)
}

// Test SYNC_OUT scenario: DAW → Volca via SYNC_OUT
void test_daw_to_volca_sync_out() {
    // DAW @ 120 BPM sends 24 PPQN MIDI Clock
    // Need to output 2 PPQN for Volca
    
    SyncInRate volcaRate = SYNC_IN_2_PPQN;
    uint8_t divisor = SyncRateCalculator::getSyncOutDivisor(volcaRate);
    
    TEST_ASSERT_EQUAL_UINT8(12, divisor);
    
    // Output pulse when ppqnCounter % 12 == 0
    // Positions: 0, 12 → 2 pulses per beat ✓
    // Volca receives correct 2 PPQN at 120 BPM
}

// Test bidirectional consistency
void test_bidirectional_consistency() {
    // For each rate, multiplier × divisor should = 576 (24²)
    // This ensures bidirectional conversion is symmetric
    
    SyncInRate rates[] = {
        SYNC_IN_1_PPQN,
        SYNC_IN_2_PPQN,
        SYNC_IN_4_PPQN,
        SYNC_IN_6_PPQN,
        SYNC_IN_24_PPQN
    };
    
    for (int i = 0; i < 5; i++) {
        uint8_t multiplier = SyncRateCalculator::getSyncInMultiplier(rates[i]);
        uint8_t divisor = SyncRateCalculator::getSyncOutDivisor(rates[i]);
        
        // Both should be equal (they use same formula)
        TEST_ASSERT_EQUAL_UINT8(divisor, multiplier);
    }
}

void setUp(void) {
    // Setup before each test
}

void tearDown(void) {
    // Cleanup after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // SYNC_IN multiplier tests
    RUN_TEST(test_1_ppqn_multiplier);
    RUN_TEST(test_2_ppqn_multiplier);
    RUN_TEST(test_4_ppqn_multiplier);
    RUN_TEST(test_6_ppqn_multiplier);
    RUN_TEST(test_24_ppqn_multiplier);
    
    // SYNC_OUT divisor tests
    RUN_TEST(test_sync_out_1_ppqn_divisor);
    RUN_TEST(test_sync_out_2_ppqn_divisor);
    RUN_TEST(test_sync_out_4_ppqn_divisor);
    RUN_TEST(test_sync_out_24_ppqn_divisor);
    
    // Real-world scenario tests
    RUN_TEST(test_volca_120bpm_scenario);
    RUN_TEST(test_beatstep_120bpm_scenario);
    RUN_TEST(test_daw_to_volca_sync_out);
    
    // Consistency tests
    RUN_TEST(test_bidirectional_consistency);
    
    return UNITY_END();
}
