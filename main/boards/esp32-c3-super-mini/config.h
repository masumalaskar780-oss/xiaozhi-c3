#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

#include <driver/gpio.h>

// ESP32-C3 Super Mini + INMP441 mic + MAX98357A amp + 1.3" SH1106 128x64 I2C OLED.
// Wiring follows the circuit diagram supplied with this board.

// ---------------------------------------------------------------------------
// Audio: the INMP441 mic and the MAX98357A amp share BCLK and WS, so this is
// a full-duplex I2S port (the ESP32-C3 has only one I2S controller anyway).
// Duplex mode clocks RX and TX from one rate, so both rates must match.
// ---------------------------------------------------------------------------
#define AUDIO_INPUT_SAMPLE_RATE  24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_2   // INMP441 SCK  + MAX98357A BCLK
#define AUDIO_I2S_GPIO_WS   GPIO_NUM_1   // INMP441 WS   + MAX98357A LRC
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_8   // INMP441 SD (mic data -> ESP32)
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_3   // MAX98357A DIN (ESP32 -> amp)

// ---------------------------------------------------------------------------
// Buttons: BOOT button on the Super Mini. There is no LED define on purpose:
// the on-board LED sits on GPIO8, which is the mic data line here.
// ---------------------------------------------------------------------------
#define BOOT_BUTTON_GPIO GPIO_NUM_9

// ---------------------------------------------------------------------------
// Display: 1.3" SH1106, 128x64, I2C
// GPIO20/21 are the default UART0 RX/TX pins on the C3, so the console is
// moved to USB-Serial-JTAG in config.json (CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG).
// ---------------------------------------------------------------------------
#define DISPLAY_I2C_PORT  I2C_NUM_0
#define DISPLAY_SDA_PIN   GPIO_NUM_21
#define DISPLAY_SCL_PIN   GPIO_NUM_20
#define DISPLAY_I2C_ADDR  0x3C

#define DISPLAY_WIDTH   128
#define DISPLAY_HEIGHT  64

// NOTE: the SH1106 driver's mirror flags are inverted relative to SSD1306:
// its init already applies SEG remap + COM reverse (the usual upright setup),
// so mirror (false,false) == upright and (true,true) == rotated 180 degrees.
// If the picture is upside down on your module, flip both to true.
#define DISPLAY_MIRROR_X false
#define DISPLAY_MIRROR_Y false

#endif // _BOARD_CONFIG_H_
