/**
 * MIDI BytePulse - Hardware Configuration
 * SparkFun Pro Micro (ATmega32U4, 5V, 16MHz)
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// MIDI Hardware UART
#define MIDI_IN_PIN         0
#define MIDI_OUT_PIN        1

// Clock Sync Output
#define SYNC_OUT_PIN          5   // Variable PPQN (switch-controlled)
#define DISPLAY_CLK_PIN       4  // Fixed 1 PPQN for TinyPulse Display

// Clock Sync Input (assumes 1 PPQN from external source)
#define SYNC_IN_PIN           7
#define SYNC_IN_DETECT_PIN    6

// Sync Rate Selector (1P6T rotary switch - controls BOTH SYNC_IN and SYNC_OUT)
// Sets the PPQN rate for analog sync signals in both directions:
//   SYNC_IN:  External clock → MIDI (multiply up to 24 PPQN)
//   SYNC_OUT: MIDI → External device (divide down from 24 PPQN)
// Wiring: Common to GND, each position connects to one GPIO pin
#define SYNC_RATE_PIN_1       9   // Position 1 = 1 PPQN (Modular, BeatStep Pro)
#define SYNC_RATE_PIN_2       10   // Position 2 = 2 PPQN (Korg Volca)
#define SYNC_RATE_PIN_3       16   // Position 3 = 4 PPQN (Roland DIN Sync)
#define SYNC_RATE_PIN_4       14   // Position 4 = 6 PPQN
#define SYNC_RATE_PIN_5       15   // Position 5 = 24 PPQN (MIDI passthrough)
#define SYNC_RATE_PIN_6       A0   // Position 6 = 48 PPQN (High-res)

// LED
#define LED_PULSE_PIN       8

// Debug
#define SERIAL_DEBUG        false
#define DEBUG_BAUD_RATE    115200

#if SERIAL_DEBUG
  #define DEBUG_PRINT(x)   Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif  // CONFIG_H
