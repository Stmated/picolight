#include "pattern.h"

typedef struct data_struct
{
  HsiaColor color_head;
  HsiaColor color_tail;
  uint32_t period;

} data_struct;

typedef struct frame_struct
{
  float progress;
  float start_index;
  float head_index;
  float tail_length;
  unsigned int seed;

} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
  data_struct *data = malloc(sizeof(data_struct));

  data->color_head = (HsiaColor){0, 0, 1, 1};
  data->color_tail = (HsiaColor){0, 0, 1, 1};
  data->period = randint_weighted_towards_min(2000, 10000, intensity);

  return data;
}

static void *frame_allocator(uint16_t len, void *dataPtr)
{
  return malloc(sizeof(frame_struct));
}

static void frame_creator(uint16_t len, uint32_t t, void *dataPtr, void *framePtr)
{
  data_struct *data = dataPtr;
  frame_struct *frame = framePtr;

  uint32_t era = (t / data->period);
  frame->progress = InLinear((t % data->period) / (float)data->period);

  // Set the new seed once every frame
  // NOTE: Makes it very important `rand_r` is called the same number of times for all pixels in the same order.
  frame->seed = era;
  float margin = (len * 0.10f);
  frame->start_index = margin + (rand_r(&frame->seed) / (RAND_MAX + 1.0f)) * (len - margin - margin);
  frame->tail_length = (len / 2.0f) + ((rand_r(&frame->seed) / (RAND_MAX + 1.0f)) * (len / 2.0f));

  // We fake the head index, to be able to go outside of bounds: [-tail..(len + tail)]
  frame->head_index = frame->start_index + (frame->progress * frame->tail_length);
}

static inline RgbwaColor executor(ExecutorArgs *args)
{
  data_struct *data = args->dataPtr;
  frame_struct *frame = args->framePtr;

  float distance = fabs(args->i - frame->head_index);

  int v_a = rand_r(&frame->seed);
  int v_r = rand_r(&frame->seed);
  int v_g = rand_r(&frame->seed);
  int v_b = rand_r(&frame->seed);

  float r_offset = 0.7f + ((v_r / (RAND_MAX + 1.0f)) * 0.3f);
  float g_offset = 0.7f + ((v_g / (RAND_MAX + 1.0f)) * 0.3f);
  float b_offset = 0.7f + ((v_b / (RAND_MAX + 1.0f)) * 0.3f);

  if (distance < 0.75)
  {
    float current = CHANNEL_MAX * (1 - distance) * (1 - frame->progress);
    return (RgbwaColor){current * r_offset, current * g_offset, current * b_offset, 0, RGB_ALPHA_MAX};
  }
  else if (frame->head_index > frame->start_index && args->i >= frame->start_index && args->i < frame->head_index)
  {
    float starting_alpha = (RGB_ALPHA_MAX * 0.10f) + ((v_a / (RAND_MAX + 1.0f)) * (RGB_ALPHA_MAX * 0.5f));
    float distance_multiplier = (1 - (distance / frame->tail_length));
    uint8_t current_alpha = starting_alpha * (1 - frame->progress) * distance_multiplier * distance_multiplier * distance_multiplier * distance_multiplier * distance_multiplier;

    return (RgbwaColor){current_alpha * r_offset, current_alpha * g_offset, current_alpha * b_offset, 0, RGB_ALPHA_MAX};
  }

  return *RGBWA_TRANSPARENT;
}

void pattern_register_meteor()
{
  pattern_register("meteor", executor, data_creator, NULL, frame_allocator, frame_creator, NULL, (PatternOptions){0.25});
}