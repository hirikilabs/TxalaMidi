#include "USB.h"
#include "channels.h"
#include "config.h"
#include "esp32-hal-tinyusb.h"
#include "gui.h"
#include "midi-notes.h"
#include "midi.h"
#include "txala.h"
#include <Adafruit_ADS7830.h>
#include <Arduino.h>
#include <ESP32RotaryEncoder.h>
#include <SPI.h>
#include <TinyLED.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <stdint.h>

// debug
// static const char *TAG = "txalamidi";

// types

// global vars
midi_data_t midi_data;
channel_data_t channel_data;
// char root_note_name[4];
int transpose_value = 63; // CCs are 0-127 so 63 is the middle

// objects
TinyLED<LED_PIN, 1, ORDER_RGB> led;
Adafruit_ADS7830 ad7830;
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, OLED_CS, OLED_DC,
                                            OLED_RST);
////////// MAIN TASKS ///////////

// read MIDI messages (TODO)
static void task_midi_read(__attribute__((unused)) void *arg) {
  // The MIDI interface always creates input and output port/jack descriptors
  // regardless of these being used or not. Therefore incoming traffic should be
  // read (possibly just discarded) to avoid the sender blocking in IO.
  // MIDI data available is in the USB_MIDI_t packet structure, m->cable_number,
  // m->code_index_number and midi bytes m->MIDI_0, MIDI_1 and MIDI_2
  uint8_t packet[4];
  bool read = false;
  for (;;) {
    delay(1);
    while (tud_midi_available()) {
      read = tud_midi_packet_read(packet);
      if (read) {
        USB_MIDI_t *m = (USB_MIDI_t *)packet;
        // CC on our channel?
        uint8_t channel = m->MIDI_0 & 0x0f;
        if (channel == MIDI_CHANNEL) {
          uint8_t message = m->MIDI_0 & 0xf0;
          if (message == MIDI_CC) {
            // ok, what CC?
            if (m->MIDI_1 == TRANSPOSE_CC) {
              transpose_value = m->MIDI_2;
              midi_transpose_notes(&midi_data, transpose_value);
            }
          }
        }
      }
    }
  }
}

static void task_midi_write(__attribute__((unused)) void *arg) {
  channel_trend_t new_trends[NUM_CHANNELS];

  for (;;) {
    delay(1);
    // read ADC channels
    // move last values up the array
    channel_move_last_values(&channel_data);
    // and read new data
    for (size_t ch = 0; ch < NUM_CHANNELS; ch++) {
      channel_data.last_values[0][ch] = ad7830.readADCsingle(ch) / 2;

      // calculate trends
      new_trends[ch] = channel_get_trend(&channel_data, ch);

      // check what we need to do for each channel
      // if last was off and now rising, note on
      if (channel_data.trends[ch] == CHANNEL_TREND_OFF &&
          new_trends[ch] == CHANNEL_TREND_UP) {
        channel_data.actions[ch] = CHANNEL_ACTION_NOTEON;
        led[0] = CRGB::Blue;
        led.show();
        // if sinking and now off, note off
      } else if (channel_data.trends[ch] == CHANNEL_TREND_DOWN &&
                 new_trends[ch] == CHANNEL_TREND_OFF) {
        channel_data.actions[ch] = CHANNEL_ACTION_NOTEOFF;
        led[0] = CRGB::Black;
        led.show();
        // if sinking and now rising, note off then note on
      } else if (channel_data.trends[ch] == CHANNEL_TREND_DOWN &&
                 new_trends[ch] == CHANNEL_TREND_UP) {
        channel_data.actions[ch] = CHANNEL_ACTION_NOTEOFFON;
      } else {
        channel_data.actions[ch] = CHANNEL_ACTION_NOTHING;
      }

      // execute channel actions
      if (channel_data.actions[ch] == CHANNEL_ACTION_NOTEON) {
        midi_usb_note_on(midi_data.channel, midi_data.module_notes[ch],
                         channel_data.last_values[0][ch]);
      } else if (channel_data.actions[ch] == CHANNEL_ACTION_NOTEOFF) {
        midi_usb_note_off(midi_data.channel, midi_data.module_notes[ch],
                          channel_data.last_values[0][ch]);
      } else if (channel_data.actions[ch] == CHANNEL_ACTION_NOTEOFFON) {
        midi_usb_note_off(midi_data.channel, midi_data.module_notes[ch],
                          channel_data.last_values[1][ch]);
        midi_usb_note_on(midi_data.channel, midi_data.module_notes[ch],
                         channel_data.last_values[0][ch]);
      }

      // and update trends
      channel_data.trends[ch] = new_trends[ch];
    }
  }
}

