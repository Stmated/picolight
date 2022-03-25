/*
void pattern_snakes(uint16_t len, uint32_t t, void *data)
{
    for (uint16_t i = 0; i < len; i++)
    {
        uint32_t x = (t + (i * 100)) % 4000;

        if (x < 400)
            put_pixel(i, 0xff, 0, 0, 0);
        else if (x < 800)
            put_pixel(i, 0, 0xff, 0, 0);
        else if (x < 1200)
            put_pixel(i, 0, 0, 0xff, 0);
        else
            put_pixel(i, 0, 0, 0, 0);
    }
}
*/

/*
void pattern_snakes_smooth(uint16_t len, uint32_t t, void *data)
{
    for (uint16_t i = 0; i < len; i++)
    {
        float x = (t + (i * 100)) % 4000;

        // TODO: Good enough, or better to simplify by multiplying all the time?
        if (x < 100)
            put_pixel(i, (x / 100.0) * 0xff, 0, 0, 0);
        else if (x < 400)
            put_pixel(i, 0xff, 0, 0, 0);

        else if (x < 500)
            put_pixel(i, (1.0 - ((x - 400.0) / 100.0)) * 0xff, ((x - 400) / 100.0) * 0xff, 0, 0);
        else if (x < 800)
            put_pixel(i, 0, 0xff, 0, 0);

        else if (x < 900)
            put_pixel(i, 0, (1.0 - ((x - 800.0) / 100.0)) * 0xff, ((x - 800) / 100.0) * 0xff, 0);
        else if (x < 1100)
            put_pixel(i, 0, 0, 0xff, 0);

        else if (x < 1200)
            put_pixel(i, 0, 0, (1.0 - ((x - 1100.0) / 100.0)) * 0xff, 0);
        else
            put_pixel(i, 0, 0, 0, 0);
    }
}
*/


/*
uint8_t COLOR_R[4] = {0xFF, 0x00, 0x00, 0x00};
uint8_t COLOR_G[4] = {0xFF, 0x00, 0x00, 0x00};
uint8_t COLOR_B[4] = {0xFF, 0x00, 0x00, 0x00};
uint8_t COLOR_W[4] = {0xFF, 0xFF, 0xFF, 0xFF};

HsiColor COLORS_HSI_RGB[] = {
    {0, 1, 0.5},
    {120, 1, 0.5},
    {240, 1, 0.5}
};
*/

/*
HsiColor COLORS_HSI[] = {
    {0, 1, 0.5},
    {120, 1, 0.5},
    {240, 1, 0.5},
    {0, 1, 1},

    {0x00, 1, 0.5},
    {0xFF, 1, 0.5},
    {0xFF, 1, 0.5},
    {0xFF, 1, 0.5},
    {0xFF, 1, 0.5},
    {0xFF, 1, 0.5},
    {0x00, 1, 0.5},
    {0x00, 1, 0.5},
    {0x00, 1, 0.5},
    {0x00, 1, 0.5},
    {0x00, 1, 0.5},
};
*/

/*
void pattern_rgb_fade_between(uint len, uint t, HsiColor colors[])
{
    int fade_time_per_color = 4000;
    int fade_time_total = fade_time_per_color * color_count;
    int fade_time_transition = fade_time_per_color / 2;

    int boxed_color = t % fade_time_total;
    int boxed_per_color = t % fade_time_per_color;

    float color_percentage = (boxed_color / (float)fade_time_total);

    int color_index = floorf(color_count * color_percentage);

    bool is_fading_in = (boxed_per_color <= fade_time_transition);

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
    if (is_fading_in)
    {
        float percentage_faded_in = (boxed_per_color / (float)fade_time_transition);
        r = colors[color_index][0] * percentage_faded_in;
        g = colors[color_index][1] * percentage_faded_in;
        b = colors[color_index][2] * percentage_faded_in;
        w = colors[color_index][3] * percentage_faded_in;
    }
    else
    {
        float percentage_faded_out = (boxed_per_color - fade_time_transition) / (float)fade_time_transition;
        r = colors[color_index][0] - (colors[color_index][0] * percentage_faded_out);
        g = colors[color_index][1] - (colors[color_index][1] * percentage_faded_out);
        b = colors[color_index][2] - (colors[color_index][2] * percentage_faded_out);
        w = colors[color_index][3] - (colors[color_index][3] * percentage_faded_out);
    }

    for (int i = 0; i < len; i++)
    {
        put_pixel(i, r, g, b, w);
    }
}
*/

