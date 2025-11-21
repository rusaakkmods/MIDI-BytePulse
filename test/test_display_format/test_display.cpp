#include <unity.h>

// Character to 7-segment conversion from Display.cpp
uint8_t charToSegment(char c) {
    switch (c) {
        case '0': return 0b00111111;
        case '1': return 0b00000110;
        case '2': return 0b01011011;
        case '3': return 0b01001111;
        case '4': return 0b01100110;
        case '5': return 0b01101101;
        case '6': return 0b01111101;
        case '7': return 0b00000111;
        case '8': return 0b01111111;
        case '9': return 0b01101111;
        case 'A': case 'a': return 0b01110111;
        case 'B': case 'b': return 0b01111100;
        case 'C': case 'c': return 0b00111001;
        case 'D': case 'd': return 0b01011110;
        case 'E': case 'e': return 0b01111001;
        case 'F': case 'f': return 0b01110001;
        case 'G': case 'g': return 0b00111101;
        case 'H': case 'h': return 0b01110110;
        case 'I': case 'i': return 0b00000110;
        case 'J': case 'j': return 0b00011110;
        case 'L': case 'l': return 0b00111000;
        case 'N': case 'n': return 0b01010100;
        case 'O': case 'o': return 0b01011100;
        case 'P': case 'p': return 0b01110011;
        case 'S': case 's': return 0b01101101;
        case 'T': case 't': return 0b01111000;
        case 'U': case 'u': return 0b00011100;
        case 'Y': case 'y': return 0b01101110;
        case '-': return 0b01000000;
        case '_': return 0b00001000;
        default: return 0b00000000;
    }
}

// BPM formatting helper
struct BPMDisplay {
    uint8_t digit1;
    uint8_t digit2;
    uint8_t digit3;
    bool hasDot;
};

BPMDisplay formatBPM(uint16_t bpm) {
    BPMDisplay display;
    display.digit1 = (bpm / 100) % 10;
    display.digit2 = (bpm / 10) % 10;
    display.digit3 = bpm % 10;
    display.hasDot = true; // "t." format
    return display;
}

// Test digit conversion
void test_char_to_segment_digits() {
    TEST_ASSERT_EQUAL_UINT8(0b00111111, charToSegment('0'));
    TEST_ASSERT_EQUAL_UINT8(0b00000110, charToSegment('1'));
    TEST_ASSERT_EQUAL_UINT8(0b01011011, charToSegment('2'));
    TEST_ASSERT_EQUAL_UINT8(0b01001111, charToSegment('3'));
    TEST_ASSERT_EQUAL_UINT8(0b01100110, charToSegment('4'));
    TEST_ASSERT_EQUAL_UINT8(0b01101101, charToSegment('5'));
    TEST_ASSERT_EQUAL_UINT8(0b01111101, charToSegment('6'));
    TEST_ASSERT_EQUAL_UINT8(0b00000111, charToSegment('7'));
    TEST_ASSERT_EQUAL_UINT8(0b01111111, charToSegment('8'));
    TEST_ASSERT_EQUAL_UINT8(0b01101111, charToSegment('9'));
}

// Test letter conversion for "IdLE"
void test_char_to_segment_idle() {
    TEST_ASSERT_EQUAL_UINT8(0b00000110, charToSegment('I')); // Looks like '1'
    TEST_ASSERT_EQUAL_UINT8(0b01011110, charToSegment('d'));
    TEST_ASSERT_EQUAL_UINT8(0b00111000, charToSegment('L'));
    TEST_ASSERT_EQUAL_UINT8(0b01111001, charToSegment('e'));
}

// Test letter conversion for "t" (BPM display prefix)
void test_char_to_segment_t() {
    TEST_ASSERT_EQUAL_UINT8(0b01111000, charToSegment('t'));
    TEST_ASSERT_EQUAL_UINT8(0b01111000, charToSegment('T'));
}

// Test special characters
void test_char_to_segment_special() {
    TEST_ASSERT_EQUAL_UINT8(0b01000000, charToSegment('-'));
    TEST_ASSERT_EQUAL_UINT8(0b00001000, charToSegment('_'));
    TEST_ASSERT_EQUAL_UINT8(0b00000000, charToSegment('?')); // Unknown char
    TEST_ASSERT_EQUAL_UINT8(0b00000000, charToSegment(' ')); // Space
}

