#ifndef __TXALA_H
#define __TXALA_H

// GPIO
#define I2C_SCL 2
#define I2C_SDA 3

#define LED_PIN 21

#define ENC_A 4
#define ENC_B 5
#define ENC_SW 6

#define OLED_CLK 7
#define OLED_MOSI 8
#define OLED_DC 9
#define OLED_CS 10
#define OLED_RST 11

// MIDI
#define MIDI_CHANNEL 0
#define TRANSPOSE_CC 65

// CHANNELS
#define NUM_CHANNELS 2
#define CHANNEL_THRESHOLD 5

// GUI
#define FONT_SMALL u8g2_font_profont12_tf
#define FONT_TITLE u8g2_font_doomalpha04_tr
#define FONT_ROOTNOTE u8g2_font_logisoso24_tf

#endif
