# unsortedLEDScreen

Simple LED matrix control system for the `unsortedLED` display.

This repo combines:
- a web UI (`WebInterface/`) that sends text/settings over Web Bluetooth,
- a XIAO BLE bridge (`Xiao/webBLE_03/`) that forwards BLE messages over UART
- Teensy display sketches (`Teensy/`) that render text on a [64x32 RGB LED Matrix](https://www.adafruit.com/product/2277) using SmartLED Shield

## Quick Start

1. Flash `Xiao/webBLE_03/webBLE_03.ino` to your XIAO BLE board.
2. Flash a Teensy sketch from `Teensy/` to your matrix controller board.
3. Wire XIAO UART to Teensy UART (TX/RX + GND), matching pins in `webBLE_03.ino`.
4. Serve `WebInterface/`
5. Click **Connect**, choose `unsortedLED`, then use **Play** buttons and display settings.

## Web UI (WIP)

- Slot text (for AV1-AV4 and BREAK)
- Scroll speed
- Font color
- "SEND LOVE" special message

