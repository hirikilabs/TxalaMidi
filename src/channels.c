#include "channels.h"

void channels_init_data(channel_data_t *data) {
  for (size_t ch = 0; ch < NUM_CHANNELS; ch++) {
    // trends
    data->trends[ch] = CHANNEL_TREND_OFF;
    // actions
    data->actions[ch] = CHANNEL_ACTION_NOTHING;
    // values
    for (size_t n = 0; n < 4; n++) {
      data->last_values[n][ch] =
          0; // or init it with some real data from the ADC???
    }
  }
}

void channel_move_last_values(channel_data_t *data) {
  // move all sensor values up a slot
  for (size_t ch = 0; ch < NUM_CHANNELS; ch++) {
    for (size_t n = 3; n > 0; n--) {
      data->last_values[n][ch] = data->last_values[n - 1][ch];
    }
  }
}

channel_trend_t channel_get_trend(channel_data_t *data, uint8_t channel) {
  int sum = 0;
  int mult_data = 0;
  int sum_index = 0;
  int sq_index = 0;
  int trend;

  // sum all values for a linear regression
  for (size_t n = 0; n < 4; n++) {
    sum += data->last_values[n][channel];
  }

  // calculate trends
  for (size_t n = 0; n < 4; n++) {
    mult_data += (n + 1) * data->last_values[n][channel];
    sum_index += (n + 1);
    sq_index += (n + 1) * (n + 1);
  }

  int numerator = (4 * mult_data) - (sum * sum_index);
  int denominator = (4 * sq_index) - (sum_index * sum_index);

  if (denominator != 0) {
    trend = numerator / denominator;
  } else {
    trend = 0;
  }

  // we have the trend, first check if channel is off
  // if last two values are below thresold, we are off
  if (data->last_values[0][channel] < CHANNEL_THRESHOLD &&
      data->last_values[1][channel] < CHANNEL_THRESHOLD) {
    return CHANNEL_TREND_OFF;
  }

  // else return trend (our array is last value first, so reverse the trend)
  if (trend < 0) {
    return CHANNEL_TREND_UP;
  } else if (trend > 0) {
    return CHANNEL_TREND_DOWN;
  } else if (trend == 0) {
    return (CHANNEL_TREND_STABLE);
  }

  return CHANNEL_TREND_UNDEFINED;
}
