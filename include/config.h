/**
 * MIDI BytePulse - Hardware Configuration
 * Pro Micro USB MIDI Sync Box - Pin Definitions and Constants
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>


// ============================================================================
// PIN DEFINITIONS (Pro Micro - SparkFun Pin Numbers)
// ============================================================================

// MIDI Hardware UART
#define MIDI_IN_PIN         0   // Pin 0 (RX1) - MIDI DIN IN via optocoupler
#define MIDI_OUT_PIN        1   // Pin 1 (TX1) - MIDI DIN OUT (reserved for future use)

// Clock Sync Output
#define CLOCK_OUT_PIN       5   // Pin 5 - Clock pulse output (via transistor Q1)
#define SYNC_DETECT_PIN     4   // Pin 4 - Jack plug detection (HIGH = inserted)

// Analog Inputs (Potentiometers)
#define POT_VOLUME_PIN     A0   // Pin 18 (A0) - Volume control (MIDI CC7)
#define POT_CUTOFF_PIN     A1   // Pin 19 (A1) - Filter cutoff control (MIDI CC74)
#define POT_RESONANCE_PIN  A2   // Pin 20 (A2) - Resonance control (MIDI CC71)

// Rotary Encoder (both pins support hardware interrupts)
#define ENCODER_A_PIN       2   // Pin 2 (INT2) - Encoder phase A 
#define ENCODER_B_PIN       3   // Pin 3 (INT0) - Encoder phase B
#define ENCODER_BTN_PIN     8   // Pin 8 - Encoder push button

// Buttons
#define BTN_FUNCTION_PIN    9   // Pin 9 - Function/Config button
#define BTN_PLAY_PIN       14   // Pin 14 - Play/Pause transport button
#define BTN_STOP_PIN       15   // Pin 15 - Stop transport button

// LEDs
#define LED_BEAT_PIN       16   // Pin 16 - Beat indicator LED (red)
// Note: Power LED on +5V rail (always on, not MCU-controlled)

// TM1637 7-Segment Display (4-digit)
#define TM1637_CLK_PIN      6   // Pin 6 - Clock
#define TM1637_DIO_PIN     10   // Pin 10 - Data I/O