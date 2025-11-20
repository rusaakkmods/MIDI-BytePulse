/**
 * MIDI BytePulse - Display Handler Implementation
 * Non-blocking updates using AceSegment library
 */

#include "Display.h"
#include "config.h"

using ace_tmi::SimpleTmi1637Interface;
using ace_segment::Tm1637Module;

void Display::begin() {
  // Create TM1637 interface with 100us bit delay - guaranteed to work with all modules
  // Even with capacitors, this is the safest setting from AceSegment examples
  tmiInterface = new SimpleTmi1637Interface(DISPLAY_DIO_PIN, DISPLAY_CLK_PIN, 100);
  ledModule = new Tm1637Module<SimpleTmi1637Interface, 4>(*tmiInterface);
  
  tmiInterface->begin();
  ledModule->begin();
  ledModule->setBrightness(2);  // Medium brightness (0-7)
  
  // Total animation: 2000ms - matching original behavior
  uint8_t segments[7] = {
    0b00000001,  // Top
    0b00000010,  // Top right
    0b00000100,  // Bottom right
    0b00001000,  // Bottom
    0b00010000,  // Bottom left
    0b00100000,  // Top left
    0b01000000   // Middle
  };
  
  // Segment cascade (10 frames, 1000ms)
  for (int frame = 0; frame < 10; frame++) {
    for (int digit = 0; digit < 4; digit++) {
      int segmentIdx = frame - digit;
      if (segmentIdx >= 0 && segmentIdx < 7) {
        ledModule->setPatternAt(digit, segments[segmentIdx]);
      } else {
        ledModule->setPatternAt(digit, 0b00000000);
      }
    }
    ledModule->flush();
    delay(100);
  }
  
  // Decimal points (4 frames, 600ms)
  for (int dp = 0; dp < 4; dp++) {
    for (int digit = 0; digit < 4; digit++) {
      ledModule->setPatternAt(digit, digit == dp ? 0b10000000 : 0b00000000);
    }
    ledModule->flush();
    delay(150);
  }
  
  // Blink all (2 blinks, 400ms)
  for (int blink = 0; blink < 2; blink++) {
    for (int digit = 0; digit < 4; digit++) {
      ledModule->setPatternAt(digit, 0xFF);
    }
    ledModule->flush();
    delay(100);
    
    for (int digit = 0; digit < 4; digit++) {
      ledModule->setPatternAt(digit, 0x00);
    }
    ledModule->flush();
    delay(100);
  }
}

void Display::showStandby() {
  if (ledModule) {
    // Show "StbY"
    uint8_t stby[] = {0b01101101, 0b01111000, 0b01111100, 0b01101110};
    for (int i = 0; i < 4; i++) {
      ledModule->setPatternAt(i, stby[i]);
    }
    ledModule->flush();
  }
}

void Display::updateClockIndicator(bool clockRunning) {
  // Not used with AceSegment - decimal is part of BPM pattern
}

void Display::setBPM(uint16_t bpm) {
  bpm = constrain(bpm, 20, 400);
  
  // Only update if changed by more than 2 BPM
  if (abs((int)bpm - (int)currentBPM) > 2) {
    currentBPM = bpm;
    
    if (ledModule) {
      // Convert BPM to 3 digits (right-aligned)
      uint8_t hundreds = (bpm / 100) % 10;
      uint8_t tens = (bpm / 10) % 10;
      uint8_t ones = bpm % 10;
      
      // Digit patterns for 0-9
      const uint8_t digitToSegment[10] = {
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01101111  // 9
      };
      
      // Right-align: "t" prefix, hundreds (or blank if 0), tens, ones with decimal
      ledModule->setPatternAt(0, 0b01111000);  // "t" prefix for tempo/BPM
      ledModule->setPatternAt(1, hundreds > 0 ? digitToSegment[hundreds] : 0b00000000);
      ledModule->setPatternAt(2, digitToSegment[tens]);
      ledModule->setPatternAt(3, digitToSegment[ones] | 0b10000000);  // Ones with decimal point
      
      // Patterns are set, flushIncremental() will handle the update
    }
  }
}

