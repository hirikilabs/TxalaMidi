#ifndef __GUI_H
#define __GUI_H

#include "midi.h"
#include "txala.h"
#include <U8g2lib.h>

typedef enum {
  GUI_SELECTED_NONE,
  GUI_SELECTED_ROOT,
  GUI_SELECTED_SCALE,
  GUI_SELECTED_MIDICH,
} gui_selected_t;

void gui_init(U8G2 *screen);
void gui_enc_isr();
void gui_button_isr();
bool gui_get_left_flag();
bool gui_get_right_flag();
bool gui_get_pressed_flag();
void gui_left_action();
void gui_right_action();
void gui_pressed_action();
gui_selected_t gui_get_selected();
void gui_update(U8G2 *screen);
void gui_clear(U8G2 *screen);
void gui_splash(U8G2 *screen);
void gui_show_data(U8G2 *screen, midi_data_t *data);
#endif
