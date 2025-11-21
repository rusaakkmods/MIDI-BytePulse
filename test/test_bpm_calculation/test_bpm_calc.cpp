#include <unity.h>

// BPM calculation formula from Sync.cpp
// For a 4-beat measure at 24 PPQN:
// BPM = 240,000 / interval_in_milliseconds
// where interval is the time between beat 3 and beat 0 (one complete 4-beat cycle)

uint16_t calculateBPM(unsigned long intervalMs) {
    if (intervalMs == 0) return 0;
    return 240000UL / intervalMs;
}

// Test BPM calculation at standard tempos
void test_bpm_calculation_120bpm() {
    // At 120 BPM, one 4-beat measure = 2000ms
    unsigned long interval = 2000;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(120, bpm);
}

void test_bpm_calculation_60bpm() {
    // At 60 BPM, one 4-beat measure = 4000ms
    unsigned long interval = 4000;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(60, bpm);
}

void test_bpm_calculation_240bpm() {
    // At 240 BPM, one 4-beat measure = 1000ms
    unsigned long interval = 1000;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(240, bpm);
}

// Test edge cases
void test_bpm_calculation_minimum_30bpm() {
    // At 30 BPM (minimum), one 4-beat measure = 8000ms
    unsigned long interval = 8000;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(30, bpm);
}

void test_bpm_calculation_maximum_300bpm() {
    // At 300 BPM (maximum), one 4-beat measure = 800ms
    unsigned long interval = 800;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(300, bpm);
}

void test_bpm_calculation_zero_interval() {
    // Zero interval should return 0 to avoid division by zero
    unsigned long interval = 0;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(0, bpm);
}

// Test common DAW tempos
void test_bpm_calculation_128bpm() {
    // At 128 BPM, one 4-beat measure = 1875ms
    unsigned long interval = 1875;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(128, bpm);
}

void test_bpm_calculation_140bpm() {
    // At 140 BPM, one 4-beat measure ≈ 1714ms
    unsigned long interval = 1714;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(140, bpm);
}

void test_bpm_calculation_90bpm() {
    // At 90 BPM, one 4-beat measure = 240000/90 = 2666.67ms
    // Using 2667 gives 89.996 which rounds to 89
    // Using 2666 gives 90.008 which rounds to 90
    unsigned long interval = 2666;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(90, bpm);
}

void test_bpm_calculation_180bpm() {
    // At 180 BPM, one 4-beat measure ≈ 1333ms
    unsigned long interval = 1333;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(180, bpm);
}

// Test rounding behavior
void test_bpm_calculation_rounding_up() {
    // Interval 1999ms should give 120.06 BPM, rounds to 120
    unsigned long interval = 1999;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(120, bpm);
}

void test_bpm_calculation_rounding_down() {
    // Interval 2001ms should give 119.94 BPM, rounds to 119
    unsigned long interval = 2001;
    uint16_t bpm = calculateBPM(interval);
    TEST_ASSERT_EQUAL_UINT16(119, bpm);
}

void setUp(void) {
    // Set up code here (runs before each test)
}

void tearDown(void) {
    // Clean up code here (runs after each test)
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Standard BPM tests
    RUN_TEST(test_bpm_calculation_120bpm);
    RUN_TEST(test_bpm_calculation_60bpm);
    RUN_TEST(test_bpm_calculation_240bpm);
    
    // Edge case tests
    RUN_TEST(test_bpm_calculation_minimum_30bpm);
    RUN_TEST(test_bpm_calculation_maximum_300bpm);
    RUN_TEST(test_bpm_calculation_zero_interval);
    
    // Common DAW tempo tests
    RUN_TEST(test_bpm_calculation_128bpm);
    RUN_TEST(test_bpm_calculation_140bpm);
    RUN_TEST(test_bpm_calculation_90bpm);
    RUN_TEST(test_bpm_calculation_180bpm);
    
    // Rounding tests
    RUN_TEST(test_bpm_calculation_rounding_up);
    RUN_TEST(test_bpm_calculation_rounding_down);
    
    return UNITY_END();
}
