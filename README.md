# rMODS MIDI BytePulse

**Universal MIDI Clock Router & Sync Converter** - Multi-source MIDI clock synchronization with intelligent priority switching and bidirectional analog sync rate conversion.

[![Platform](https://img.shields.io/badge/platform-ATmega32U4-blue.svg)](https://www.sparkfun.com/products/12640)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Tests](https://img.shields.io/badge/tests-21%20passed-brightgreen.svg)](test/)

---

## 🎵 Features

### Multi-Source Clock Support
- **USB MIDI Clock** - Computer/DAW sync via native USB MIDI
- **DIN MIDI Clock** - Hardware MIDI IN port (5-pin DIN)
- **Analog Sync Input** - Universal sync input with 5-position PPQN selector (1, 2, 4, 6, 24 PPQN)
- **Intelligent Priority System** - Automatic source switching: SYNC_IN > USB > DIN

### Clock Distribution
- **USB MIDI Clock Output** - Standard 24 PPQN to DAW/software
- **DIN MIDI Clock Output** - Standard 24 PPQN to hardware devices
- **Analog Sync Output** - Variable PPQN (1-48) via rotary switch
- **Display Clock Output** - Dedicated 1 PPQN output for TinyPulse Display module

### Universal Sync Rate Converter
**6-Position Rotary Switch** - Controls both SYNC_IN and SYNC_OUT rates:
- **Position 1:** 1 PPQN (Modular, Arturia BeatStep Pro)
- **Position 2:** 2 PPQN (Korg Volca series)
- **Position 3:** 4 PPQN (Roland DIN Sync)
- **Position 4:** 6 PPQN (Custom devices)
- **Position 5:** 24 PPQN (MIDI passthrough)
- **Position 6:** 48 PPQN (High-resolution sync)

### MIDI Message Handling
- **Bidirectional MIDI Routing:**
  - USB ↔ DIN: All MIDI messages (Clock, Start, Stop, Continue, channel messages)
  - DIN IN → DIN OUT: Clock messages forwarded with priority rules
- **Standard Clock Messages** - Start (0xFA), Stop (0xFC), Continue (0xFB), Clock (0xF8)
- **Zero Latency** - Optimized non-blocking architecture

---

## 🔧 Hardware

### Platform
- **SparkFun Pro Micro** (ATmega32U4, 5V, 16MHz)
- Native USB MIDI support (no FTDI required)
- **Flash:** 40.7% used (11,674 / 28,672 bytes)
- **RAM:** 50.5% used (1,293 / 2,560 bytes)

### Pin Configuration

| Pin | Function | Description |
|-----|----------|-------------|
| 0 | MIDI IN | Hardware Serial RX (5-pin DIN) |
| 1 | MIDI OUT | Hardware Serial TX (5-pin DIN) |
| 4 | DISPLAY CLK | Fixed 1 PPQN for TinyPulse Display |
| 5 | SYNC OUT | Variable PPQN analog clock output |
| 6 | SYNC IN DETECT | Cable detection for sync input |
| 7 | SYNC IN | Analog clock input (interrupt-capable) |
| 8 | LED PULSE | Beat indicator LED (50ms pulse, 1 PPQN) |
| 9 | SYNC RATE 1 | Rotary switch position 1 (1 PPQN) |
| 10 | SYNC RATE 2 | Rotary switch position 2 (2 PPQN) |
| 14 | SYNC RATE 4 | Rotary switch position 4 (6 PPQN) |
| 15 | SYNC RATE 5 | Rotary switch position 5 (24 PPQN) |
| 16 | SYNC RATE 3 | Rotary switch position 3 (4 PPQN) |
| A0 | SYNC RATE 6 | Rotary switch position 6 (48 PPQN) |
| 2, 3 | Reserved | I2C + interrupts (future: encoder) |
| A1-A3 | Reserved | Analog inputs (future: potentiometers) |

### Connections

**MIDI (5-pin DIN):**
- Standard MIDI circuit with 220Ω resistors and 6N138 optocoupler
- IN: Pin 0 (RX1) via optocoupler
- OUT: Pin 1 (TX1) via 220Ω resistor

**Analog Sync (3.5mm mono jacks):**
- SYNC_IN: Pin 7 (interrupt-capable), 5V trigger signal
- SYNC_OUT: Pin 5, variable PPQN (1-48) based on switch, 5ms pulse width
- DISPLAY_CLK: Pin 4, fixed 1 PPQN for TinyPulse Display, 5ms pulse width
- Cable detection via switched jack to pin 6

**Rotary Switch (1P6T):**
- Common terminal to GND
- Position terminals to pins 9, 10, 16, 14, 15, A0
- Internal pullups enabled (no external resistors needed)
- For positions with dual pin connections, use 1N4148 diodes

**TinyPulse Display Module (separate project):**
- Clock: Pin 4 (DISPLAY_CLK) - always 1 PPQN
- MIDI tap: Passive connection to MIDI IN optocoupler output
- Provides real-time BPM display with 4-digit TM1637

**LED Indicator:**
- Pin 8: Beat pulse LED (1 PPQN, 50ms pulse)
- Connect LED + current-limiting resistor to pin 8, cathode to GND

---

## ⚙️ Technical Specifications

### Timing & Synchronization
- **MIDI Clock:** 24 PPQN (standard) on USB and DIN outputs
- **Analog Sync Rates:** 1, 2, 4, 6, 24 PPQN (switch-selectable)
- **BPM Range:** 20-400 BPM supported
- **Clock Accuracy:** Microsecond-precision interrupt handling
- **Latency:** <1ms typical (non-blocking architecture)
- **Pulse Widths:** 5ms (SYNC_OUT, DISPLAY_CLK), 50ms (LED)

### Clock Source Priority
1. **SYNC_IN** - Highest priority (analog/modular gear)
2. **USB MIDI** - Computer/DAW (with 3-second timeout)
3. **DIN MIDI** - Hardware MIDI IN port (lowest priority)

When multiple sources are active, the device automatically switches to the highest priority source with graceful fallback.

### Memory Usage
- **Flash:** 11,674 bytes / 28,672 bytes (40.7%)
- **RAM:** 1,293 bytes / 2,560 bytes (50.5%)
- **Available for expansion:** 59.3% Flash remaining

---

## 📋 Usage

### Basic Operation

**Power On:**
- Device initializes with LED pulse indicator
- Reads rotary switch position for sync rate configuration
- All MIDI routing active immediately

**Clock Playback:**
1. Connect a clock source (USB MIDI from DAW, DIN MIDI, or analog sync)
2. Set rotary switch to match SYNC_IN/OUT device PPQN rate
3. Start playback from your source device
4. LED pulses on every quarter note
5. All connected outputs receive synchronized clock

**Rotary Switch Settings:**
- Set to match your connected analog device's PPQN rate
- Same switch position works for both SYNC_IN and SYNC_OUT
- MIDI outputs always send standard 24 PPQN regardless of switch

**Clock Stopping:**
- Stop playback on source device
- All outputs stop sending clock
- LED stops pulsing

### Connection Scenarios

**Scenario 1: DAW → MIDI BytePulse → Hardware Synths**
```
Computer (USB MIDI) → BytePulse → DIN MIDI OUT → Synths
                             └──→ SYNC_OUT (switch: pos 2) → Volca
```
- All devices receive synchronized clock at 120 BPM
- Volca on SYNC_OUT needs switch position 2 (2 PPQN)
- MIDI devices receive standard 24 PPQN

**Scenario 2: Korg Volca → MIDI BytePulse → DAW**
```
Volca (2 PPQN sync) → SYNC_IN → BytePulse → USB MIDI → DAW
                              └──→ DIN MIDI OUT → Hardware
```
- Set switch to position 2 (2 PPQN for Volca)
- Volca tempo controls everything
- DAW and hardware sync to Volca's clock

**Scenario 3: BeatStep Pro → MIDI BytePulse → Multiple Destinations**
```
BeatStep Pro (1 PPQN) → SYNC_IN → BytePulse → USB + DIN + SYNC_OUT
```
- Set switch to position 1 (1 PPQN for BeatStep)
- Single clock source drives all outputs
- SYNC_OUT can drive additional modular gear

**Scenario 4: Modular → BytePulse → TinyPulse Display**
```
Eurorack Clock → SYNC_IN → BytePulse → DISPLAY_CLK → TinyPulse
```
- TinyPulse always shows correct BPM (uses dedicated 1 PPQN clock)
- Switch setting doesn't affect display accuracy
- SYNC_OUT can simultaneously drive other modular gear

### MIDI Implementation

**Transmitted Messages:**
- `0xF8` - Clock (24 per quarter note)
- `0xFA` - Start
- `0xFC` - Stop
- `0xFB` - Continue
- `0xFE` - Active Sensing (USB only)
- All note/CC/program change messages (passthrough)

**Received Messages:**
- All standard MIDI messages received and processed
- Clock messages trigger sync engine
- Non-clock messages forwarded between USB ↔ DIN

---

## 🧪 Testing

### Unit Tests
Comprehensive automated test suite validates core functionality:

```bash
# Run all unit tests (21 tests)
pio test -e native

# Run specific test suite
pio test -e native -f test_clock_priority
pio test -e native -f test_sync_rate
```

**Test Coverage:**
- **Clock Priority** (7 tests) - SYNC_IN > USB > DIN hierarchy, fallback behavior
- **Sync Rate Conversion** (14 tests) - PPQN multiplication/division, real-world device scenarios

**Total: 21 unit tests, 100% pass rate** - See [test/TESTING_GUIDE.md](test/TESTING_GUIDE.md) for detailed testing documentation.

### Test Results
```
test_clock_priority: 7/7 PASSED
  ✓ Priority: SYNC_IN blocks USB
  ✓ Priority: SYNC_IN blocks DIN
  ✓ Priority: USB blocks DIN
  ✓ Fallback: SYNC_IN to USB
  ✓ Fallback: USB to DIN
  ✓ Priority chain (full hierarchy)
  ✓ Initial state accepts any source

test_sync_rate: 14/14 PASSED
  ✓ SYNC_IN multipliers (1, 2, 4, 6, 24 PPQN)
  ✓ SYNC_OUT divisors (1, 2, 4, 24 PPQN)
  ✓ Volca @ 120 BPM scenario
  ✓ BeatStep Pro @ 120 BPM scenario
  ✓ DAW to Volca SYNC_OUT
  ✓ Bidirectional consistency
```

---

## 🛠️ Building & Flashing

### Prerequisites
- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- SparkFun Pro Micro board definitions
- USB cable (micro-USB)

### Dependencies
All dependencies auto-installed via PlatformIO:
```ini
- MIDI Library v5.0.2 (fortyseveneffects)
- MIDIUSB v1.0.5 (Arduino)
```

### Build & Upload

**Using PlatformIO:**
```bash
# Build project
pio run

# Upload to device
pio run --target upload

# Monitor serial output (debug mode)
pio device monitor
```

**Using PlatformIO IDE (VS Code):**
1. Open project folder in VS Code
2. Click "Build" (✓) in status bar
3. Click "Upload" (→) in status bar

### Debug Mode
Enable serial debugging in `config.h`:
```cpp
#define SERIAL_DEBUG    true
#define DEBUG_BAUD_RATE 115200
```
- Monitor clock source switching and sync rate selection
- Outputs timing information for debugging
- USB timeout warnings

---

## 🎛️ Configuration

### Adjustable Parameters

**`config.h` - Hardware Pins:**
```cpp
#define SYNC_IN_PIN         7    // Analog sync input
#define SYNC_OUT_PIN        5    // Variable PPQN analog sync output
#define DISPLAY_CLK_PIN     4    // Fixed 1 PPQN for TinyPulse
#define LED_PULSE_PIN       8    // Beat indicator LED
// Rotary switch pins: 9, 10, 16, 14, 15, A0
```

**`Sync.cpp` - Timing Constants:**
```cpp
#define CLOCK_PULSE_WIDTH_US 5000      // SYNC_OUT/DISPLAY_CLK pulse width (5ms)
#define LED_PULSE_WIDTH_MS 50          // LED pulse width (50ms)
```

---

## 📖 Code Architecture

### Main Components

**`main.cpp`** - Application entry point
- Setup: Initializes MIDI, Sync engine
- Loop: Processes USB/DIN MIDI, updates sync engine
- Interrupt: Handles SYNC_IN pulses (ISR)

**`Sync.cpp/h`** - Clock synchronization engine
- Multi-source clock management with priority hierarchy
- SYNC_IN PPQN multiplication (1-48 → 24 PPQN MIDI)
- SYNC_OUT PPQN division (24 PPQN MIDI → 1-48)
- Rotary switch reading with debouncing
- Clock distribution to all outputs

**`MIDIHandler.cpp/h`** - MIDI I/O management
- USB ↔ DIN MIDI bidirectional forwarding
- Clock message forwarding with priority rules
- Message parsing and routing

**`config.h`** - Hardware configuration
- Pin definitions
- Debug settings
- Compile-time constants

### Data Flow

```
┌──────────────┐
│  USB MIDI    │ ──┐
│  DIN MIDI    │ ──┼──→ MIDIHandler ──→ Sync Engine ──┬──→ USB MIDI OUT (24 PPQN)
│  SYNC_IN     │ ──┘                                   ├──→ DIN MIDI OUT (24 PPQN)
│ (Interrupt)  │                                       ├──→ SYNC_OUT (1-24 PPQN)
└──────────────┘                                       ├──→ DISPLAY_CLK (1 PPQN)
                                                       └──→ LED_PULSE (1 PPQN)
```

---

## 🔍 Troubleshooting

### Rotary switch not working
- **Check wiring** - Common to GND, positions to correct pins
- **Verify switch type** - Must be 1P5T (1-pole, 5-throw)
- **Add diodes** - Positions 4 and 6 may need 1N4148 diodes for dual connections
- **Serial debug** - Enable debug mode to see switch reading

### Wrong BPM with SYNC_IN
- **Check switch position** - Must match device's PPQN rate (Volca=2, BeatStep=1, etc.)
- **Verify source PPQN** - Some devices use non-standard rates
- **MIDI outputs correct** - USB/DIN always send correct 24 PPQN regardless of switch

### SYNC_OUT tempo incorrect
- **Verify switch position** - Receiving device must match switch PPQN setting
- **Check pulse width** - 5ms pulse compatible with most devices
- **Voltage levels** - Ensure 5V compatible or use level shifter

### MIDI messages not passing through
- **Baud rate** - Hardware MIDI must be 31250 baud (auto-configured)
- **Optocoupler** - Check 6N138 wiring and power supply
- **USB driver** - Update USB MIDI drivers on computer
- **Priority blocking** - Lower priority sources blocked when higher priority active

---

## 📐 Sync Rate Conversion

### Understanding PPQN

**PPQN** = Pulses Per Quarter Note (clock resolution)

- **MIDI Standard:** Always 24 PPQN (USB and DIN outputs)
- **Analog Devices:** Varies (1, 2, 4, 6, 24 PPQN)

The **rotary switch** configures both SYNC_IN and SYNC_OUT to match your device's native rate.

### How It Works

**SYNC_IN → MIDI (Multiplication):**
- Device receives analog pulses at configured PPQN
- Multiplies to 24 PPQN for MIDI outputs
- Example: 2 PPQN input × 12 = 24 PPQN MIDI output

**MIDI → SYNC_OUT (Division):**
- Device receives 24 PPQN MIDI clock internally
- Divides to configured PPQN for SYNC_OUT
- Example: 24 PPQN ÷ 12 = 2 PPQN output (for Volca)

### Common Device PPQN Rates

| Device | PPQN | Switch Position |
|--------|------|-----------------|
| Korg Volca series | 2 | Position 2 |
| Arturia BeatStep Pro | 1 | Position 1 |
| Roland TR-series (DIN Sync) | 4 | Position 3 |
| Eurorack (typical) | 24 | Position 5 |
| Modular (varies) | 1, 4, or 24 | Pos 1, 3, or 5 |

### Switch Setting Examples

**Scenario: Volca as master (2 PPQN) → Switch Position 2**
- SYNC_IN receives 2 pulses per beat
- Multiplies ×12 → 24 PPQN MIDI Clock
- All MIDI devices show correct 120 BPM

**Scenario: DAW → Volca via SYNC_OUT → Switch Position 2**
- MIDI Clock runs at 24 PPQN internally
- SYNC_OUT divides ÷12 → 2 PPQN output
- Volca receives correct 2 PPQN at 120 BPM

---

## 🚀 Future Enhancements

Potential features for future versions:
- [ ] Rotary encoder for tempo adjustment (pins 2, 3 reserved)
- [ ] Potentiometers for swing/groove (A1-A3 reserved)
- [ ] EEPROM settings persistence
- [ ] MIDI message filtering
- [ ] Tap tempo function
- [ ] Clock divider/multiplier modes
- [ ] Additional PPQN rates

**Current Flash Available:** 59.3% (17,000 bytes free)

---

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 🙏 Credits

**Libraries:**
- [MIDI Library](https://github.com/FortySevenEffects/arduino_midi_library) by FortySevenEffects
- [MIDIUSB](https://github.com/arduino-libraries/MIDIUSB) by Arduino

**Hardware:**
- SparkFun Pro Micro board design

**Related Projects:**
- [rMODS TinyPulse Display](../rMODS%20TinyPulse%20Display) - Companion BPM display module

---
