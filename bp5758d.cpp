#include <Arduino.h>
#include <math.h>
#include "components/bp5758d.h"

// ---- BP5758D register addresses (auto-incrementing write) ----
static const uint8_t ADDR_ENABLE = 0x90;   // OUTPUT_1_TO_5_ENABLEMENT (start of frame)

// Per-channel current ranges — preserved from stock. Colours 0x10, whites 0x1A.
static const uint8_t CURRENT[5] = { 0x10, 0x10, 0x10, 0x1A, 0x1A };

// ---- gamma table, built once ----
static uint8_t  gammaTable[256];
static bool     gammaReady = false;
static void buildGamma(float g = 2.2f) {
    for (int i = 0; i < 256; i++)
        gammaTable[i] = (uint8_t)lround(255.0 * pow(i / 255.0, g));
    gammaReady = true;
}
static inline uint8_t gamma8(uint8_t v) { return gammaReady ? gammaTable[v] : v; }

BP5758D::BP5758D(uint8_t sda_pin, uint8_t scl_pin) : _sda(sda_pin), _scl(scl_pin) {}

void BP5758D::tick() { delayMicroseconds(2); }

void BP5758D::startCond() {                 // SDA falling while SCL high = START
    digitalWrite(_sda, LOW);  tick();
    digitalWrite(_scl, LOW);  tick();
}
void BP5758D::stopCond() {                  // SDA rising while SCL high = STOP (idle)
    digitalWrite(_scl, HIGH); tick();
    digitalWrite(_sda, HIGH); tick();
}
void BP5758D::writeByte(uint8_t value) {    // MSB first
    for (int i = 7; i >= 0; i--) {
        digitalWrite(_sda, (value >> i) & 1); tick();
        digitalWrite(_scl, HIGH);             tick();
        digitalWrite(_scl, LOW);              tick();
    }
}
void BP5758D::ackClock() {                  // release SDA, one clock (ack not verified)
    pinMode(_sda, INPUT);
    digitalWrite(_scl, HIGH); tick();
    digitalWrite(_scl, LOW);  tick();
    pinMode(_sda, OUTPUT);
}

void BP5758D::sendFrame(const uint8_t out[5], bool gammaCorrect) {
    uint8_t data[17];
    data[0] = ADDR_ENABLE;
    data[1] = 0x1F;                         // enable OUT1..OUT5
    for (int i = 0; i < 5; i++) data[2 + i] = CURRENT[i];
    for (int i = 0; i < 5; i++) {
        uint8_t v = gammaCorrect ? gamma8(out[i]) : out[i];
        uint16_t word = (uint16_t)v * 4;    // 0-255 -> 0-1020 (10-bit grayscale)
        if (word == 0 && v > 0) word = 1;   // keep 1% visible
        data[7 + i * 2] = word & 0x1F;      // low 5 bits
        data[8 + i * 2] = word >> 5;        // high 5 bits
    }
    startCond();
    for (int i = 0; i < 17; i++) { writeByte(data[i]); ackClock(); }
    stopCond();
}

void BP5758D::begin() {
    if (!gammaReady) buildGamma();
    pinMode(_sda, OUTPUT);
    pinMode(_scl, OUTPUT);
    digitalWrite(_scl, HIGH);               // idle
    digitalWrite(_sda, HIGH);
    tick();
    off();
}

// colour order in -> OUT order: OUT1=B OUT2=G OUT3=R OUT4=W OUT5=C
void BP5758D::set(uint8_t r, uint8_t g, uint8_t b, uint8_t warm, uint8_t cold) {
    _r = r; _g = g; _b = b; _w = warm; _c = cold;
    uint8_t out[5] = { b, g, r, warm, cold };
    sendFrame(out, true);
}
void BP5758D::setRaw(uint8_t r, uint8_t g, uint8_t b, uint8_t warm, uint8_t cold) {
    _r = r; _g = g; _b = b; _w = warm; _c = cold;
    uint8_t out[5] = { b, g, r, warm, cold };
    sendFrame(out, false);
}
void BP5758D::off() {
    _r = _g = _b = _w = _c = 0;
    uint8_t z[5] = { 0, 0, 0, 0, 0 };
    sendFrame(z, false);
}

void BP5758D::fadeTo(uint8_t r, uint8_t g, uint8_t b, uint8_t warm, uint8_t cold, uint16_t ms) {
    const int steps = 32;
    int sr = _r, sg = _g, sb = _b, sw = _w, sc = _c;
    for (int s = 1; s <= steps; s++) {
        float t = (float)s / steps;
        set((uint8_t)(sr + (r - sr) * t), (uint8_t)(sg + (g - sg) * t),
            (uint8_t)(sb + (b - sb) * t), (uint8_t)(sw + (warm - sw) * t),
            (uint8_t)(sc + (cold - sc) * t));
        delay(ms / steps);
    }
    set(r, g, b, warm, cold);
}

// ---- the one instance everything shares ----
BP5758D bulb(19, 18);   // SDA=GPIO19, SCL=GPIO18