// Test BPM formatting for 120 BPM
void test_format_bpm_120() {
    BPMDisplay display = formatBPM(120);
    TEST_ASSERT_EQUAL_UINT8(1, display.digit1);
    TEST_ASSERT_EQUAL_UINT8(2, display.digit2);
    TEST_ASSERT_EQUAL_UINT8(0, display.digit3);
    TEST_ASSERT_TRUE(display.hasDot);
}

// Test BPM formatting for 60 BPM (two digits)
void test_format_bpm_60() {
    BPMDisplay display = formatBPM(60);
    TEST_ASSERT_EQUAL_UINT8(0, display.digit1);
    TEST_ASSERT_EQUAL_UINT8(6, display.digit2);
    TEST_ASSERT_EQUAL_UINT8(0, display.digit3);
}

// Test BPM formatting for 300 BPM (three digits)
void test_format_bpm_300() {
    BPMDisplay display = formatBPM(300);
    TEST_ASSERT_EQUAL_UINT8(3, display.digit1);
    TEST_ASSERT_EQUAL_UINT8(0, display.digit2);
    TEST_ASSERT_EQUAL_UINT8(0, display.digit3);
}

// Test BPM formatting for 999 BPM (max displayable)
void test_format_bpm_999() {
    BPMDisplay display = formatBPM(999);
    TEST_ASSERT_EQUAL_UINT8(9, display.digit1);
    TEST_ASSERT_EQUAL_UINT8(9, display.digit2);
    TEST_ASSERT_EQUAL_UINT8(9, display.digit3);
}

// Test BPM formatting for single digit
void test_format_bpm_5() {
    BPMDisplay display = formatBPM(5);
    TEST_ASSERT_EQUAL_UINT8(0, display.digit1);
    TEST_ASSERT_EQUAL_UINT8(0, display.digit2);
    TEST_ASSERT_EQUAL_UINT8(5, display.digit3);
}

// Test BPM formatting for edge cases
void test_format_bpm_edge_cases() {
    // Minimum BPM (30)
    BPMDisplay display30 = formatBPM(30);
    TEST_ASSERT_EQUAL_UINT8(0, display30.digit1);
    TEST_ASSERT_EQUAL_UINT8(3, display30.digit2);
    TEST_ASSERT_EQUAL_UINT8(0, display30.digit3);
    
    // Common BPMs
    BPMDisplay display128 = formatBPM(128);
    TEST_ASSERT_EQUAL_UINT8(1, display128.digit1);
    TEST_ASSERT_EQUAL_UINT8(2, display128.digit2);
    TEST_ASSERT_EQUAL_UINT8(8, display128.digit3);
    
    BPMDisplay display140 = formatBPM(140);
    TEST_ASSERT_EQUAL_UINT8(1, display140.digit1);
    TEST_ASSERT_EQUAL_UINT8(4, display140.digit2);
    TEST_ASSERT_EQUAL_UINT8(0, display140.digit3);
}

// Test case sensitivity
void test_char_case_insensitive() {
    // Letters should work in both upper and lower case
    TEST_ASSERT_EQUAL(charToSegment('A'), charToSegment('a'));
    TEST_ASSERT_EQUAL(charToSegment('E'), charToSegment('e'));
    TEST_ASSERT_EQUAL(charToSegment('T'), charToSegment('t'));
}

void setUp(void) {
    // Set up code here
}

void tearDown(void) {
    // Clean up code here
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    // Character conversion tests
    RUN_TEST(test_char_to_segment_digits);
    RUN_TEST(test_char_to_segment_idle);
    RUN_TEST(test_char_to_segment_t);
    RUN_TEST(test_char_to_segment_special);
    RUN_TEST(test_char_case_insensitive);
    
    // BPM formatting tests
    RUN_TEST(test_format_bpm_120);
    RUN_TEST(test_format_bpm_60);
    RUN_TEST(test_format_bpm_300);
    RUN_TEST(test_format_bpm_999);
    RUN_TEST(test_format_bpm_5);
    RUN_TEST(test_format_bpm_edge_cases);
    
    return UNITY_END();
}
