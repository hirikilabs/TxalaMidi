#include "gui.h"
#include "midi.h"
#include "txala.h"
#include <FunctionalInterrupt.h>

static uint8_t right_flag, left_flag, pressed_flag;
static gui_selected_t selected;
static unsigned long lastButton;

// inits OLED screen and GUI data structures
void gui_init(U8G2 *screen) {
  // gui inital state
  right_flag = left_flag = pressed_flag = false;
  selected = GUI_SELECTED_NONE;

  // start the OLED screen
  screen->begin();

  // setup the encoder GPIO and interrupts
  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);
  pinMode(ENC_SW, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_A), gui_enc_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), gui_enc_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_SW), gui_button_isr, FALLING);
}

// encoder interrupt callback
// sets flags for the encoder moving left or right
void IRAM_ATTR gui_enc_isr() {
  static uint8_t old_AB = 3; // lookup table index
  static int8_t encval = 0;  // encoder value
  static const int8_t enc_states[] = {0, -1, 1, 0, 1, 0,  0, -1, -1,
                                      0, 0,  1, 0, 1, -1, 0}; // lookup table

  old_AB <<= 2; // remember previous state

  if (digitalRead(ENC_A))
    old_AB |= 0x02; // add current state of pin A
  if (digitalRead(ENC_B))
    old_AB |= 0x01; // add current state of pin B

  encval += enc_states[(old_AB & 0x0f)];

  // update counter if encoder has rotated a full indent, that is at least 4
  // steps
  if (encval > 3) {    // four steps forward
    right_flag = true; // right
    encval = 0;
  } else if (encval < -3) { // four steps backwards
    left_flag = true;       // left
    encval = 0;
  }
}

// encoder button interrupt callback
// sets flag if the encoder button is pressed
// and iterates through configuration options
void IRAM_ATTR gui_button_isr() {
  if (millis() - lastButton > 150) {
    pressed_flag = true;
    switch (selected) {
    case GUI_SELECTED_NONE:
      selected = GUI_SELECTED_ROOT;
      break;
    case GUI_SELECTED_ROOT:
      selected = GUI_SELECTED_SCALE;
      break;
    case GUI_SELECTED_SCALE:
      selected = GUI_SELECTED_MIDICH;
      break;
    case GUI_SELECTED_MIDICH:
      selected = GUI_SELECTED_NONE;
      break;
    }
    lastButton = millis();
  }
}

// updates OLED screen
void gui_update(U8G2 *screen) { screen->sendBuffer(); }

// clears the screen
void gui_clear(U8G2 *screen) { screen->clearBuffer(); }

// flag getters
bool gui_get_left_flag() { return left_flag; }
bool gui_get_right_flag() { return right_flag; }
bool gui_get_pressed_flag() { return pressed_flag; }
gui_selected_t gui_get_selected() { return selected; }

// gui actions
void gui_left_action() { left_flag = false; }
void gui_right_action() { right_flag = false; }
void gui_pressed_action() { pressed_flag = false; }

// shows splash screen
void gui_splash(U8G2 *screen) {
  screen->clearBuffer();
  screen->setFont(FONT_TITLE);
  screen->drawStr(35, 20, "TxalaMidi");
  screen->setFont(FONT_SMALL);
  screen->drawStr(40, 40, "Ver. 0.1");
  screen->sendBuffer();
}

// shows gui main data
void gui_show_data(U8G2 *screen, midi_data_t *data) {
  // root note
  char root_note_name[4];
  char mod_notes[22];
  root_note_name[2] = ' ';
  root_note_name[3] = 0;
  snprintf(root_note_name, 4, "%s%d", midi_get_note_name(data->root_note),
           midi_get_octave(data->root_note));
  screen->setDrawColor(1);
  screen->setFont(FONT_ROOTNOTE);
  if (selected == GUI_SELECTED_ROOT) {
    screen->drawButtonUTF8(0, 25, U8G2_BTN_INV, 0, 2, 2, root_note_name);
  } else {
    screen->drawStr(0, 25, root_note_name);
  }
  // scale
  screen->setFont(FONT_TITLE);
  if (selected == GUI_SELECTED_SCALE) {
    screen->drawButtonUTF8(0, 44, U8G2_BTN_INV, 0, 2, 2,
                           scale_names[data->scale]);
  } else {
    screen->drawStr(0, 44, scale_names[data->scale]);
  }
  // midi ch
  screen->setFont(FONT_SMALL);
  char midich[8];
  snprintf(midich, 8, "Ch: %d", data->channel + 1);
  if (selected == GUI_SELECTED_MIDICH) {
    screen->drawButtonUTF8(90, 12, U8G2_BTN_INV, 0, 2, 2, midich);
  } else {
    screen->drawStr(90, 12, midich);
  }

  // module notes
  snprintf(mod_notes, 22, "%s %s %s %s %s %s %s %s",
           midi_get_note_name(data->module_notes[0]),
           midi_get_note_name(data->module_notes[1]),
           midi_get_note_name(data->module_notes[2]),
           midi_get_note_name(data->module_notes[3]),
           midi_get_note_name(data->module_notes[4]),
           midi_get_note_name(data->module_notes[5]),
           midi_get_note_name(data->module_notes[6]),
           midi_get_note_name(data->module_notes[7]));
  screen->drawStr(0, 63, mod_notes);
}