/*
int64_t previousStep = -1;
RgbColor previousRgb;
*/

void pattern_halloween_eyes(uint16_t len, uint32_t t, void *data)
{
    // red, blank, blank, red
    // fade in, stay, fade out, wait for quite a while
}

void pattern_cylon(uint16_t len, uint32_t t, void *data)
{
    /*
    void CylonBounce(byte red, byte green, byte blue, int EyeSize, int SpeedDelay, int ReturnDelay){

        for(int i = 0; i < NUM_LEDS-EyeSize-2; i++) {
            setAll(0,0,0);
            setPixel(i, red/10, green/10, blue/10);
            for(int j = 1; j <= EyeSize; j++) {
            setPixel(i+j, red, green, blue);
            }
            setPixel(i+EyeSize+1, red/10, green/10, blue/10);
            showStrip();
            delay(SpeedDelay);
        }

        delay(ReturnDelay);

        for(int i = NUM_LEDS-EyeSize-2; i > 0; i--) {
            setAll(0,0,0);
            setPixel(i, red/10, green/10, blue/10);
            for(int j = 1; j <= EyeSize; j++) {
            setPixel(i+j, red, green, blue);
            }
            setPixel(i+EyeSize+1, red/10, green/10, blue/10);
            showStrip();
            delay(SpeedDelay);
        }

        delay(ReturnDelay);
    }
    */
}


void pattern_fast_sparkle(uint16_t len, uint32_t t, void *data)
{
    // Sparkle(0xff, 0xff, 0xff, 0);
    /*
    int Pixel = random(NUM_LEDS);
  setPixel(Pixel,red,green,blue);
  showStrip();
  delay(SpeedDelay);
  setPixel(Pixel,0,0,0);
  */
}

void pattern_snow_sparkle(uint16_t len, uint32_t t, void *data)
{
    // SnowSparkle(0x10, 0x10, 0x10, 20, random(100,1000));
    /*
    setAll(red,green,blue);

  int Pixel = random(NUM_LEDS);
  setPixel(Pixel,0xff,0xff,0xff);
  showStrip();
  delay(SparkleDelay);
  setPixel(Pixel,red,green,blue);
  showStrip();
  delay(SpeedDelay);
  */
}

void pattern_running_lights(uint16_t len, uint32_t t, void *data)
{
    /*
    void RunningLights(byte red, byte green, byte blue, int WaveDelay) {
  int Position=0;

  for(int j=0; j<NUM_LEDS*2; j++)
  {
      Position++; // = 0; //Position + Rate;
      for(int i=0; i<NUM_LEDS; i++) {
        // sine wave, 3 offset waves make a rainbow!
        //float level = sin(i+Position) * 127 + 128;
        //setPixel(i,level,0,0);
        //float level = sin(i+Position) * 127 + 128;
        setPixel(i,((sin(i+Position) * 127 + 128)/255)*red,
                   ((sin(i+Position) * 127 + 128)/255)*green,
                   ((sin(i+Position) * 127 + 128)/255)*blue);
      }

      showStrip();
      delay(WaveDelay);
  }
}
*/
}

void pattern_rainbow_cycle(uint16_t len, uint32_t t, void *data)
{

    /*
void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< NUM_LEDS; i++) {
      c=Wheel(((i * 256 / NUM_LEDS) + j) & 255);
      setPixel(i, *c, *(c+1), *(c+2));
    }
    showStrip();
    delay(SpeedDelay);
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];

  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }

  return c;
}
*/
}

// void pattern_fire(uint len, uint t)
//{
/*
void loop() {
  Fire(55,120,15);
}

void Fire(int Cooling, int Sparking, int SpeedDelay) {
  static byte heat[NUM_LEDS];
  int cooldown;

  // Step 1.  Cool down every cell a little
  for( int i = 0; i < NUM_LEDS; i++) {
    cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);

    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < Sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
    //heat[y] = random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < NUM_LEDS; j++) {
    setPixelHeatColor(j, heat[j] );
  }

  showStrip();
  delay(SpeedDelay);
}

void setPixelHeatColor (int Pixel, byte temperature) {
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature/255.0)*191);

  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252

  // figure out which third of the spectrum we're in:
  if( t192 > 0x80) {                     // hottest
    setPixel(Pixel, 255, 255, heatramp);
  } else if( t192 > 0x40 ) {             // middle
    setPixel(Pixel, 255, heatramp, 0);
  } else {                               // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
}
*/
//}

