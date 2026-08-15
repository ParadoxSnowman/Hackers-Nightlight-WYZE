#pragma once
#include <stdint.h>

// BP5758D 5-channel constant-current LED driver (Wyze WLPA19CV2).
// Bit-banged two-wire protocol on SDA=GPIO19, SCL=GPIO18.
//
// Output -> colour map and per-channel current ranges are preserved
// exactly from the known-good init sequence in the stock community fw:
//   OUT1=Blue OUT2=Green OUT3=Red  (current 0x10)
//   OUT4=Warm OUT5=Cold            (current 0x1A)
// Do NOT change the current-range bytes without measuring LED current.

class BP5758D {
public:
    BP5758D(uint8_t sda_pin, uint8_t scl_pin);

    void begin();                                              // call once, post-Arduino-init
    void set(uint8_t r, uint8_t g, uint8_t b,
             uint8_t warm, uint8_t cold);                      // 0-255, gamma-corrected
    void setRaw(uint8_t r, uint8_t g, uint8_t b,
                uint8_t warm, uint8_t cold);                   // 0-255, linear (calibration)
    void fadeTo(uint8_t r, uint8_t g, uint8_t b,
                uint8_t warm, uint8_t cold, uint16_t ms = 300);
    void off();

private:
    uint8_t _sda, _scl;
    uint8_t _r = 0, _g = 0, _b = 0, _w = 0, _c = 0;            // last targets, for fading

    void sendFrame(const uint8_t out[5], bool gammaCorrect);   // out[i] = OUT(i+1) grayscale 0-255
    void tick();
    void startCond();
    void stopCond();
    void writeByte(uint8_t value);
    void ackClock();
};

extern BP5758D bulb;   // single global instance (defined in bp5758d.cpp)
