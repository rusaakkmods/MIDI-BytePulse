/**
 * MIDI BytePulse - Controls Module
 * Handles pots, encoder, and buttons with debouncing
 */

#ifndef CONTROLS_H
#define CONTROLS_H

#include <Arduino.h>
#include <Encoder.h>
#include "config.h"

// Forward declaration
class MidiHandler;

class Controls {
public:
    Controls();
    
    void begin();
    void update();
    
    // Set MIDI handler for sending CC messages
    void setMidiHandler(MidiHandler* midiHandler) { _midiHandler = midiHandler; }
    
    // Button states
    bool functionPressed() const { return _functionPressed; }
    bool playPressed() const { return _playPressed; }
    bool stopPressed() const { return _stopPressed; }
    bool encoderPressed() const { return _encoderPressed; }
    
    // Encoder value (-1, 0, +1 for each update)
    int8_t getEncoderDelta();
    
    // Pot values (0-127 MIDI range)
    uint8_t getVolumeValue() const { return _volumeValue; }
    uint8_t getCutoffValue() const { return _cutoffValue; }
    uint8_t getResonanceValue() const { return _resonanceValue; }
    
private:
    // MIDI handler reference
    MidiHandler* _midiHandler;
    
    // Encoder - using standard library
    Encoder _encoder;
    long _lastEncoderPosition;
    
    // Button states
    bool _functionPressed;
    bool _functionLastState;
    unsigned long _functionLastDebounceTime;
    
    bool _playPressed;
    bool _playLastState;
    unsigned long _playLastDebounceTime;
    
    bool _stopPressed;
    bool _stopLastState;
    unsigned long _stopLastDebounceTime;
    
    bool _encoderPressed;
    bool _encoderLastState;
    unsigned long _encoderLastDebounceTime;
    
    // Pot values
    uint8_t _volumeValue;
    uint8_t _cutoffValue;
    uint8_t _resonanceValue;
    
    uint16_t _volumeRaw;
    uint16_t _cutoffRaw;
    uint16_t _resonanceRaw;
    
    // EMA smoothing (accumulated smoothed values)
    float _volumeSmooth;
    float _cutoffSmooth;
    float _resonanceSmooth;
    
    unsigned long _lastPotReadTime;
    
    // Methods
    void updateButtons();
    void updateEncoder();
    void updatePots();
    
    bool debounceButton(uint8_t pin, bool& lastState, unsigned long& lastDebounceTime);
    uint8_t mapAdcToMidi(uint16_t adcValue);
    bool potValueChanged(uint16_t newRaw, uint16_t& oldRaw, uint8_t& midiValue, float& smoothValue);
    uint16_t oversampleAnalogRead(uint8_t pin);
};

#endif // CONTROLS_H
