#include "../patterns.h"

typedef struct data_struct
{
    HsiaColor color;
    float tail_length;
    int period;

} data_struct;

typedef struct frame_struct
{

} frame_struct;

static void data_destroyer(void *dataPtr)
{
    free(dataPtr);
}

static void *data_creator(uint16_t len, float intensity)
{
    data_struct *data = malloc(sizeof(data_struct));

    data->color = (HsiaColor){0, 0, 1, 1};
    data->period = randint_weighted_towards_min(3000, 20000, intensity);
    //data->intensity = intensity;

    return data;
}

static void *frame_creator(uint16_t len, uint32_t t, void *dataPtr)
{
    data_struct *data = dataPtr;
    frame_struct *frame = calloc(1, sizeof(frame_struct));

    // The "pointer" should fly back and forth all the time
    // The meteor should only be visible sometimes
    // It should never bounce
    // It should live for a certain amount of time
    // The tail should be a certain length
    // The head should slightly flicker
    // The tail should flicker quite a bit (fade out like with fire -- but try to avoid using sin. Maybe just make it index-based so that index is always slow/fast/medium/whatever)

    /*
    if (t > data->dies_at)
    {
        // Time to build a new one

        data->color = (HsiaColor) {0, 0, 1, 1};
        data->tail_length = randint_weighted_towards_min(len / 20.0, len / 4, data->intensity);
        data->spawn_at = t;
        data->dies_at = t + randint_weighted_towards_min(2000, 6000, data->intensity);
        data->wait = randint_weighted_towards_min(100, 5000, data->intensity);
    }
    */

    //float age = (t - data->spawn_at);
    //float ttl = (data->dies_at - data->spawn_at);
    //float p = age / ttl;

    //data->head_index =

    //float p = (t % data->period) / (float)data->period;
    //frame->head_index = len * InOutLinear(frame->p);

    return frame;
}

static inline void executor(uint16_t i, void *dataPtr, void *framePtr, Printer *printer)
{
    data_struct *data = dataPtr;
    frame_struct *frame = framePtr;

    HsiaColor c = {0, 1, 1, a};
    printer->print(i, &c, printer);

    /*
void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {
  setAll(0,0,0);

  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {


    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );
      }
    }

    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      }
    }

    showStrip();
    delay(SpeedDelay);
  }
  */
}

void pattern_register_meteor()
{
    pattern_register("meteor", executor, data_creator, data_destroyer, frame_creator, NULL, (PatternOptions){1});
}