void pattern_bouncing_balls(uint16_t len, uint32_t t, void *data)
{
    /*

void loop() {
  BouncingBalls(0xff,0,0, 3);
}

void BouncingBalls(byte red, byte green, byte blue, int BallCount) {
  float Gravity = -9.81;
  int StartHeight = 1;

  float Height[BallCount];
  float ImpactVelocityStart = sqrt( -2 * Gravity * StartHeight );
  float ImpactVelocity[BallCount];
  float TimeSinceLastBounce[BallCount];
  int   Position[BallCount];
  long  ClockTimeSinceLastBounce[BallCount];
  float Dampening[BallCount];

  for (int i = 0 ; i < BallCount ; i++) {
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0;
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i)/pow(BallCount,2);
  }

  while (true) {
    for (int i = 0 ; i < BallCount ; i++) {
      TimeSinceLastBounce[i] =  millis() - ClockTimeSinceLastBounce[i];
      Height[i] = 0.5 * Gravity * pow( TimeSinceLastBounce[i]/1000 , 2.0 ) + ImpactVelocity[i] * TimeSinceLastBounce[i]/1000;

      if ( Height[i] < 0 ) {
        Height[i] = 0;
        ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
        ClockTimeSinceLastBounce[i] = millis();

        if ( ImpactVelocity[i] < 0.01 ) {
          ImpactVelocity[i] = ImpactVelocityStart;
        }
      }
      Position[i] = round( Height[i] * (NUM_LEDS - 1) / StartHeight);
    }

    for (int i = 0 ; i < BallCount ; i++) {
      setPixel(Position[i],red,green,blue);
    }

    showStrip();
    setAll(0,0,0);
  }
}
*/
}

