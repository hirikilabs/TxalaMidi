#ifndef __CONFIG_H
#define __CONFIG_H

#include <Preferences.h>

#define CONFIG_READ_OK 0
#define CONFIG_ERROR -1
#define CONFIG_NAMESPACE "TxalaConfig"
#define PREFS_KEY_ROOTNOTE "RootNote"
#define PREFS_KEY_SCALE "Scale"
#define PREFS_KEY_MIDICH "MidiCh"

typedef struct {
  uint8_t midi_ch;
  uint8_t root_note;
  uint8_t scale;
} config_data_t;

// class Config {
// private:
//   Preferences prefs;
//   uint8_t midi_ch;
//   uint8_t scale;
//   uint8_t root_note;

// public:
//   Config();
//   void read();
//   void save();
//   uint8_t get_midi_ch() { return midi_ch; }
//   uint8_t get_scale() { return scale; }
//   uint8_t get_root_note() { return root_note; }
//   void set_midi_ch(uint8_t ch) { midi_ch = ch; }
//   void set_scale(uint8_t s) { scale = s; }
//   void set_root_note(uint8_t note) { root_note = note; }
// };

int config_read(config_data_t *config);
void config_save(config_data_t config);

#endif
