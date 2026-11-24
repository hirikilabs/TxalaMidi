#include "midi.h"
#include "USB.h"
#include "esp32-hal-tinyusb.h"
#include "midi-notes.h"
#include <stdint.h>

// scales
// chromatic
// major
// natural minor
// harmonic minor
// melodic minor
// dorian
// phrygian
// lydian
// mixolidian
// locrian
uint8_t scale_semitones[10][8] = {
    {0, 1, 2, 3, 4, 5, 6, 7},   {0, 2, 4, 5, 7, 9, 11, 12},
    {0, 2, 3, 5, 7, 8, 10, 12}, {0, 2, 3, 5, 7, 8, 11, 12},
    {0, 2, 3, 5, 7, 9, 11, 12}, {0, 2, 3, 5, 7, 9, 10, 12},
    {0, 1, 3, 5, 7, 8, 10, 12}, {0, 2, 4, 6, 7, 9, 11, 12},
    {0, 2, 4, 5, 7, 9, 10, 12}, {0, 1, 3, 5, 6, 8, 10, 12},
};

char note_names[12][3] = {"C",  "C#", "D",  "D#", "E",  "F",
                          "F#", "G",  "G#", "A",  "A#", "B"};

char scale_names[10][15] = {
    "Chromatic", "Major",    "Natural minor", "Harmonic minor", "Melodic minor",
    "Dorian",    "Phrygian", "Lydian",        "Mixolidian",     "Locrian"};

// init all midi data structure fields
void midi_init_data(midi_data_t *data, uint8_t channel, uint8_t root,
                    uint8_t scale) {
  data->channel = channel;
  data->root_note = root;
  data->scale = scale;
  midi_init_scale(data);
}

// init scale notes
void midi_init_scale(midi_data_t *data) {
  for (size_t n = 0; n < 8; n++) {
    data->module_notes[n] = data->root_note + scale_semitones[data->scale][n];
  }
}

// get octave
uint8_t midi_get_octave(uint8_t note) {
  // C0 is 0x0C = 12
  note = note - 12; // now C0 = 0
  return note / 12;
}

// get note name
char *midi_get_note_name(uint8_t note) {
  note = note - 12; // C0 = 0
  return note_names[note % 12];
}

// transposes the module notes
void midi_transpose_notes(midi_data_t *data, uint8_t value) {
  // calculate transpose
  value = value - 63; // -63 to 64
  // check limits
  // low
  if ((data->root_note + value) < MIDI_C0) {
    value = MIDI_C0 - data->root_note;
  }
  // high
  if ((data->root_note + scale_semitones[data->scale][7] + value) > MIDI_B8) {
    value = MIDI_B8 - (data->root_note + scale_semitones[data->scale][7]);
  }

  // move them
  for (size_t n = 0; n < 8; n++) {
    data->module_notes[n] = data->module_notes[n] + value;
  }
}

uint16_t midi_usb_load_descriptor(uint8_t *dst, uint8_t *itf) {
  uint8_t str_index = tinyusb_add_string_descriptor("Txala MIDI");
  uint8_t ep_num = tinyusb_get_free_duplex_endpoint();
  TU_VERIFY(ep_num != 0);
  uint8_t descriptor[TUD_MIDI_DESC_LEN] = {
      // Interface number, string index, EP Out & EP In address, EP size
      TUD_MIDI_DESCRIPTOR(*itf, str_index, ep_num, (uint8_t)(0x80 | ep_num),
                          64)};
  *itf += 1;
  memcpy(dst, descriptor, TUD_MIDI_DESC_LEN);
  return TUD_MIDI_DESC_LEN;
}

void midi_usb_event_callback(__attribute__((unused)) void *arg,
                             esp_event_base_t event_base, int32_t event_id,
                             void *event_data) {
  if (event_base == ARDUINO_USB_EVENTS) {
    arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
    switch (event_id) {
    case ARDUINO_USB_STARTED_EVENT:
      Serial.println("USB PLUGGED");
      break;
    case ARDUINO_USB_STOPPED_EVENT:
      Serial.println("USB UNPLUGGED");
      break;
    case ARDUINO_USB_SUSPEND_EVENT:
      Serial.printf("USB SUSPENDED: remote_wakeup_en: %u\n",
                    data->suspend.remote_wakeup_en);
      break;
    case ARDUINO_USB_RESUME_EVENT:
      Serial.println("USB RESUMED");
      break;

    default:
      break;
    }
  }
}

void midi_usb_note_on(uint8_t channel, uint8_t pitch, uint8_t velocity) {
  if (tud_midi_mounted()) {
    uint8_t note_on[3] = {(uint8_t)(NOTE_ON | channel), pitch, velocity};
    tud_midi_stream_write(0, note_on, 3);
  }
}

void midi_usb_note_off(uint8_t channel, uint8_t pitch, uint8_t velocity) {
  if (tud_midi_mounted()) {
    uint8_t note_off[3] = {(uint8_t)(NOTE_OFF | channel), pitch, velocity};
    tud_midi_stream_write(0, note_off, 3);
  }
}

void midi_usb_cc_change(uint8_t channel, uint8_t control, uint8_t value) {
  if (tud_midi_mounted()) {
    uint8_t cc[3] = {(uint8_t)(MIDI_CC | channel), control, value};
    tud_midi_stream_write(0, cc, 3);
  }
}
