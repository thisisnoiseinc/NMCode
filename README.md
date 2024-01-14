# NMCode
The Brains That Make Noise Machine (NMSVE) Tick.

## Prerequisites
 * https://github.com/espressif/arduino-esp32

## Programming

**USB to TTL Converter**

You'll need a USB to TTL Converter to connect Noise Machine (NMSVE) to your PC.

See below for reference:

https://a.co/d/i3A57Qw

**Pin Diagram**

![alt text](https://github.com/thisisnoiseinc/NMCode/blob/main/Programming/Pins.png)

Top to Bottom (in picture above):

BOOT
 <br />
EN
 <br />
GND
 <br />
3V3
 <br />
RX
 <br />
TX

**Arduino**

Use "Firebeetle-ESP32" as the board when uploading.

## Notes
 Originally based off neilbags code:
 * https://github.com/neilbags/arduino-esp32-BLE-MIDI
