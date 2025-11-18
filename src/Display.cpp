/**
 * MIDI BytePulse - Display Implementation (U8g2)
 * Lightweight display using U8g2 library
 */

#include "Display.h"
#include "MidiHandler.h"
#include "ClockSync.h"
#include "Controls.h"

Display::Display()
    : _display(U8G2_R0, U8X8_PIN_NONE)
    , _midiHandler(nullptr)
    , _clockSync(nullptr)
    , _controls(nullptr)
    , _mode(MODE_SPLASH)
    , _lastUpdate(0)
    , _splashStartTime(0)
    , _lastActivity(0)
    , _selectedItem(MENU_PPQN)
    , _editingValue(false)
    , _editPPQN(DEFAULT_PPQN)
    , _editChannel(MIDI_CHANNEL)
    , _editClockSource(DEFAULT_CLOCK_SOURCE)
    , _lastVolumeValue(255)
    , _lastCutoffValue(255)
    , _lastResonanceValue(255)
    , _lastPanValue(64)
    , _wasStopped(true)
    , _encoderButtonPressed(false)
    , _needsUpdate(true)
    , _lastPlayingState(false) {
    strcpy(_lastControlLabel, "---");
}

void Display::begin() {
    if (!_display.begin()) {
        DEBUG_PRINTLN("OLED fail");
        return;
    }
    
    showSplash();
    DEBUG_PRINTLN("Disp OK");
}

void Display::update() {
    unsigned long now = millis();
    
    // Always check for value changes, but respect update interval for actual rendering
    bool shouldRender = (now - _lastUpdate >= DISPLAY_UPDATE_MS) || _needsUpdate;
    
    if (!shouldRender) {
        return;
    }
    _lastUpdate = now;
    
    switch (_mode) {
        case MODE_SPLASH:
            if (now - _splashStartTime >= SPLASH_DURATION_MS) {
                showMain();
            }
            break;
            
        case MODE_MAIN:
            renderMain();
            _needsUpdate = false;  // Reset after render
            break;
            
        case MODE_MENU:
            if (now - _lastActivity >= MENU_TIMEOUT_MS) {
                exitMenu();
            } else {
                renderMenu();
                _needsUpdate = false;  // Reset after render
            }
            break;
    }
}

void Display::showSplash() {
    _mode = MODE_SPLASH;
    _splashStartTime = millis();
    renderSplash();
}

void Display::showMain() {
    _mode = MODE_MAIN;
    _display.clearBuffer();  // Clear the splash screen
    _needsUpdate = true;      // Force first render
    renderMain();
}

void Display::enterMenu() {
    _mode = MODE_MENU;
    _selectedItem = MENU_PPQN;
    _editingValue = false;
    
    if (_clockSync) {
        _editPPQN = _clockSync->getPPQN();
    }
    if (_midiHandler) {
        _editClockSource = _midiHandler->getClockSource();
    }
    _editChannel = MIDI_CHANNEL;
    
    _display.clearBuffer();  // Clear the main screen
    _needsUpdate = true;      // Force render of menu
    
    resetMenuTimeout();
    DEBUG_PRINTLN("Menu");
}

void Display::exitMenu() {
    _mode = MODE_MAIN;
    _editingValue = false;
    _display.clearBuffer();  // Clear the menu
    _needsUpdate = true;      // Force render of main screen
    DEBUG_PRINTLN("Exit");
}

void Display::handleEncoderDelta(int8_t delta) {
    if (_mode != MODE_MENU) return;
    
    resetMenuTimeout();
    
    if (_editingValue) {
        switch (_selectedItem) {
            case MENU_PPQN:
                _editPPQN = constrain(_editPPQN + delta, MIN_PPQN, MAX_PPQN);
                break;
                
            case MENU_CLOCK_SOURCE:
                {
                    int newSource = (int)_editClockSource + delta;
                    if (newSource < 0) newSource = 2;
                    if (newSource > 2) newSource = 0;
                    _editClockSource = (ClockSource)newSource;
                }
                break;
                
            case MENU_MIDI_CHANNEL:
                _editChannel = constrain(_editChannel + delta, 1, 16);
                break;
                
            default:
                break;
        }
    } else {
        int newItem = (int)_selectedItem + delta;
        if (newItem < 0) newItem = MENU_COUNT - 1;
        if (newItem >= MENU_COUNT) newItem = 0;
        _selectedItem = (MenuItem)newItem;
    }
}

void Display::handleEncoderPress() {
    if (_mode != MODE_MENU) return;
    
    resetMenuTimeout();
    
    if (_editingValue) {
        switch (_selectedItem) {
            case MENU_PPQN:
                if (_clockSync) {
                    _clockSync->setPPQN(_editPPQN);
                }
                break;
                
            case MENU_CLOCK_SOURCE:
                if (_midiHandler) {
                    _midiHandler->setClockSource(_editClockSource);
                }
                break;
                
            case MENU_MIDI_CHANNEL:
                break;
                
            default:
                break;
        }
        _editingValue = false;
    } else {
        switch (_selectedItem) {
            case MENU_PPQN:
            case MENU_CLOCK_SOURCE:
            case MENU_MIDI_CHANNEL:
                _editingValue = true;
                break;
                
            case MENU_EXIT:
                exitMenu();
                break;
                
            default:
                break;
        }
    }
}

void Display::renderSplash() {
    _display.clearBuffer();
    _display.setFont(u8g2_font_6x10_tr);
    _display.drawStr(30, 20, "BytePulse");
    _display.sendBuffer();
}

