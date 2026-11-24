#include "gui.h"
#include "midi.h"
#include "txala.h"
#include <ESP32RotaryEncoder.h>
#include <FunctionalInterrupt.h>

static uint8_t right_flag, left_flag, pressed_flag;
static gui_selected_t selected;
static unsigned long lastButton;

void gui_init(U8G2 *screen) {
  right_flag = left_flag = pressed_flag = false;
  selected = GUI_SELECTED_NONE;

  screen->begin();
  // encoder = new RotaryEncoder(ENC_A, ENC_B, ENC_SW);
  selected = GUI_SELECTED_NONE;
  // encoder->setEncoderType(EncoderType::FLOATING);
  // encoder->setBoundaries(-1, 1, false);
  // encoder->onTurned(std::bind(&GUI::encoderCallback, this, _1));
  // encoder->onPressed(std::bind(&GUI::buttonCallback, this, _1));
  // encoder->begin();
  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);
  pinMode(ENC_SW, INPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_A), gui_enc_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_B), gui_enc_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_SW), gui_button_isr, FALLING);
}

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

// void GUI::encoderCallback(long value) {
//   // Don't do anything if either flag is set;
//   // it means we haven't taken action yet
//   if (right_flag || left_flag)
//     return;

//   // Set a flag that we can look for in the gpio task
//   // so that we know we have something to do
//   switch (value) {
//   case 1:
//     right_flag = true;
//     break;

//   case -1:
//     left_flag = true;
//     break;
//   }
//   // Override the tracked value back to 0 so that
//   // we can continue tracking right/left events
//   encoder->setEncoderValue(0);
// }

// void GUI::buttonCallback(unsigned long duration) {
//   if (duration > 0) {
//     pressed_flag = true;
//     switch (selected) {
//     case GUI_SELECTED_NONE:
//       selected = GUI_SELECTED_ROOT;
//       break;
//     case GUI_SELECTED_ROOT:
//       selected = GUI_SELECTED_SCALE;
//       break;
//     case GUI_SELECTED_SCALE:
//       selected = GUI_SELECTED_MIDICH;
//       break;
//     case GUI_SELECTED_MIDICH:
//       selected = GUI_SELECTED_NONE;
//       break;
//     }
//   }
// }

void gui_update(U8G2 *screen) { screen->sendBuffer(); }

void gui_clear(U8G2 *screen) { screen->clearBuffer(); }
bool gui_get_left_flag() { return left_flag; }
bool gui_get_right_flag() { return right_flag; }
bool gui_get_pressed_flag() { return pressed_flag; }
void gui_left_action() { left_flag = false; }
void gui_right_action() { right_flag = false; }
void gui_pressed_action() { pressed_flag = false; }
gui_selected_t gui_get_selected() { return selected; }