void Display::showClockIndicator() {
  // Show "t  0." when clock starts
  if (ledModule) {
    isIdle = false;  // Stop idle animation
    showingMIDIMessage = false;  // Stop showing MIDI messages
    ledModule->setPatternAt(0, 0b01111000);  // "t" prefix
    ledModule->setPatternAt(1, 0b00000000);
    ledModule->setPatternAt(2, 0b00000000);
    ledModule->setPatternAt(3, 0b00111111 | 0b10000000);  // 0 with decimal
    currentBPM = 0;
  }
}

void Display::setSource(const char* source) {
  // Optional: Show source indicator
}

uint8_t Display::charToSegment(char c) {
  // Convert character to 7-segment pattern
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
    case 'N': case 'n': return 0b01010100;
    case 'o': case 'O': return 0b01011100;
    case 'P': case 'p': return 0b01110011;
    case 't': case 'T': return 0b01111000;
    case '.': return 0b10000000;  // Decimal point
    case '#': return 0b00000000;  // Placeholder (channel will be shown)
    default: return 0b00000000;
  }
}

void Display::showMIDIMessage(const char* type, uint8_t data, uint8_t channel) {
  // Show MIDI message briefly when idle
  // Note On: "1n.3C" (channel 1, 'n' with decimal, note 0x3C)
  if (ledModule && isIdle) {
    showingMIDIMessage = true;
    midiMessageTime = millis();
    
    // Convert hex data to two chars (e.g., 0x7F -> "7F")
    char hex1 = (data >> 4) < 10 ? '0' + (data >> 4) : 'A' + (data >> 4) - 10;
    char hex2 = (data & 0x0F) < 10 ? '0' + (data & 0x0F) : 'A' + (data & 0x0F) - 10;
    
    // First digit: channel in hex (0-F)
    char channelHex = channel < 10 ? '0' + channel : 'A' + channel - 10;
    
    // Second digit: 'n' with decimal point
    uint8_t nWithDecimal = charToSegment('n') | 0b10000000;
    
    ledModule->setPatternAt(0, charToSegment(channelHex));
    ledModule->setPatternAt(1, nWithDecimal);
    ledModule->setPatternAt(2, charToSegment(hex1));
    ledModule->setPatternAt(3, charToSegment(hex2));
  }
}

void Display::clear() {
  if (ledModule) {
    // Start idle animation
    isIdle = true;
    idleAnimFrame = 0;
    lastIdleAnimTime = millis();
    currentBPM = 0;
  }
}

void Display::flush() {
  // Call flushIncremental() every 20ms like the AceSegment examples
  // This updates one digit at a time in a round-robin fashion
  if (ledModule) {
    unsigned long now = millis();
    if ((unsigned long)(now - lastFlushTime) >= 20) {
      lastFlushTime = now;
      ledModule->flushIncremental();
    }
    
    // Handle idle animation
    if (isIdle && (unsigned long)(now - lastIdleAnimTime) >= 100) {
      lastIdleAnimTime = now;
      
      // Check if we should resume animation after showing MIDI message
      if (showingMIDIMessage && (unsigned long)(now - midiMessageTime) >= 1000) {
        showingMIDIMessage = false;
      }
      
      // Only animate if not showing MIDI message
      if (!showingMIDIMessage) {
        // Rotating segments animation (6 frames)
        const uint8_t rotatePattern[6] = {
          0b00000001,  // Top
          0b00000010,  // Top right
          0b00000100,  // Bottom right
          0b00001000,  // Bottom
          0b00010000,  // Bottom left
          0b00100000   // Top left
        };
        
        // Show rotating pattern on all 4 digits with offset
        for (int i = 0; i < 4; i++) {
          uint8_t frame = (idleAnimFrame + i) % 6;
          ledModule->setPatternAt(i, rotatePattern[frame]);
        }
        
        idleAnimFrame = (idleAnimFrame + 1) % 6;
      }
    }
  }
}
