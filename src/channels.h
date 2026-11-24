#ifndef __CHANNELS_H
#define __CHANNELS_H

#include "txala.h"
#include <stddef.h>
#include <stdint.h>

// channel trends
typedef enum {
  CHANNEL_TREND_OFF,
  CHANNEL_TREND_STABLE,
  CHANNEL_TREND_UP,
  CHANNEL_TREND_DOWN,
  CHANNEL_TREND_UNDEFINED,
} channel_trend_t;

// channel actions
typedef enum {
  CHANNEL_ACTION_NOTHING,
  CHANNEL_ACTION_NOTEON,
  CHANNEL_ACTION_NOTEOFF,
  CHANNEL_ACTION_NOTEOFFON,
} channel_action_t;

// channel values
typedef struct {
  uint8_t last_values[4][NUM_CHANNELS]; // last values of each channels
  channel_trend_t trends[NUM_CHANNELS];
  channel_action_t actions[NUM_CHANNELS];
} channel_data_t;

#ifdef __cplusplus
extern "C" {
#endif

void channels_init_data(channel_data_t *data);
void channel_move_last_values(channel_data_t *data);
channel_trend_t channel_get_trend(channel_data_t *data, uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