void gui_splash(U8G2 *screen) {
  screen->clearBuffer();
  screen->setFont(FONT_TITLE);
  screen->drawStr(35, 20, "TxalaMidi");
  screen->setFont(FONT_SMALL);
  screen->drawStr(40, 40, "Ver. 0.1");
  screen->sendBuffer();
}

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
    // screen->setDrawColor(0);
  } else {
    screen->drawStr(0, 44, scale_names[data->scale]);
  }
  // midi ch
  screen->setFont(FONT_SMALL);
  char midich[8];
  snprintf(midich, 8, "Ch: %d", data->channel + 1);
  if (selected == GUI_SELECTED_MIDICH) {
    screen->drawButtonUTF8(90, 12, U8G2_BTN_INV, 0, 2, 2, midich);
    // screen->setDrawColor(0);
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

// static char root_note_name[4];
// static gui_selected_t gui_selected;
// static bool right_flag = false;
// static bool left_flag = false;
// static bool pressed_flag = false;

// RotaryEncoder encoder(ENC_A, ENC_B, ENC_SW);

// void gui_encoder_callback(long value) {
//   // Don't do anything if either flag is set;
//   // it means we haven't taken action yet
//   if (right_flag || left_flag)
//     return;

//   // Set a flag that we can look for in the gpio task
//   // so that we know we have something to do
//   switch (value) {
//   case 1:
//     right_flag = true;
//     break;

//   case -1:
//     left_flag = true;
//     break;
//   }
//   // Override the tracked value back to 0 so that
//   // we can continue tracking right/left events
//   encoder.setEncoderValue(0);
// }

// void gui_button_callback(unsigned long duration) {
//   if (duration > 0) {
//     pressed_flag = true;
//     switch (gui_selected) {
//     case GUI_SELECTED_NONE:
//       gui_selected = GUI_SELECTED_ROOT;
//       break;
//     case GUI_SELECTED_ROOT:
//       gui_selected = GUI_SELECTED_SCALE;
//       break;
//     case GUI_SELECTED_SCALE:
//       gui_selected = GUI_SELECTED_MIDICH;
//       break;
//     case GUI_SELECTED_MIDICH:
//       gui_selected = GUI_SELECTED_NONE;
//       break;
//     }
//   }
// }

// bool gui_get_left_flag() { return left_flag; }

// bool gui_get_right_flag() { return right_flag; }

// bool gui_get_pressed_flag() { return pressed_flag; }

// gui_selected_t gui_get_selected() { return gui_selected; }

// void gui_left_action() { left_flag = false; }

// void gui_right_action() { right_flag = false; }

// void gui_pressed_action() { pressed_flag = false; }

// void gui_init(U8G2 *screen) {
//   gui_selected = GUI_SELECTED_NONE;
//   screen->begin();
//   encoder.setEncoderType(EncoderType::FLOATING);
//   encoder.setBoundaries(-1, 1, false);
//   encoder.onTurned(&gui_encoder_callback);
//   encoder.onPressed(&gui_button_callback);
//   encoder.begin();
// }

// void gui_update(U8G2 *screen) { screen->sendBuffer(); }
// void gui_clear(U8G2 *screen) { screen->clearBuffer(); }

// void gui_splash_screen(U8G2 *screen) {
//   screen->clearBuffer();
//   screen->setFont(FONT_TITLE);
//   screen->drawStr(35, 20, "TxalaMidi");
//   screen->setFont(FONT_SMALL);
//   screen->drawStr(40, 40, "Ver. 0.1");
//   screen->sendBuffer();
// }

// void gui_update_data(U8G2 *screen, midi_data_t *data) {
//   // root note
//   root_note_name[2] = ' ';
//   root_note_name[3] = 0;
//   snprintf(root_note_name, 4, "%s%d", midi_get_note_name(data->root_note),
//            midi_get_octave(data->root_note));
//   screen->setDrawColor(1);
//   screen->setFont(FONT_ROOTNOTE);
//   if (gui_selected == GUI_SELECTED_ROOT) {
//     screen->drawButtonUTF8(0, 25, U8G2_BTN_INV, 0, 2, 2, root_note_name);
//   } else {
//     screen->drawStr(0, 25, root_note_name);
//   }
//   // scale
//   screen->setFont(FONT_TITLE);
//   if (gui_selected == GUI_SELECTED_SCALE) {
//     screen->drawButtonUTF8(0, 44, U8G2_BTN_INV, 0, 2, 2,
//                            scale_names[data->scale]);
//     // screen->setDrawColor(0);
//   } else {
//     screen->drawStr(0, 44, scale_names[data->scale]);
//   }
//   // midi ch
//   screen->setFont(FONT_SMALL);
//   char midich[8];
//   snprintf(midich, 8, "Ch: %d", data->channel + 1);
//   if (gui_selected == GUI_SELECTED_MIDICH) {
//     screen->drawButtonUTF8(0, 63, U8G2_BTN_INV, 0, 2, 2, midich);
//     // screen->setDrawColor(0);
//   } else {
//     screen->drawStr(0, 63, midich);
//   }
// }
