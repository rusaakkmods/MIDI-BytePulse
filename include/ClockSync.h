/**
 * MIDI BytePulse - Clock Sync Module
 * Generates analog clock pulses from MIDI clock
 */

#ifndef CLOCK_SYNC_H
#define CLOCK_SYNC_H

#include <Arduino.h>
#include "config.h"

class ClockSync {
public:
    ClockSync();
    
    void begin();
    void update();
    
    // MIDI clock event handlers
    void onMidiClock();
    void onTransportStart();
    void onTransportContinue();
    void onTransportStop();
    void reset();
    
    // Configuration
    void setPPQN(uint8_t ppqn);
    uint8_t getPPQN() const { return _ppqn; }
    
    // Status
    bool isCableInserted() const { return _cableInserted; }
    bool isClockActive() const { return _clockActive; }
    uint32_t getPulseCount() const { return _pulseCount; }
    
    // Beat LED control
    bool shouldBeatLedBeOn() const { return _beatLedActive; }
    
private:
    // Configuration
    uint8_t _ppqn;                    // Pulses per quarter note
    uint8_t _clockDivider;            // MIDI clocks per output pulse
    
    // State
    bool _cableInserted;              // Sync cable plug detection
    bool _clockActive;                // Clock currently running
    uint8_t _midiClockCounter;        // Counter for clock divider
    uint32_t _pulseCount;             // Total pulses generated
    
    // Pulse timing
    unsigned long _pulseStartTime;    // When current pulse started
    bool _pulseHigh;                  // Current pulse state
    
    // Beat LED timing
    unsigned long _beatLedStartTime;  // When beat LED turned on
    bool _beatLedActive;              // Beat LED should be on
    uint8_t _beatClockCounter;        // Counter for beat detection (24 clocks = 1 beat)
    
    // Hardware control
    void setPulseHigh();
    void setPulseLow();
    void updateCableDetection();
    void calculateClockDivider();
};

#endif // CLOCK_SYNC_H
