/**
 * MIDI BytePulse - Controls Implementation
 */

#include "Controls.h"
#include "MidiHandler.h"

Controls::Controls()
    : _encoder(ENCODER_A_PIN, ENCODER_B_PIN)
    , _midiHandler(nullptr)
    , _lastEncoderPosition(0)
    , _functionPressed(false)
    , _functionLastState(HIGH)
    , _functionLastDebounceTime(0)
    , _playPressed(false)
    , _playLastState(HIGH)
    , _playLastDebounceTime(0)
    , _stopPressed(false)
    , _stopLastState(HIGH)
    , _stopLastDebounceTime(0)
    , _encoderPressed(false)
    , _encoderLastState(HIGH)
    , _encoderLastDebounceTime(0)
    , _volumeValue(0)
    , _cutoffValue(0)
    , _resonanceValue(0)
    , _volumeRaw(0)
    , _cutoffRaw(0)
    , _resonanceRaw(0)
    , _lastPotReadTime(0) {
}

void Controls::begin() {
    // Configure button pins with internal pullups
    pinMode(BTN_FUNCTION_PIN, INPUT_PULLUP);
    pinMode(BTN_PLAY_PIN, INPUT_PULLUP);
    pinMode(BTN_STOP_PIN, INPUT_PULLUP);
    pinMode(ENCODER_BTN_PIN, INPUT_PULLUP);
    
    // Configure ADC pins (analog inputs)
    pinMode(POT_VOLUME_PIN, INPUT);
    pinMode(POT_CUTOFF_PIN, INPUT);
    pinMode(POT_RESONANCE_PIN, INPUT);
    
    // Initialize pot values
    _volumeRaw = analogRead(POT_VOLUME_PIN);
    _cutoffRaw = analogRead(POT_CUTOFF_PIN);
    _resonanceRaw = analogRead(POT_RESONANCE_PIN);
    
    _volumeValue = mapAdcToMidi(_volumeRaw);
    _cutoffValue = mapAdcToMidi(_cutoffRaw);
    _resonanceValue = mapAdcToMidi(_resonanceRaw);
    
    DEBUG_PRINTLN("Controls initialized");
}

void Controls::update() {
    updateButtons();
    updateEncoder();
    updatePots();
}

void Controls::updateButtons() {
    // Debounce all buttons
    _functionPressed = debounceButton(BTN_FUNCTION_PIN, _functionLastState, _functionLastDebounceTime);
    _playPressed = debounceButton(BTN_PLAY_PIN, _playLastState, _playLastDebounceTime);
    _stopPressed = debounceButton(BTN_STOP_PIN, _stopLastState, _stopLastDebounceTime);
    _encoderPressed = debounceButton(ENCODER_BTN_PIN, _encoderLastState, _encoderLastDebounceTime);
}

void Controls::updateEncoder() {
    // Encoder library handles state internally
    // We just track position changes
    long newPosition = _encoder.read();
    
    // Normalize to steps of 4 (typical encoder resolution)
    newPosition /= 4;
    
    if (newPosition != _lastEncoderPosition) {
        _lastEncoderPosition = newPosition;
    }
}

void Controls::updatePots() {
    unsigned long now = millis();
    
    // Throttle pot reading to POT_POLL_INTERVAL
    if (now - _lastPotReadTime < POT_POLL_INTERVAL) {
        return;
    }
    _lastPotReadTime = now;
    
    // Read all pots
    uint16_t volumeRaw = analogRead(POT_VOLUME_PIN);
    uint16_t cutoffRaw = analogRead(POT_CUTOFF_PIN);
    uint16_t resonanceRaw = analogRead(POT_RESONANCE_PIN);
    
    // Check for changes and send MIDI CC if changed
    if (potValueChanged(volumeRaw, _volumeRaw, _volumeValue)) {
        if (_midiHandler) {
            _midiHandler->sendCC(CC_VOLUME, _volumeValue);
            DEBUG_PRINT("Volume: ");
            DEBUG_PRINTLN(_volumeValue);
        }
    }
    
    if (potValueChanged(cutoffRaw, _cutoffRaw, _cutoffValue)) {
        if (_midiHandler) {
            _midiHandler->sendCC(CC_CUTOFF, _cutoffValue);
            DEBUG_PRINT("Cutoff: ");
            DEBUG_PRINTLN(_cutoffValue);
        }
    }
    
    if (potValueChanged(resonanceRaw, _resonanceRaw, _resonanceValue)) {
        if (_midiHandler) {
            _midiHandler->sendCC(CC_RESONANCE, _resonanceValue);
            DEBUG_PRINT("Resonance: ");
            DEBUG_PRINTLN(_resonanceValue);
        }
    }
}

int8_t Controls::getEncoderDelta() {
    long newPosition = _encoder.read() / 4;
    int8_t delta = (int8_t)(newPosition - _lastEncoderPosition);
    _lastEncoderPosition = newPosition;
    return delta;
}

bool Controls::debounceButton(uint8_t pin, bool& lastState, unsigned long& lastDebounceTime) {
    bool reading = digitalRead(pin);
    unsigned long now = millis();
    
    // If state changed, reset debounce timer
    if (reading != lastState) {
        lastDebounceTime = now;
        lastState = reading;
    }
    
    // Check if enough time has passed for stable reading
    if ((now - lastDebounceTime) > BUTTON_DEBOUNCE_MS) {
        // Button is pressed when pin reads LOW (active low with pullup)
        return (reading == LOW);
    }
    
    return false;
}

uint8_t Controls::mapAdcToMidi(uint16_t adcValue) {
    // Map 10-bit ADC (0-1023) to 7-bit MIDI (0-127)
    return (uint8_t)((adcValue * MIDI_MAX_VALUE) / ADC_MAX_VALUE);
}

bool Controls::potValueChanged(uint16_t newRaw, uint16_t& oldRaw, uint8_t& midiValue) {
    // Check if raw value changed beyond deadzone
    int16_t diff = abs((int16_t)newRaw - (int16_t)oldRaw);
    
    if (diff > ADC_DEADZONE) {
        oldRaw = newRaw;
        uint8_t newMidiValue = mapAdcToMidi(newRaw);
        
        // Only report change if MIDI value actually changed
        if (newMidiValue != midiValue) {
            midiValue = newMidiValue;
            return true;
        }
    }
    
    return false;
}
