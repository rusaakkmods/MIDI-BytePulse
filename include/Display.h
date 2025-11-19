/**
 * MIDI BytePulse - TM1637 7-Segment Display
 * Lightweight BPM display using TM1637 4-digit module
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <TM1637Display.h>
#include "config.h"

class Display {
public:
    Display();
    
    void begin();
    void update();
    void showBPM(uint16_t bpm);
    void showPPQN(uint8_t ppqn);
    void clear();
    void setBrightness(uint8_t brightness); // 0-7
    
private:
    TM1637Display _display;
    uint16_t _lastBPM;
    bool _needsUpdate;
};

#endif // DISPLAY_H