void pattern_meteor_rain(uint16_t len, uint32_t t, void *data)
{
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


// TODO: Looks like crap
/*
void pattern_rainbow_wave_spawn(uint16_t len, uint32_t t, void* data)
{
    double offset = (t / (double)1000);
    int offsetHue = 360.0 * offset;
    double dlen = (double)len;

    int hue = 0;
    double huePerLed = (360.0 / dlen);

    int topRowSize = LED_PER_ROW;
    int bottomRowSize = LED_PER_ROW * 4;

    uint8_t rgbw[4];
    for (int i = 0; i < len; i++)
    {
        int ledHue = abs(hue - offsetHue) % 360;
        float c_s = 1;
        float c_i = 0.5;

        if (i > (len - topRowSize))
        {
            ledHue = 0;
            c_s = 0;

            if ((rand() % 16) == 0)
            {
                c_i = 1;
            }
            else
            {
                c_i = 0;
            }
        }
        else if (i < bottomRowSize)
        {
            int horizontalIndex = (i % topRowSize);
            int time_offset_of_row = i * (2000 / LED_PER_ROW);
            int time_on_row = (t + time_offset_of_row);
            double percentage_into_animation = (time_on_row % 2000) / 2000.0;

            if (percentage_into_animation > 0.5)
            {
                // Flip it, since it's going the other direction
                percentage_into_animation = 1 - percentage_into_animation;
            }

            int blackUntil = (topRowSize * percentage_into_animation);
            int verticalIndex = (i / (double)topRowSize);

            if (verticalIndex <= blackUntil)
            {
                c_s = 0;
                c_i = 0;
            }
        }

        hsi2rgbw(ledHue, c_s, c_i, rgbw);
        put_pixel(i, rgbw[0], rgbw[1], rgbw[2], rgbw[3]);

        hue += huePerLed;
        if (hue > 360)
        {
            huePerLed = huePerLed * -1;
            hue += huePerLed;
        }
        else if (hue < 0)
        {
            huePerLed = huePerLed * -1;
            hue += huePerLed;
        }
    }
}
*/

// TODO: Improve, and ADD to list
void pattern_rainbow_wave_vertical(uint16_t len, uint32_t t, void *data)
{
    for (int i = 0; i < len; i++)
    {
        float percentage = 0.5 * (float)(1 + sin((i + (t / (double)6000))));

        uint8_t rgbw[4];
        hsi2rgbw(360.0 * percentage, 1, 0.5, rgbw);

        put_pixel(i, rgbw[0], rgbw[1], rgbw[2], 0x00);
    }
}

// TODO: Doesn't seem to work?
void pattern_heatmap(uint16_t len, uint32_t t, void *data)
{
    float offset1 = (t / (float)1000);
    float offset2 = (t / (float)500);

    for (int i = 0; i < len; i++)
    {
        float percentage = 0.5 * (float)(1 + (sin(0.61 * (i + offset1)) + sin(0.47 * (i - offset2))) / 2);

        HsvColor hsv;
        hsv.h = 360.0 * percentage;
        hsv.s = 1;
        hsv.v = 0.5;

        RgbColor rgb = HsvToRgb(hsv);

        put_pixel(i, rgb.r, rgb.g, rgb.b, 0x00);
    }
}

int getColumn(int i)
{
    return i % (int)LED_PER_ROW;
}

int getRow(int i)
{
    return floor(i / LED_PER_ROW);
}

// TODO: FIX! It's ugly
void pattern_fire(uint16_t len, uint32_t t, void *data)
{
    // Base white
    // Then increasingly from orange to red
    // Jitter the edges

    HsiColor white = {0, 1, 1};
    HsiColor yellow = {60, 1, 0.5};
    HsiColor red = {0, 1, 0.5};
    HsiColor black = {0, 0, 0};

    float percentageToYellow = 0.1; // + lerp_bounce(0.0, 0.1, (t % 4000) / 4000);
    float percentageToRed = 1;      // - lerp_bounce(0.1, 0.6, (t % 2000) / 2000);

    // TODO: Make "pixels" that move up, and then check distance from all the pixels each time.
    //          The closer to a pixel we are, the more color we should have.
    //          The further up we are, the redder it should be

    /*
    uint8_t pixels[len][2];

    int ledPerRow = LED_PER_ROW;
    for (int column = 0; column < ledPerRow; column++)
    {
    }

    uint8_t rgbw[4];
    for (int i = 0; i < len; i++)
    {
        hsi2rgbw(pixels[i][0], pixels[i][1], 0.75, rgbw);
        put_pixel(i, rgbw[0], rgbw[1], rgbw[2], rgbw[3]);
    }
    */

    /*
    float radiansPerX = (M_TWOPI / LED_PER_ROW);

    for (int i = 0; i < len; i++)
    {
        int row = getRow(i);
        int column = getColumn(i);

        // This should be done for each column, where the y-value is the height in rows for that column
        //float xPercentage = (column / LED_PER_ROW);
        float radians = column * radiansPerX;
        float lifetimeY = 0.5 * (1 + (sin(2 * (radians)) + sin(4.32 * radians)) / 2);

        // "cool" the colours the further up they get
    }
    */

    /*
    int aTenth = (len * 0.1);
    int aThird = (len * 0.75);

    for (int i = 0; i < len; i++)
    {
        HsvColor hsv;
        hsv.h = 0;
        hsv.s = 1;
        hsv.v = 0.5;

        float h_p = 0.5 * (float)(1 + (sin(0.61 * (float)(i + (t / (float)250))) + sin(0.47 * (float)(i - (t / (float)100)))) / 2);

        hsv.h = 60 - (60 * h_p);
        hsv.s = MIN(1, ((i / (double)(aTenth))));
        hsv.v = 1 - ((i / (double)len));

        RgbColor rgb = HsvToRgb(hsv);

        put_pixel(i, rgb.r, rgb.g, rgb.b, 0x00);
    }
    */
}

void pattern_red_bouncer(uint16_t len, uint32_t t, void *data)
{
    float percentage = InOutCubic((t % 2000) / 2000.0);
    uint32_t litLedIndex = roundf((len - 1) * percentage);

    for (uint32_t i = 0; i < len; ++i)
    {
        if (i == litLedIndex)
        {
            printer(i, 0xff, 0, 0, 0);
        }
        else
        {
            printer(i, 0x00, 0, 0, 0);
        }
    }

    sleep_ms(1);
}

void pattern_fade_pulse(uint16_t len, uint32_t t, void *data, printer printer)
{
    // auto y = sin((M_TWOPI) * ((t % 4000) / 4000));

    // pattern_rgb_fade_between(len, t, COLORS_RGB, 4);

    setAll(len, 0xff, 0xff, 0, 0, printer);
}