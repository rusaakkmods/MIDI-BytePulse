/**
 * MIDI BytePulse - Display Implementation
 */

#include "Display.h"
#include "MidiHandler.h"
#include "ClockSync.h"
#include "Controls.h"

Display::Display()
    : _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)
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
    , _editClockSource(DEFAULT_CLOCK_SOURCE) {
}

void Display::begin() {
    // Initialize I2C display
    if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
        DEBUG_PRINTLN("SSD1306 allocation failed");
        return;
    }
    
    _display.clearDisplay();
    _display.setTextColor(SSD1306_WHITE);
    
    showSplash();
    
    DEBUG_PRINTLN("Display initialized");
}

void Display::update() {
    unsigned long now = millis();
    
    // Check if it's time to update display
    if (now - _lastUpdate < DISPLAY_UPDATE_MS) {
        return;
    }
    _lastUpdate = now;
    
    // Handle mode transitions
    switch (_mode) {
        case MODE_SPLASH:
            if (now - _splashStartTime >= SPLASH_DURATION_MS) {
                showMain();
            }
            break;
            
        case MODE_MAIN:
            renderMain();
            break;
            
        case MODE_MENU:
            // Auto-exit menu after timeout
            if (now - _lastActivity >= MENU_TIMEOUT_MS) {
                exitMenu();
            } else {
                renderMenu();
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
    renderMain();
}

void Display::enterMenu() {
    _mode = MODE_MENU;
    _selectedItem = MENU_PPQN;
    _editingValue = false;
    
    // Load current values
    if (_clockSync) {
        _editPPQN = _clockSync->getPPQN();
    }
    if (_midiHandler) {
        _editClockSource = _midiHandler->getClockSource();
    }
    _editChannel = MIDI_CHANNEL;
    
    resetMenuTimeout();
    DEBUG_PRINTLN("Entered menu");
}

void Display::exitMenu() {
    _mode = MODE_MAIN;
    _editingValue = false;
    DEBUG_PRINTLN("Exited menu");
}

void Display::handleEncoderDelta(int8_t delta) {
    if (_mode != MODE_MENU) return;
    
    resetMenuTimeout();
    
    if (_editingValue) {
        // Adjust value
        switch (_selectedItem) {
            case MENU_PPQN:
                _editPPQN = constrain(_editPPQN + delta, MIN_PPQN, MAX_PPQN);
                break;
                
            case MENU_CLOCK_SOURCE:
                {
                    int newSource = (int)_editClockSource + delta;
                    if (newSource < 0) newSource = 2;  // Wrap to CLOCK_FORCE_DIN
                    if (newSource > 2) newSource = 0;  // Wrap to CLOCK_AUTO
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
        // Navigate menu
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
        // Confirm value change
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
                // Store MIDI channel (would be saved to EEPROM)
                break;
                
            default:
                break;
        }
        _editingValue = false;
        
    } else {
        // Enter edit mode or execute action
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

// ============================================================================
// Rendering Methods
// ============================================================================

void Display::renderSplash() {
    _display.clearDisplay();
    
    centerText("MIDI BytePulse", 20, 2);
    centerText("v1.0", 40, 1);
    
    _display.display();
}

void Display::renderMain() {
    _display.clearDisplay();
    
    // Transport status and BPM (top)
    drawTransportStatus(0, 0);
    
    // Clock info (middle)
    drawClockInfo(0, 30);
    
    // Control values (bottom)
    drawControlValues(0, 45);
    
    // Cable status indicator
    drawCableStatus(110, 0);
    
    _display.display();
}

void Display::renderMenu() {
    _display.clearDisplay();
    
    _display.setTextSize(1);
    _display.setCursor(0, 0);
    _display.print("-- MENU --");
    
    // PPQN setting
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", _editPPQN);
    drawMenuItem(12, "PPQN", buf, _selectedItem == MENU_PPQN, _editingValue);
    
    // Clock Source setting
    const char* clockSourceStr = "AUTO";
    if (_editClockSource == CLOCK_FORCE_USB) clockSourceStr = "USB";
    else if (_editClockSource == CLOCK_FORCE_DIN) clockSourceStr = "DIN";
    drawMenuItem(24, "Clock", clockSourceStr, _selectedItem == MENU_CLOCK_SOURCE, _editingValue);
    
    // MIDI Channel setting
    snprintf(buf, sizeof(buf), "%d", _editChannel);
    drawMenuItem(36, "MIDI Ch", buf, _selectedItem == MENU_MIDI_CHANNEL, _editingValue);
    
    // Exit option
    drawMenuItem(48, "Exit", "", _selectedItem == MENU_EXIT, false);
    
    _display.display();
}

void Display::drawTransportStatus(int16_t x, int16_t y) {
    _display.setTextSize(1);
    _display.setCursor(x, y);
    
    if (_midiHandler && _midiHandler->isPlaying()) {
        _display.print("PLAY ");
        _display.print(_midiHandler->getBPM());
        _display.print(" BPM");
        
        // Show active clock source
        ClockSource active = _midiHandler->getActiveClockSource();
        _display.setCursor(x, y + 10);
        if (active == CLOCK_FORCE_USB) {
            _display.print("USB");
        } else if (active == CLOCK_FORCE_DIN) {
            _display.print("DIN");
        } else {
            _display.print("---");
        }
    } else {
        _display.print("STOP");
    }
}

void Display::drawClockInfo(int16_t x, int16_t y) {
    _display.setTextSize(1);
    _display.setCursor(x, y);
    _display.print("PPQN:");
    if (_clockSync) {
        _display.print(_clockSync->getPPQN());
        _display.print("  Pulses:");
        _display.print(_clockSync->getPulseCount());
    }
}

void Display::drawControlValues(int16_t x, int16_t y) {
    if (!_controls) return;
    
    _display.setTextSize(1);
    
    _display.setCursor(x, y);
    _display.print("Vol:");
    _display.print(_controls->getVolumeValue());
    
    _display.setCursor(x + 50, y);
    _display.print("Cut:");
    _display.print(_controls->getCutoffValue());
    
    _display.setCursor(x, y + 10);
    _display.print("Res:");
    _display.print(_controls->getResonanceValue());
}

void Display::drawCableStatus(int16_t x, int16_t y) {
    if (_clockSync && _clockSync->isCableInserted()) {
        _display.fillCircle(x, y + 4, 3, SSD1306_WHITE);
    } else {
        _display.drawCircle(x, y + 4, 3, SSD1306_WHITE);
    }
}

void Display::drawMenuItem(int16_t y, const char* label, const char* value, bool selected, bool editing) {
    _display.setTextSize(1);
    
    // Draw selection indicator
    if (selected) {
        _display.setCursor(0, y);
        _display.print(">");
    }
    
    // Draw label
    _display.setCursor(10, y);
    _display.print(label);
    
    // Draw value (if provided)
    if (value[0] != '\0') {
        _display.setCursor(70, y);
        
        // Highlight if editing
        if (editing) {
            _display.fillRect(68, y - 1, 30, 10, SSD1306_WHITE);
            _display.setTextColor(SSD1306_BLACK);
        }
        
        _display.print(value);
        
        if (editing) {
            _display.setTextColor(SSD1306_WHITE);
        }
    }
}

void Display::centerText(const char* text, int16_t y, uint8_t textSize) {
    _display.setTextSize(textSize);
    
    int16_t x1, y1;
    uint16_t w, h;
    _display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    
    int16_t x = (SCREEN_WIDTH - w) / 2;
    _display.setCursor(x, y);
    _display.print(text);
}

void Display::resetMenuTimeout() {
    _lastActivity = millis();
}
