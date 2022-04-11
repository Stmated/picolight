
// IDEAS:

// Sea Waves:
// y = ((((sin(x+a) + sin((x*2.2)+a) + sin((x*5)+a)) / 3)) + 1) / 2
// Sliding "a" makes it naturally bounce like a wave 
// Range: 0...1

// Random weird waves, with the amplification being random:
// Y = ((sin(x+a) + sin(x + (a*2.2)) + sin(x + (a*6)))) / 3

// Sliding same wave (will be used for fire)
// y = ((sin((x+a)) + sin((x+a)*2.2) + sin((x+a)*5)) / 3) * (1 - (x / LEN))
// The result of y should be multiplied by the distance from X=0
// Where LEN is the max of X (probably the number of LED lights available)

void pattern_halloween_eyes(uint16_t len, uint32_t t, void *data)
{
    // red, blank, blank, red
    // fade in, stay, fade out, wait for quite a while
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
    
}