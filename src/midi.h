#ifndef __MIDI_H
#define __MIDI_H
#include "esp_event_base.h"
#include <inttypes.h>
#include <stddef.h>

// Basic MIDI Messages
#define NOTE_OFF 0x80
#define NOTE_ON 0x90
#define MIDI_CC 0xB0

// scale names
#define SCALE_CHROMATIC 0
#define SCALE_MAJOR 1
#define SCALE_NAT_MINOR 2
#define SCALE_HARM_MINOR 3
#define SCALE_MEL_MINOR 4
#define SCALE_DORIAN 5
#define SCALE_PHRYGIAN 6
#define SCALE_LYDIAN 7
#define SCALE_MIXOLIDIAN 8
#define SCALE_LOCRIAN 9

// midi variables declaration
extern uint8_t scale_semitones[10][8];
extern char note_names[12][3];
extern char scale_names[10][15];

// From usb.org MIDI 1.0 specification. This 4 byte structure is the unit
// of transfer for MIDI data over USB.
typedef struct __attribute__((__packed__)) {
  uint8_t code_index_number : 4;
  uint8_t cable_number : 4;
  uint8_t MIDI_0;
  uint8_t MIDI_1;
  uint8_t MIDI_2;
} USB_MIDI_t;

typedef struct {
  uint8_t channel;
  uint8_t root_note;
  uint8_t scale;
  uint8_t module_notes[8];
} midi_data_t;

void midi_init_data(midi_data_t *data, uint8_t channel, uint8_t root,
                    uint8_t scale);
void midi_init_scale(midi_data_t *data);
uint8_t midi_get_octave(uint8_t note);
char *midi_get_note_name(uint8_t note);
void midi_transpose_notes(midi_data_t *data, uint8_t value);
uint16_t midi_usb_load_descriptor(uint8_t *dst, uint8_t *itf);
void midi_usb_event_callback(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data);
void midi_usb_note_on(uint8_t channel, uint8_t pitch, uint8_t velocity);
void midi_usb_note_off(uint8_t channel, uint8_t pitch, uint8_t velocity);
void midi_usb_cc_change(uint8_t channel, uint8_t control, uint8_t value);

#endif