void Display::renderMain() {
    char buf[32];
    
    // Don't clear entire buffer - we'll only update changed regions
    _display.setFont(u8g2_font_6x10_tr);
    
    #if TEST_MODE
    // Test mode: Display volume, cutoff, and resonance values (use small font)
    
    if (_controls) {
        // Get current MIDI values
        uint8_t volumeVal = _controls->getVolumeValue();
        uint8_t cutoffVal = _controls->getCutoffValue();
        uint8_t resonanceVal = _controls->getResonanceValue();
        
        // Check which control changed
        if (volumeVal != _lastVolumeValue) {
            _lastVolumeValue = volumeVal;
            strcpy(_lastControlLabel, "VOL");
        }
        if (cutoffVal != _lastCutoffValue) {
            _lastCutoffValue = cutoffVal;
            strcpy(_lastControlLabel, "CUT");
        }
        if (resonanceVal != _lastResonanceValue) {
            _lastResonanceValue = resonanceVal;
            strcpy(_lastControlLabel, "RES");
        }
        // Pan is updated directly from testModeLoop
        
        // Clear and redraw control value area only (top line)
        _display.setDrawColor(0);  // Black
        _display.drawBox(0, 0, 80, 16);  // Clear control value area
        _display.setDrawColor(1);  // White
        
        // Display the last changed control
        if (strcmp(_lastControlLabel, "VOL") == 0) {
            snprintf(buf, sizeof(buf), "VOL:%d", _lastVolumeValue);
        } else if (strcmp(_lastControlLabel, "CUT") == 0) {
            snprintf(buf, sizeof(buf), "CUT:%d", _lastCutoffValue);
        } else if (strcmp(_lastControlLabel, "RES") == 0) {
            snprintf(buf, sizeof(buf), "RES:%d", _lastResonanceValue);
        } else if (strcmp(_lastControlLabel, "PAN") == 0) {
            snprintf(buf, sizeof(buf), "PAN:%d", _lastPanValue);
        } else {
            snprintf(buf, sizeof(buf), "---");
        }
        _display.drawStr(0, 15, buf);
        
        // Clear and redraw encoder button status area only (top right)
        _display.setDrawColor(0);  // Black
        _display.drawBox(80, 0, 48, 16);  // Clear button status area
        _display.setDrawColor(1);  // White
        
        if (_encoderButtonPressed) {
            _display.drawStr(80, 15, "PRESSED");
        }
        
        // Check for play state changes
        bool currentPlayingState = (_midiHandler && _midiHandler->isPlaying());
        if (currentPlayingState != _lastPlayingState) {
            _lastPlayingState = currentPlayingState;
        }
        
        _display.setDrawColor(0);  // Black
        _display.drawBox(0, 16, 60, 16);  // Clear transport area
        _display.setDrawColor(1);  // White
        
        if (currentPlayingState) {
            _display.drawStr(0, 30, "PLAY");
        } else if (_wasStopped) {
            _display.drawStr(0, 30, "STOP");
        } else {
            _display.drawStr(0, 30, "PAUSE");
        }
    } else {
        _display.drawStr(0, 20, "---");
    }
    #else
        // Line 1: Controls (VOL CUT RES PPQN) in 2-digit hex
        if (_controls && _clockSync) {
            snprintf(buf, sizeof(buf), "V%02X C%02X R%02X Q%02X",
                     _controls->getVolumeValue(),
                     _controls->getCutoffValue(),
                     _controls->getResonanceValue(),
                     _clockSync->getPPQN());
            _display.drawStr(0, 10, buf);
        }
        
        // Cable status
        if (_clockSync && _clockSync->isCableInserted()) {
            _display.drawDisc(120, 7, 3);
        } else {
            _display.drawCircle(120, 7, 3);
        }
    #endif
    
    // Send only the changed regions to display (framebuffer mode)
    _display.sendBuffer();
}

void Display::renderMenu() {
    char buf[16];
    
    _display.clearBuffer();  // Clear for menu
    _display.setFont(u8g2_font_6x10_tr);
    int y = 12;
    
    // PPQN
    snprintf(buf, sizeof(buf), "%d", _editPPQN);
    if (_selectedItem == MENU_PPQN) _display.drawStr(0, y, ">");
    _display.drawStr(10, y, "PPQN");
    _display.drawStr(70, y, buf);
    if (_selectedItem == MENU_PPQN && _editingValue) {
        _display.drawFrame(68, y-9, 30, 12);
    }
    y += 14;
    
    // Clock Source
    const char* cs = "AUTO";
    if (_editClockSource == CLOCK_FORCE_USB) cs = "USB";
    else if (_editClockSource == CLOCK_FORCE_DIN) cs = "DIN";
    if (_selectedItem == MENU_CLOCK_SOURCE) _display.drawStr(0, y, ">");
    _display.drawStr(10, y, "Clk");
    _display.drawStr(70, y, cs);
    if (_selectedItem == MENU_CLOCK_SOURCE && _editingValue) {
        _display.drawFrame(68, y-9, 30, 12);
    }
    y += 14;
    
    // MIDI Channel
    snprintf(buf, sizeof(buf), "%d", _editChannel);
    if (_selectedItem == MENU_MIDI_CHANNEL) _display.drawStr(0, y, ">");
    _display.drawStr(10, y, "Ch");
    _display.drawStr(70, y, buf);
    if (_selectedItem == MENU_MIDI_CHANNEL && _editingValue) {
        _display.drawFrame(68, y-9, 30, 12);
    }
    y += 18;
    
    // Exit
    if (_selectedItem == MENU_EXIT) _display.drawStr(0, y, ">");
    _display.drawStr(10, y, "Exit");
    
    _display.sendBuffer();
}

void Display::resetMenuTimeout() {
    _lastActivity = millis();
}
