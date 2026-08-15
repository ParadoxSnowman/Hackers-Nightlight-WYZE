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
- **Serial crosses.** Bulb TX → adapter RX, bulb RX → adapter TX. The ROM
  bootloader's UART is fixed to GPIO21 (TX) / GPIO20 (RX). If unsure which pad is
  which, buzz continuity to module pins 20/21 rather than trusting silkscreen.
- **Strap = bootloader.** GPIO8 high + GPIO9 low at reset forces the ESP32-C3 into
  the serial bootloader. **Remove those two wires after flashing** so it boots
  normally. Safer than hard-tying: GPIO8 high through ~10 kΩ, GPIO9 low through
  ~1 kΩ.
- **3.3 V, not 5 V.** Check the adapter's voltage jumper. 5 V will damage the module.
- **Never connect mains** while the adapter is attached.
- **Power.** The CP2102's onboard 3.3 V regulator is weak; the ESP32-C3 can brown
  out (`rst:0xf` in the boot log). If so, add a 470–1000 µF cap across the bulb's
  3V3/GND, or power the module from an external 3.3 V supply and leave the
  adapter's 3V3 pin disconnected.

## Building (PlatformIO)

PlatformIO isn't in Kali's apt repos. Install it in a venv:

```
python3 -m venv ~/.pio-venv
~/.pio-venv/bin/pip install platformio
echo 'alias pio="$HOME/.pio-venv/bin/pio"' >> ~/.zshrc
source ~/.zshrc
```

**Case-sensitivity fix (Linux).** The source `#include`s reference
`components/webserver/` (lowercase) but the directories ship as `WebServer`.
Windows ignores this; Linux doesn't. Rename them:

```
cd WYZE
mv include/components/WebServer include/components/webserver
mv src/components/WebServer src/components/webserver
```

Build:

```
cd WYZE
pio run -e esp32-c3-devkitm-1
```

If a rebuild finishes in a couple of seconds with no `Compiling` lines after you've
edited a file, PlatformIO is reusing cached objects on a stale timestamp. Force a
clean build:

```
pio run -e esp32-c3-devkitm-1 -t clean
pio run -e esp32-c3-devkitm-1
```

## Flashing

The board must be strapped into the bootloader (straps on, above). Since DTR/RTS
aren't wired, tell esptool not to auto-reset.

**Back up stock firmware first** — it's your only rollback, and it holds the
device's keys:

```
esptool --chip esp32c3 -p /dev/ttyUSB0 -b 115200 --before no_reset --after no_reset \
  read_flash 0 0x400000 wyze_stock.bin
```

Flash the three build artifacts:

```
esptool --chip esp32c3 -p /dev/ttyUSB0 -b 115200 --before no_reset --after no_reset \
  write_flash --flash_mode qio --flash_size 4MB \
  0x0 .pio/build/esp32-c3-devkitm-1/bootloader.bin \
  0x8000 .pio/build/esp32-c3-devkitm-1/partitions.bin \
  0x10000 .pio/build/esp32-c3-devkitm-1/firmware.bin
```

Three `Hash of data verified.` = success. Then **remove the GPIO8 and GPIO9 strap
wires** and power-cycle — it now boots the firmware.

> `esptool.py` was renamed `esptool` in v4.x. esptool v5 wants hyphens:
> `write-flash`, `read-flash`.

## Using the /light page

Once running, the bulb is a WiFi AP. Join it, then open:

```
http://192.168.4.1/light
```

Colour picker, brightness slider, presets, and a warm-white tab. Tapping a colour
POSTs to `/setcolor`.

### /setcolor API

`POST /setcolor`, JSON body.

RGB:
```json
{"brightness":"1024","rgb":{"r":255,"g":0,"b":0}}
```

Warm white:
```json
{"brightness":"1024","warm":"800"}
```

- `brightness` — 0–1024 master level (string or number)
- `rgb.r` / `rgb.g` / `rgb.b` — 0–255 each
- `warm` — 0–1024
- `fade` (optional) — `false` for an instant change instead of a fade

Example:
```
curl -X POST http://192.168.4.1/setcolor \
  -H "Content-Type: application/json" \
  -d '{"brightness":"1024","rgb":{"r":255,"g":0,"b":0}}'
```

## Troubleshooting

**Serial port keeps renumbering / disconnects every ~60 s.**
Kali's `brltty` (braille daemon) claims CP210x adapters and cycles them. Remove it:
```
sudo apt remove brltty
```
For a device name that survives re-plugging, add a udev rule:
```
echo 'SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", SYMLINK+="bulb"' | sudo tee /etc/udev/rules.d/99-bulb.rules
sudo udevadm control --reload-rules
```
Re-plug, then use `/dev/bulb` everywhere.

**`esptool: command not found`** — it's `esptool`, not `esptool.py`, in current
versions. Or install with `pip install esptool`.

**Colour is wrong** (ask for red, get green) — the OUT→colour order in `set()` in
`bp5758d.cpp` needs adjusting for your board revision. Reorder the `out[]` array.

**`rst:0xf (BROWNOUT_RST)` in the boot log** — supply sag. See the power note under
Wiring.

**Verifying without lighting the LED.** With the module desoldered or off mains you
can't see colour, but you can confirm the driver is clocking data: probe GPIO19
(SDA) or GPIO18 (SCL) with a multimeter — it idles ~3.3 V and twitches on each
colour change — or a logic analyser. Combined with the `/light` page responding,
that confirms the full path short of the LEDs themselves.

## Credit

BP5758D protocol adapted from the driver implementations in Tasmota and ESPHome.
