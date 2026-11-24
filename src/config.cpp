#include "config.h"
#include "midi-notes.h"
#include "midi.h"

static Preferences prefs;

int config_read(config_data_t *data) {
  if (!prefs.begin(CONFIG_NAMESPACE, false))
    return CONFIG_ERROR;
  // check if preferences exist
  if (!prefs.isKey(PREFS_KEY_MIDICH) || !prefs.isKey(PREFS_KEY_ROOTNOTE) ||
      !prefs.isKey(PREFS_KEY_SCALE)) {
    // init them
    prefs.putUChar(PREFS_KEY_ROOTNOTE, MIDI_C3);
    prefs.putUChar(PREFS_KEY_SCALE, SCALE_CHROMATIC);
    prefs.putUChar(PREFS_KEY_MIDICH, 0);
  }
  // ok, read them
  data->root_note = prefs.getUChar(PREFS_KEY_ROOTNOTE);
  data->scale = prefs.getUChar(PREFS_KEY_SCALE);
  data->midi_ch = prefs.getUChar(PREFS_KEY_MIDICH);
  prefs.end();
  return CONFIG_READ_OK;
}

void config_save(config_data_t data) {
  prefs.begin(CONFIG_NAMESPACE, false);
  prefs.putUChar(PREFS_KEY_ROOTNOTE, data.root_note);
  prefs.putUChar(PREFS_KEY_SCALE, data.scale);
  prefs.putUChar(PREFS_KEY_MIDICH, data.midi_ch);
  prefs.end();
}
