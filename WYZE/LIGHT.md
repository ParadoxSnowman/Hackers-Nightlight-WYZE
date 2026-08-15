cat > ~/Hackers-Nightlight/WYZE/LIGHT.md << 'LIGHTMD'
# Wyze Bulb Color (WLPA19CV2) — Working LED Control

This document covers the **LED / light-control additions** for the Wyze Bulb Color
build: a working `BP5758D` driver and a `/light` web page for setting colour. It
does not cover the rest of the firmware.

The stock build shipped with LED control marked "WIP" — the `/setcolor` endpoint
didn't actually change the light. These changes fix that.

## What was wrong

The Wyze Bulb Color drives its LEDs through a **BP5758D**, a 5-channel
constant-current LED driver, over a bit-banged two-wire protocol. The original
`/setcolor` handler instead called `ledcWrite()` (the ESP32 LEDC **PWM**
peripheral) on GPIOs copied from the Vont variant — pins that aren't connected to
the LEDs on this board, on LEDC channels that were never initialised. Every colour
request was a no-op. That's why colour control never worked on this bulb.

## What these changes add

New files:
- `WYZE/include/components/bp5758d.h`
- `WYZE/src/components/bp5758d.cpp` — the BP5758D driver
- `WYZE/include/components/webserver/page_light.h` — the gzipped `/light` page

Modified:
- `WYZE/src/system_manager.cpp` — initialises the driver at boot
- `WYZE/src/components/webserver/webserver.cpp` — `/setcolor` now drives the BP5758D; adds the `/light` route
- `WYZE/include/components/webserver/webserver.h` — declares the `/light` handler

## The driver

The BP5758D is bit-banged on:
- **GPIO19 → SDA**
- **GPIO18 → SCL**

Channel → hardware-output mapping, with the per-channel current-range bytes
preserved from the stock init sequence. **Do not change the current bytes without
measuring LED current** — the values below are what the bulb ships with.

| Output | Colour | Current |
|--------|--------|---------|
| OUT1   | Blue   | 0x10 |
| OUT2   | Green  | 0x10 |
| OUT3   | Red    | 0x10 |
| OUT4   | Warm   | 0x1A |
| OUT5   | Cold   | 0x1A |

Grayscale is a 10-bit value per channel, sent low-5-bits then high-5-bits. The
driver adds gamma correction (perceptually linear dimming) and optional smooth
fades. `set()` is gamma-corrected; `setRaw()` is linear, for calibration.

## Wiring for flashing

Flash over UART with a **3.3 V** USB-serial adapter (e.g. CP2102). **Six wires.**
Colours below match one common legend — adapt to your own.

| Wire   | Bulb pad     | Adapter pin |
|--------|--------------|-------------|
| Black  | GND          | GND |
| Red    | 3.3V         | 3V3 |
| Green  | TX (GPIO21)  | RXD |
| Blue   | RX (GPIO20)  | TXD |
| Orange | GPIO8        | 3V3 |
| White  | GPIO9        | GND |

Notes:
- **Serial crosses.** Bulb TX -> adapter RX, bulb RX -> adapter TX. The ROM
  bootloader's UART is fixed to GPIO21 (TX) / GPIO20 (RX). If unsure which pad is
  which, buzz continuity to module pins 20/21 rather than trusting silkscreen.
- **Strap = bootloader.** GPIO8 high + GPIO9 low at reset forces the ESP32-C3 into
  the serial bootloader. **Remove those two wires after flashing** so it boots
  normally. Safer than hard-tying: GPIO8 high through ~10k, GPIO9 low through ~1k.
- **3.3 V, not 5 V.** Check the adapter's voltage jumper. 5 V will damage the module.
- **Never connect mains** while the adapter is attached.
- **Power.** The CP2102's onboard 3.3 V regulator is weak; the ESP32-C3 can brown
  out (`rst:0xf` in the boot log). If so, add a 470-1000uF cap across the bulb's
  3V3/GND, or power the module from an external 3.3 V supply and leave the
  adapter's 3V3 pin disconnected.

## Building (PlatformIO)

PlatformIO isn't in Kali's apt repos. Install it in a venv:
