# ESP32-C3 Super Mini + SH1106 OLED (INMP441 + MAX98357A)

Build variant: `esp32-c3-super-mini-sh1106` (board type `esp32-c3-super-mini`, target `esp32c3`, 4 MB flash).

## Wiring

| Function | Pin |
|---|---|
| OLED SDA / SCL (I2C 0x3C, SH1106 128x64) | GPIO21 / GPIO20 |
| I2S BCLK (INMP441 SCK + MAX98357A BCLK) | GPIO2 |
| I2S WS (INMP441 WS + MAX98357A LRC) | GPIO1 |
| INMP441 SD (mic data in) | GPIO8 |
| MAX98357A DIN (speaker data out) | GPIO3 |
| BOOT button | GPIO9 |

INMP441 L/R -> GND (left slot). MAX98357A VIN -> 5V.

## Build

```sh
source /path/to/esp-idf/export.sh      # ESP-IDF >= 6.0.1
python3 scripts/build.py esp32-c3-super-mini --name esp32-c3-super-mini-sh1106
```

## Notes

- Display driver: `esp_lcd_new_panel_sh1106()` from the `tny-robotics/sh1106-esp-idf` component
  (already declared in `main/idf_component.yml`). It handles the SH1106 2-column RAM offset itself.
- GPIO20/21 are UART0 pins on the C3, so the console is moved to USB-Serial-JTAG.
- The SH1106 driver's mirror flags are inverted vs SSD1306. If the image is upside down, set
  `DISPLAY_MIRROR_X` / `DISPLAY_MIRROR_Y` to `true` in `config.h`.
- No LED is defined: the Super Mini's on-board LED (GPIO8) is the mic data line in this wiring.
- 4 MB layout (`partitions/v2/4m.csv`) has a single factory app slot, so no dual-slot OTA.
