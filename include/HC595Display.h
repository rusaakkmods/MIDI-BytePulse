/**
 * 74HC595 Dual Shift Register 7-Segment Display Driver
 * Hardware SPI + Timer-based multiplexing for non-blocking operation
 */

#ifndef HC595_DISPLAY_H
#define HC595_DISPLAY_H

#include <Arduino.h>

class HC595Display {
public:
    HC595Display(uint8_t latchPin);
    
    void begin();
    void clear();
    void setBrightness(uint8_t level);  // 0-15 (PWM via multiplexing duty cycle)
    
    // Display control
    void showNumber(uint16_t number, bool leadingZeros = false);
    void showCounter(uint8_t count);  // 0-31 counter
    void setDigit(uint8_t position, uint8_t digit, bool decimalPoint = false);
    void setSegments(uint8_t position, uint8_t segments, bool decimalPoint = false);
    
    // Beat indicator
    void setDecimalPoint(uint8_t position, bool on);
    void allDecimalsOff();
    
    // Low-level access
    void updateDisplay();  // Call this in main loop (optional - ISR handles it)
    
    // Public for ISR access
    uint8_t _displayBuffer[4];  // Segment data for each digit
    static HC595Display* _instance;
    
private:
    uint8_t _latchPin;
    uint8_t _brightness;
    
    static const uint8_t DIGIT_PATTERNS[10];
    
    void shiftOutBitBang(uint8_t data);  // For testing
};

// Timer setup (called from main.cpp)
void setupDisplayTimer();

#endif // HC595_DISPLAY_H
