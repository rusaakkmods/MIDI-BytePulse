/**
 * MIDI BytePulse - Settings Module
 * Handles EEPROM persistence for configuration
 */

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"

struct SettingsData {
    uint16_t magic;           // Magic number for validation
    uint8_t version;          // Settings structure version
    
    // Configuration values
    uint8_t ppqn;             // Clock pulses per quarter note
    uint8_t midiChannel;      // MIDI channel (1-16)
    uint8_t clockSource;      // Clock source: AUTO, FORCE_USB, FORCE_DIN
    
    // Checksums/validation
    uint8_t checksum;         // Simple checksum for data validation
};

class Settings {
public:
    Settings();
    
    void begin();
    
    // Load/Save
    bool load();
    void save();
    void reset();  // Reset to defaults
    
    // Getters
    uint8_t getPPQN() const { return _data.ppqn; }
    uint8_t getMidiChannel() const { return _data.midiChannel; }
    ClockSource getClockSource() const { return (ClockSource)_data.clockSource; }
    
    // Setters
    void setPPQN(uint8_t ppqn);
    void setMidiChannel(uint8_t channel);
    void setClockSource(ClockSource source);
    
    // Validation
    bool isValid() const;
    
private:
    SettingsData _data;
    
    void setDefaults();
    uint8_t calculateChecksum() const;
    void writeToEEPROM();
    void readFromEEPROM();
};

#endif // SETTINGS_H