static void task_gpio_control(__attribute__((unused)) void *arg) {
  for (;;) {
    delay(1);
    if (gui_get_selected() != GUI_SELECTED_NONE) {
      if (gui_get_left_flag()) {
        if (gui_get_selected() == GUI_SELECTED_ROOT) {
          midi_data.root_note--;
          if (midi_data.root_note < MIDI_C0) {
            midi_data.root_note = MIDI_C0;
          }
          midi_init_scale(&midi_data);
          gui_clear(&u8g2);
          gui_show_data(&u8g2, &midi_data);
          gui_update(&u8g2);
        } else if (gui_get_selected() == GUI_SELECTED_SCALE) {
          if (midi_data.scale > SCALE_CHROMATIC) {
            midi_data.scale--;
          }
          midi_init_scale(&midi_data);
          gui_clear(&u8g2);
          gui_show_data(&u8g2, &midi_data);
          gui_update(&u8g2);
        } else if (gui_get_selected() == GUI_SELECTED_MIDICH) {
          if (midi_data.channel > 0)
            midi_data.channel--;
          gui_clear(&u8g2);
          gui_show_data(&u8g2, &midi_data);
          gui_update(&u8g2);
        }
        gui_left_action();
      }
      if (gui_get_right_flag()) {
        if (gui_get_selected() == GUI_SELECTED_ROOT) {
          midi_data.root_note++;
          if (midi_data.root_note > MIDI_C8) {
            midi_data.root_note = MIDI_C8;
          }
          midi_init_scale(&midi_data);
          gui_clear(&u8g2);
          gui_show_data(&u8g2, &midi_data);
          gui_update(&u8g2);
        } else if (gui_get_selected() == GUI_SELECTED_SCALE) {
          midi_data.scale++;
          if (midi_data.scale > SCALE_LOCRIAN) {
            midi_data.scale = SCALE_LOCRIAN;
          }
          midi_init_scale(&midi_data);
          gui_clear(&u8g2);
          gui_show_data(&u8g2, &midi_data);
          gui_update(&u8g2);
        } else if (gui_get_selected() == GUI_SELECTED_MIDICH) {
          midi_data.channel++;
          if (midi_data.channel > 15) {
            midi_data.channel = 15;
          }
          gui_clear(&u8g2);
          gui_show_data(&u8g2, &midi_data);
          gui_update(&u8g2);
        }
        gui_right_action();
      }
    }
    if (gui_get_pressed_flag()) {
      gui_clear(&u8g2);
      gui_show_data(&u8g2, &midi_data);
      gui_update(&u8g2);
      gui_pressed_action();
      // ok, save modified values
      config_save((config_data_t){.midi_ch = midi_data.channel,
                                  .root_note = midi_data.root_note,
                                  .scale = midi_data.scale});
    }
  }
}

// main task
// just launchs the freertos threads needed
void app_main(void) {
  // MIDI Read task
  ESP_LOGI(TAG, "MIDI read task init");
  xTaskCreate(task_midi_read, "task_midi_read", 2 * 1024, NULL, 5, NULL);
  // MIDI Write task
  ESP_LOGI(TAG, "MIDI write task init");
  xTaskCreate(task_midi_write, "task_midi_write", 2 * 1024, NULL, 4, NULL);
  // GPIO (encoder) task
  ESP_LOGI(TAG, "GPIO task init");
  xTaskCreate(task_gpio_control, "task_gpio", 2 * 1024, NULL, 3, NULL);
}

/////////// Arduino Setup() /////////////

void setup() {
  Serial.begin(115200);

  Serial.println("Starting...");

  // LED
  led.setCorrection(Correction::TypicalLEDStrip);
  led.setMaxPower(5, 200); // 5V, 200 mA
  led.setBrightness(100);
  led.clear();
  led[0] = CRGB::Red;
  led.show();

  // GUI and OLED
  SPI.begin(OLED_CLK, -1, OLED_MOSI);
  gui_init(&u8g2);
  gui_splash(&u8g2);
  delay(1000);

  // ADC
  Wire.begin(I2C_SDA, I2C_SCL);
  if (!ad7830.begin()) {
    Serial.println("Failed to initialize ADS7830!");
  }
  Serial.println("Started ADS7830.");
  led[0] = CRGB::Green;
  led.show();

  // USB Midi
  USB.productName("TxalaMidi");
  USB.manufacturerName("Medialab TBK");
  USB.serialNumber("0000");
  USB.firmwareVersion(0x0001);
  USB.onEvent(midi_usb_event_callback);
  tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN,
                           midi_usb_load_descriptor);
  USB.begin();
  while (!Serial && millis() < 5000)
    delay(10);

  // initial values
  config_data_t c;
  if (config_read(&c) != 0) {
    // initial values
    c.midi_ch = 0;
    c.root_note = MIDI_C2;
    c.scale = SCALE_CHROMATIC;
  }
  midi_init_data(&midi_data, c.midi_ch, c.root_note, c.scale);
  channels_init_data(&channel_data);

  // show root note and scale
  gui_clear(&u8g2);
  gui_show_data(&u8g2, &midi_data);
  gui_update(&u8g2);

  // start threads
  app_main();
}

// Empty loop, everything is run by freertos tasks
void loop() {}
