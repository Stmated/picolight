#include "../patterns.h"

// If larger size, more random falloff, but nobody would ever notice the repetitiveness.
#define RANDOM_FALLOFF_BUCKET_SIZE 20

// If larger size, more time until starting points repeat, but nobody would ever notice.
#define RANDOM_START_BUCKET_SIZE 10
// Use start bucket size as base, but offset, to create faked randomness (the will very rarely sync up)
#define RANDOM_END_BUCKET_SIZE (RANDOM_START_BUCKET_SIZE + 3)

typedef struct data_struct
{
  HsiaColor color;
  float tail_length;
  int period;
  float weights[RANDOM_FALLOFF_BUCKET_SIZE];

  // The starting point of a meteor.
  // It will be on this index that the meteor appears,
  int starts[RANDOM_START_BUCKET_SIZE];
  int ends[RANDOM_END_BUCKET_SIZE];

} data_struct;

typedef struct frame_struct
{
  float progress_raw;
  float progress;
  float head_index;
  int start_index;
  int end_index;
} frame_struct;

static void *data_creator(uint16_t len, float intensity)
{
  data_struct *data = malloc(sizeof(data_struct));

  data->color = (HsiaColor){0, 0, 1, 1};
  data->period = randint_weighted_towards_min(5000, 30000, intensity);
  data->tail_length = len / 2.0;

  for (int i = 0; i < RANDOM_FALLOFF_BUCKET_SIZE; i++)
  {
    data->weights[i] = randint(1000) / (float)1000.0;
  }

  // TODO: This should have some sanity checks, so we can make sure the meteor at least lives longer than X pixels.
  //       Right now the meteor could spawn at index 3 on a downward path. So we see it for only 3 pixels. Yikes.
  for (int i = 0; i < RANDOM_START_BUCKET_SIZE; i++)
  {
    data->starts[i] = randint(len);
  }

  // The end could possibly never be reached, since it can be before the start.
  // This is as intended, since the meteor should sometimes fly outside of bounds.
  for (int i = 0; i < RANDOM_END_BUCKET_SIZE; i++)
  {
    data->ends[i] = randint(len);
  }

  return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
  data_struct *data = dataPtr;
  frame_struct *frame = calloc(1, sizeof(frame_struct));

  frame->progress_raw = (t % data->period) / (float)data->period;
  frame->progress = InOutLinear(frame->progress_raw);

  // We fake the head index, to be able to go outside of bounds: [-tail..(len + tail)]
  frame->head_index = ((len + (data->tail_length * 2)) * frame->progress) - data->tail_length;

  const int era = floor(t / data->period);
  frame->start_index = data->starts[era % RANDOM_START_BUCKET_SIZE];
  frame->end_index = data->ends[era % RANDOM_END_BUCKET_SIZE];

  return frame;
}

static inline HsiaColor executor_lit(uint16_t i, void *dataPtr, void *framePtr, float distance)
{
  data_struct *data = dataPtr;

  // The falloff per distance should be different for each pixel index.
  // This way it will feel like the sparkle is falling of with a glitter.
  // The weight will be a [0..1] float, and should exponentially, but initially weakly, impact fading.
  float weight = data->weights[i % RANDOM_FALLOFF_BUCKET_SIZE];
  float weightedDistance = powf(distance, 1 + (0.05 * weight));
  float alpha = MAX(0, 1 - (weightedDistance / data->tail_length));
  if (distance > 1)
  {

    // Comet head should be slightly brighter than the tail. So weaken anything further than 1 pixel.
    alpha = alpha * 0.6;
  }

  HsiaColor c = {0, 0, 1, alpha};
  return c;
}

static inline HsiaColor executor(uint16_t i, void *dataPtr, void *framePtr)
{
  data_struct *data = dataPtr;
  frame_struct *frame = framePtr;

  float rawDistance = (i - frame->head_index);
  float distance = fabs(rawDistance);

  if (distance < data->tail_length)
  {
    if (frame->progress_raw < 0.5)
    {
      // We're going upwards
      if (rawDistance < 0 && i >= frame->start_index && i <= frame->end_index)
      {
        return executor_lit(i, dataPtr, framePtr, distance);
      }
      else
      {
        HsiaColor c = {0, 0, 0, 0};
        return c;
      }
    }
    else
    {
      // We're going downwards
      if (rawDistance > 0 && i <= frame->start_index && i >= frame->end_index)
      {
        return executor_lit(i, dataPtr, framePtr, distance);
      }
      else
      {
        HsiaColor c = {0, 0, 0, 0};
        return c;
      }
    }
  }
  else
  {
    HsiaColor c = {0, 0, 0, 0};
    return c;
  }
}

void pattern_register_meteor()
{
  pattern_register("meteor", executor, data_creator, NULL, frame_creator, NULL, (PatternOptions){1});
}