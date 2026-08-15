<p align="center">
  <img src="https://github.com/user-attachments/assets/6d3e3b2f-939f-49c1-91ad-322a55c86a11" alt="Centered Image" width="200"/>
</p>

<h1 align="center">Announcing Hackers Nightlight V2</h1>

<p align="center">Hackers nightlight V2 is inspired by the open source project's idea of creating a covert penetration testing tool to help uncover hardware vulnerabilities, test network security and response readiness. </p>

<p align="center">Hackers nightlight V2 brings new custom hardware packed inside of the same discrete light bulb format, containing a brand new webUI with focus on ease of use, and a suite of new abilities and tools.  </p>

<p align="center">Learn more here: https://hackersnightlight.com </p>

---

> ## About this fork
>
> This is a fork focused on **making the LED / light control on the Wyze Bulb Color
> actually work and be usable.** Upstream shipped the Wyze color control as "WIP" —
> the endpoint didn't drive the light. This fork adds a working `BP5758D` driver and
> a dedicated web page (`<IP>/light`) for changing color and brightness from a phone
> or browser.
>
> **This fork does not touch the pentesting features.** It only fixes and adds the
> lighting side; everything else is unchanged from upstream. The updated Wyze
> walkthrough is in the [Wyze Bulb Color](#wyze-bulb-color-wlpa19cv2) section below,
> and a full driver / API / build reference is in [`WYZE/LIGHT.md`](WYZE/readme.md).

---

## Hacker-Nightlight

![324166552-2f7a9811-08fe-47ba-a03e-0092ca4ed871 (1)](https://github.com/Peaakss/Hacker-Nightlight/assets/115900893/678e1534-e29e-462e-a4bd-22b98f3bd397)

the Wi-Fi hacking light bulb ( More models coming soon )


# Context

The Hacker night light project is meant to show the attack possibilities and vulnerabilities with smart light bulbs.


WiFi-connected smart lights are modern lighting solutions that you can control with your phone or voice commands. They work by connecting to your home WiFi network, allowing you to adjust them remotely. You can change the brightness, color, and even set schedules for when they turn on or off. They're easy to install and can be used in various forms like bulbs or light strips. Plus, they help save energy and add convenience to your home by letting you customize your lighting preferences easily.


lights often use simple WiFi-enabled microcontrollers to manage connections and control the LED arrays. These microcontrollers are like tiny computers embedded within the smart light system. They handle tasks such as connecting to your home WiFi network, communicating with your smartphone or other devices, and controlling the LEDs

![Untitled-5](https://github.com/Peaakss/Hacker-Nightlight/assets/115900893/72595671-05d8-4ed1-a157-279d740cc9cb) (Govee ‎B60081C3)
                                                          



Different models of lights will use different microcontrollers for these operations. Certain microcontrollers possess the capability to perform WiFi network penetration or exploitation.
One model is the ESP32-C3. Capability of doing such this such as 

* PMKID capture
* Deauthentication attacks using various methods
* WPA/WPA2 handshake capture and parsing
* Passive handshake sniffing
* Packet sniffing

With many open source firmwares public on github such as https://github.com/risinek/esp32-wifi-penetration-tool

# Soldering Uart connections

Using a $7 USB to TTL on amazon we can connect the TX to RX together on the TTL USB while also grounding IO9 on the light bulb PCB, setting the ESP32-C3 into boot loader mode from there we can flash our new firmware. Once the chip is flashed, unground IO9 and power cycle and the ESP will reboot into the newly flashed firmware

![Capture22](https://github.com/Peaakss/Hacker-Nightlight/assets/115900893/7d2a8d30-3ea9-43a4-a269-bcbafb421ee9)

Once connected we are able to read and write to the flash 

(USB to TTL: https://www.amazon.com/dp/B00LODGRV8)

![Untitled-2](https://github.com/Peaakss/Hacker-Nightlight/assets/115900893/3faa23ab-5814-478f-b56d-15a533a8d59e)


# Flashing 

Donwload the Flashtool from https://www.espressif.com/en/support/download/other-tools 

DO NOT FLASH WHILE OR PLUG IN USB ADAPTER WHILE LIGHTBULB IS ON 120V or 240V POWER SOURCE!!! 

when opening the flashtool, you will want to set Chiptype to ESP32-C3 and make sure WorkMode is on Develop and LoadMode is on UART

Once loaded, set the values as follows 

1. bootloader.bin @ 0x0
2. partitions.bin @ 0x8000
3. firmware.bin @ 0x10000

Ensure your SPI mode is set to QIO and baud rate is at 115200

![Capture66](https://github.com/Peaakss/Hacker-Nightlight/assets/115900893/a9f9c861-25b8-4685-a90f-1d4cd26a7d59)


# After flashing

once you flash the ESP32-C3 you will want to remove you will want to unpower the device and remove it from bootloader, once you have done that you can either 

1. plug in the USB to UART (LED array will not be on)
2. remove UART cables from USB and plug light into E26 socket (lamp or light socket)

**DEFAULT SSID:** Nightlight

**DEFAULT Password:** Nightlight12345

from there you will want to connect to the Nightlight AP using the Password "Nightlight12345" then navigate to the web page hosted on 192.168.4.1

![gdjgndrkgndr](https://github.com/Peaakss/Hacker-Nightlight/assets/115900893/28d1392a-6f49-4e97-bf9e-9aef69b9064e)

to change light colors scroll down to the light control page

![fefeljfblekf](https://github.com/Peaakss/Hacker-Nightlight/assets/115900893/4542043f-0afd-4369-8c15-6d5f36757b61)

Click the box, this is give you a color picker to select a RGB code for the light

YOU CANNOT HAVE RBG AND WARM/COLD LIGHTS ON AT THE SAME TIME, if you move the warm/cold slider it will turn on the warm/cold lights, to turn RBG back on slide it back and the RBG will turn back on

Tiktok Demo: https://www.tiktok.com/@o.mg_peaks/video/7360587336507280683

# Wyze Bulb Color (WLPA19CV2) 

The Wyze bulb colors use the exact same MCU as the vont color lights. ESP32-C3, however have a diffrent flashing process.

Bulb link: https://www.amazon.com/dp/B097C3VLLL

![IMG_1663](https://github.com/hak5peaks/Hackers-Nightlight/assets/115900893/f475e295-e994-411f-8fcc-7a32f0029c96)

> **This fork makes the Wyze color control work.** The Wyze drives its LEDs through a
> **BP5758D** constant-current driver, which upstream never wired up — so color was a
> no-op. This fork adds a real driver plus a `<IP>/light` page to change color and
> brightness. Full technical reference: [`WYZE/LIGHT.md`](WYZE/LIGHT.md).
>
> **Check the chip first.** Wyze has shipped more than one hardware revision of this
> bulb. This firmware targets the **ESP32-C3** version — confirm the module inside is
> an ESP32-C3 before taking one apart.

## Opening the bulb (go slow — the plastic is brittle)

The housing isn't built to come apart. Expect to fight it, and expect casualties if
you rush.

* **The outer case may crack.** Work around the seam slowly and evenly — prying hard
  in one spot is how it splits.
* **The LED module connector is fragile.** When you separate the LED board from the
  driver board, the inner plastic around those connector pins can crack. Ease it
  apart; don't yank the connector.
* **Digging out the potting/rubber is the worst part.** Keep a trash bin right next to
  you — it's messy and there's a lot of it.

![Bulb PCB after teardown](images/pcb.jpg)

## Soldering the flash leads

Solder six wires to the ESP32-C3 for UART flashing. **The copper pads tear off
easily** — use flux, a fine tip, and get in and out in a couple of seconds per joint.
If a pad lifts, solder directly to the module pin instead.

| Wire   | Bulb pad     | CP2102 pin |
|--------|--------------|------------|
| Black  | GND          | GND |
| Red    | 3.3V         | 3V3 |
| Green  | TX (GPIO21)  | RXD |
| Blue   | RX (GPIO20)  | TXD |
| Orange | GPIO8        | 3V3 |
| White  | GPIO9        | GND |

Unlike the Vont (where you only ground IO9), the Wyze needs **both** straps:

```
GPIO8 -> HIGH  (bridge to 3.3V)
GPIO9 -> LOW   (bridge to GND)
```

* **Serial crosses:** bulb TX -> dongle RX, bulb RX -> dongle TX.
* **Make sure none of your connector pins touch.** This is the most finicky part — a
  bridge between adjacent pins stops the flash cold. Check continuity with a
  multimeter before powering anything.
* **Confirm the dongle is on 3.3 V, not 5 V.** 5 V can kill the module.

## Flashing (Linux)

Plug the soldered leads into the CP2102 dongle, then into USB. With the GPIO8/GPIO9
straps in place, the chip boots into bootloader mode ready to flash.

**1. Install esptool**
```
sudo apt install esptool     # or: pip install esptool
```

**2. Find the port**
```
ls /dev/ttyUSB*
```
Usually `/dev/ttyUSB0`. If the port keeps disappearing or renumbering, Kali's
`brltty` daemon is cycling the adapter — `sudo apt remove brltty`, then replug.

**3. Back up stock firmware first (don't skip — it's your only rollback and holds the device keys)**
```
esptool --chip esp32c3 -p /dev/ttyUSB0 -b 115200 --before no_reset --after no_reset \
  read_flash 0 0x400000 wyze_stock.bin
```

**4. Flash** (building from source? see [`WYZE/LIGHT.md`](WYZE/LIGHT.md))
```
esptool --chip esp32c3 -p /dev/ttyUSB0 -b 115200 --before no_reset --after no_reset \
  write_flash --flash_mode qio --flash_size 4MB \
  0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```
Three `Hash of data verified.` lines = success. If it resets partway or reports
`BROWNOUT_RST`, the CP2102's regulator is sagging — add a 470-1000uF cap across the
bulb's 3V3/GND, or power the module from an external 3.3 V source.

## After flashing

1. Unplug the dongle from USB.
2. **Remove the GPIO8 and GPIO9 strap wires** (orange and white, off 3.3V and GND).
   Leaving them on keeps the chip in bootloader mode and it won't run.
3. Plug back in to power it.

Join the bulb's AP, open its IP for the main page, then go to **`<IP>/light`** for the
color and brightness controls. Tap a color, drag the brightness slider, or use the
warm-white tab — no app, no terminal.

# [Big thanks to https://github.com/Spooks4576 for assisting in the creation of the firmware]
