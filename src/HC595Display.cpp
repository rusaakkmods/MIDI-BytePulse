/**
 * 74HC595 Dual Shift Register 7-Segment Display Driver
 * Hardware SPI + Timer-based multiplexing for non-blocking operation
 */

#include "HC595Display.h"
#include "config.h"
#include <SPI.h>

// 7-Segment patterns for digits 0-9 (common cathode - normal)
// Bit order: DP G F E D C B A
const uint8_t HC595Display::DIGIT_PATTERNS[10] = {
    0xC0,  // 0
    0xF9,  // 1
    0xA4,  // 2
    0xB0,  // 3
    0x99,  // 4
    0x92,  // 5
    0x82,  // 6
    0xF8,  // 7
    0x80,  // 8
    0x90   // 9
};

// Static instance for ISR access
HC595Display* HC595Display::_instance = nullptr;

// Timer4 ISR for digit multiplexing (ATmega32U4 doesn't have Timer2)
// Common anode: digit LOW = ON, segment LOW = ON
ISR(TIMER4_COMPA_vect) {
    if (HC595Display::_instance) {
        static uint8_t currentDigit = 0;
        
        // Get segment data for current digit
        uint8_t segments = HC595Display::_instance->_displayBuffer[currentDigit];
        // Try lower nibble instead (bits 0-3)
        uint8_t digitMask = ~(1 << currentDigit);  // Inverted: 0 = ON for common anode
        
        // Shift out data: segments first, then digits (reverse order)
        digitalWrite(DISPLAY_RCLK_PIN, LOW);
        SPI.transfer(segments);     // Segment pattern (first 595)
        SPI.transfer(digitMask);    // Digit select inverted (second 595)
        digitalWrite(DISPLAY_RCLK_PIN, HIGH);
        
        // Next digit
        currentDigit = (currentDigit + 1) & 0x03;  // Wrap 0-3
    }
}

HC595Display::HC595Display(uint8_t latchPin) 
    : _latchPin(latchPin), _brightness(15) {
    _instance = this;
    for (uint8_t i = 0; i < 4; i++) {
        _displayBuffer[i] = 0;
    }
}

void HC595Display::begin() {
    // Configure pins
    pinMode(_latchPin, OUTPUT);
    pinMode(DISPLAY_DIO_PIN, OUTPUT);
    pinMode(DISPLAY_SCLK_PIN, OUTPUT);
    
    digitalWrite(_latchPin, LOW);
    
    clear();
    
    // Test: Show "8" on first digit using the same pattern as the example
    // Digit mapping: 1=8, 2=4, 3=2, 4=1
    shiftOutBitBang(0x80);  // Segments for "8"
    shiftOutBitBang(8);     // Digit 1 (bit 3)
    digitalWrite(_latchPin, HIGH);
    digitalWrite(_latchPin, LOW);
}

void HC595Display::shiftOutBitBang(uint8_t data) {
    for (int i = 7; i >= 0; i--) {
        digitalWrite(DISPLAY_SCLK_PIN, LOW);
        digitalWrite(DISPLAY_DIO_PIN, (data >> i) & 0x01);
        digitalWrite(DISPLAY_SCLK_PIN, HIGH);
    }
}

void HC595Display::clear() {
    for (uint8_t i = 0; i < 4; i++) {
        _displayBuffer[i] = 0xFF;  // All segments off
    }
}

void HC595Display::setBrightness(uint8_t level) {
    _brightness = constrain(level, 0, 15);
    // Note: Full hardware PWM brightness control would require more complex timing
    // For now, brightness is always maximum. Can be implemented via duty cycle.
}

void HC595Display::showNumber(uint16_t number, bool leadingZeros) {
    if (number > 9999) number = 9999;
    
    uint8_t digits[4];
    digits[0] = number / 1000;
    digits[1] = (number / 100) % 10;
    digits[2] = (number / 10) % 10;
    digits[3] = number % 10;
    
    for (uint8_t i = 0; i < 4; i++) {
        if (!leadingZeros && i < 3 && digits[i] == 0 && number < pow(10, 3 - i)) {
            _displayBuffer[i] = 0;  // Blank leading zeros
        } else {
            _displayBuffer[i] = DIGIT_PATTERNS[digits[i]];
        }
    }
}

void HC595Display::showCounter(uint8_t count) {
    count = count % 32;  // 0-31
    
    if (count < 10) {
        // Single digit: show in rightmost position
        _displayBuffer[0] = 0;
        _displayBuffer[1] = 0;
        _displayBuffer[2] = 0;
        _displayBuffer[3] = DIGIT_PATTERNS[count];
    } else {
        // Two digits
        _displayBuffer[0] = 0;
        _displayBuffer[1] = 0;
        _displayBuffer[2] = DIGIT_PATTERNS[count / 10];
        _displayBuffer[3] = DIGIT_PATTERNS[count % 10];
    }
}

void HC595Display::setDigit(uint8_t position, uint8_t digit, bool decimalPoint) {
    if (position > 3 || digit > 9) return;
    
    _displayBuffer[position] = DIGIT_PATTERNS[digit];
    if (decimalPoint) {
        _displayBuffer[position] &= 0x7F;  // Clear bit 7 for DP
    }
}

void HC595Display::setSegments(uint8_t position, uint8_t segments, bool decimalPoint) {
    if (position > 3) return;
    
    _displayBuffer[position] = segments;
    if (decimalPoint) {
        _displayBuffer[position] &= 0x7F;  // Clear bit 7 for DP
    }
}

void HC595Display::setDecimalPoint(uint8_t position, bool on) {
    if (position > 3) return;
    
    if (on) {
        _displayBuffer[position] &= 0x7F;  // Clear bit = ON
    } else {
        _displayBuffer[position] |= 0x80;  // Set bit = OFF
    }
}

void HC595Display::allDecimalsOff() {
    for (uint8_t i = 0; i < 4; i++) {
        _displayBuffer[i] |= 0x80;  // Set bit = OFF
    }
}

void HC595Display::updateDisplay() {
    // Rapid multiplexing - call this frequently from main loop
    // Digit mapping from example: 1=8, 2=4, 3=2, 4=1
    static uint8_t currentDigit = 0;
    static const uint8_t digitBits[4] = {8, 4, 2, 1};  // Digit 0,1,2,3 mapping
    
    // Send segments first, digit select second
    digitalWrite(_latchPin, LOW);
    shiftOutBitBang(_displayBuffer[currentDigit]);
    shiftOutBitBang(digitBits[currentDigit]);
    digitalWrite(_latchPin, HIGH);
    
    currentDigit = (currentDigit + 1) & 0x03;  // Wrap 0-3
}
