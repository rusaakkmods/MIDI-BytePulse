/**
 * MIDI BytePulse - Display Module
 * Handles OLED display and UI
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

// Forward declarations
class MidiHandler;
class ClockSync;
class Controls;

enum DisplayMode {
    MODE_SPLASH,
    MODE_MAIN,
    MODE_MENU
};

enum MenuItem {
    MENU_PPQN,
    MENU_CLOCK_SOURCE,
    MENU_MIDI_CHANNEL,
    MENU_EXIT,
    MENU_COUNT  // Total number of menu items
};

class Display {
public:
    Display();
    
    void begin();
    void update();
    
    // Set module references
    void setMidiHandler(MidiHandler* midiHandler) { _midiHandler = midiHandler; }
    void setClockSync(ClockSync* clockSync) { _clockSync = clockSync; }
    void setControls(Controls* controls) { _controls = controls; }
    
    // Display modes
    void showSplash();
    void showMain();
    void enterMenu();
    void exitMenu();
    bool isInMenu() const { return _mode == MODE_MENU; }
    
    // Menu navigation
    void handleEncoderDelta(int8_t delta);
    void handleEncoderPress();
    
private:
    Adafruit_SSD1306 _display;
    
    // Module references
    MidiHandler* _midiHandler;
    ClockSync* _clockSync;
    Controls* _controls;
    
    // Display state
    DisplayMode _mode;
    unsigned long _lastUpdate;
    unsigned long _splashStartTime;
    unsigned long _lastActivity;
    
    // Menu state
    MenuItem _selectedItem;
    bool _editingValue;
    uint8_t _editPPQN;
    uint8_t _editChannel;
    ClockSource _editClockSource;
    
    // Rendering methods
    void renderSplash();
    void renderMain();
    void renderMenu();
    
    void drawTransportStatus(int16_t x, int16_t y);
    void drawClockInfo(int16_t x, int16_t y);
    void drawControlValues(int16_t x, int16_t y);
    void drawCableStatus(int16_t x, int16_t y);
    
    void drawMenuItem(int16_t y, const char* label, const char* value, bool selected, bool editing);
    
    // Helper methods
    void centerText(const char* text, int16_t y, uint8_t textSize = 1);
    void resetMenuTimeout();
};

#endif // DISPLAY_H
