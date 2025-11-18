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

// Clock Sync Output
#define CLOCK_OUT_PIN       5   // Pin 5 - Clock pulse output (via transistor Q1)
#define SYNC_DETECT_PIN     4   // Pin 4 - Jack plug detection (HIGH = inserted)

// Analog Inputs (Potentiometers)
#define POT_VOLUME_PIN     A0   // Pin 18 (A0) - Volume control (MIDI CC7)
#define POT_CUTOFF_PIN     A1   // Pin 19 (A1) - Filter cutoff control (MIDI CC74)
#define POT_RESONANCE_PIN  A2   // Pin 20 (A2) - Resonance control (MIDI CC71)

// Rotary Encoder
#define ENCODER_A_PIN       6   // Pin 6 - Encoder phase A
#define ENCODER_B_PIN       7   // Pin 7 - Encoder phase B
#define ENCODER_BTN_PIN     8   // Pin 8 - Encoder push button

// Buttons
#define BTN_FUNCTION_PIN    9   // Pin 9 - Function/Config button
#define BTN_PLAY_PIN       14   // Pin 14 - Play/Pause transport button
#define BTN_STOP_PIN       15   // Pin 15 - Stop transport button

// LEDs
#define LED_BEAT_PIN       16   // Pin 16 - Beat indicator LED (red)
// Note: Power LED on +5V rail (always on, not MCU-controlled)

// I2C Display (OLED SSD1306)
#define OLED_SDA_PIN        SDA   // Pin 2 (SDA) - I2C data
#define OLED_SCL_PIN        SCL   // Pin 3 (SCL) - I2C clock

// Reserved for future DIN MIDI OUT
#define MIDI_OUT_PIN        TX1   // Pin 1 (TX1) - Reserved, not used in v1

// ============================================================================
// MIDI CONFIGURATION
// ============================================================================

#define MIDI_BAUD_RATE     31250  // Standard MIDI baud rate
#define MIDI_CHANNEL       1      // Default MIDI channel (1-16)

// MIDI Clock Source Selection
enum ClockSource {
    CLOCK_AUTO = 0,      // Auto-select: USB has priority, fallback to DIN
    CLOCK_FORCE_USB = 1, // Force USB MIDI clock only
    CLOCK_FORCE_DIN = 2  // Force DIN MIDI clock only
};

#define DEFAULT_CLOCK_SOURCE  CLOCK_AUTO  // Default clock source mode

// Clock Activity Timeout (milliseconds)
#define CLOCK_TIMEOUT_MS   3000   // If no clock for 3 seconds, consider source inactive

// MIDI CC Assignments
#define CC_VOLUME          7      // Volume pot → CC7 (Main Volume)
#define CC_CUTOFF          74     // Cutoff pot → CC74 (Brightness)
#define CC_RESONANCE       71     // Resonance pot → CC71 (Resonance)

// ============================================================================
// CLOCK SYNC CONFIGURATION
// ============================================================================

#define DEFAULT_PPQN       2      // Default clock pulses per quarter note (Korg/Arturia style)
#define MIN_PPQN           1      // Minimum PPQN
#define MAX_PPQN           24     // Maximum PPQN (full MIDI clock resolution)

#define CLOCK_PULSE_WIDTH  5      // Clock pulse width in milliseconds
#define MIDI_CLOCKS_PER_QN 24     // MIDI clock ticks per quarter note (standard)

// ============================================================================
// CONTROL INPUT CONFIGURATION
// ============================================================================

// ADC Settings
#define ADC_RESOLUTION     10     // 10-bit ADC (0-1023)
#define ADC_MAX_VALUE      1023   // Maximum ADC reading
#define MIDI_MAX_VALUE     127    // Maximum MIDI value (7-bit)
#define ADC_DEADZONE       3      // Noise threshold for pot changes

// Debounce Settings
#define BUTTON_DEBOUNCE_MS 20     // Button debounce time (milliseconds)
#define ENCODER_DEBOUNCE_MS 5     // Encoder debounce time (milliseconds)

// Polling Intervals
#define POT_POLL_INTERVAL  50     // Pot reading interval (milliseconds)
#define BUTTON_POLL_INTERVAL 10   // Button polling interval (milliseconds)

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================

#define SCREEN_WIDTH       128    // OLED width in pixels
#define SCREEN_HEIGHT      64     // OLED height in pixels
#define OLED_RESET         -1     // Reset pin not used (shared with Pro Micro reset)
#define OLED_I2C_ADDRESS   0x3C   // Default I2C address for SSD1306

#define DISPLAY_UPDATE_MS  50     // Display refresh interval (20 Hz)
#define SPLASH_DURATION_MS 2000   // Splash screen duration

// ============================================================================
// EEPROM CONFIGURATION
// ============================================================================

#define EEPROM_MAGIC       0xBEA7 // Magic number for settings validation
#define EEPROM_START_ADDR  0      // EEPROM starting address
#define EEPROM_VERSION     1      // Settings structure version

// ============================================================================
// TIMING CONFIGURATION
// ============================================================================

#define BEAT_LED_DURATION  50     // Beat LED on-time (milliseconds)
#define MENU_TIMEOUT_MS    5000   // Auto-exit menu after inactivity (milliseconds)

// ============================================================================
// SYSTEM CONFIGURATION
// ============================================================================

#define SERIAL_DEBUG       false  // Enable/disable debug output on Serial
#define DEBUG_BAUD_RATE    115200 // Debug serial baud rate

#if SERIAL_DEBUG
  #define DEBUG_PRINT(x)   Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif // CONFIG_H
