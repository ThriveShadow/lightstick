#ifndef MODES_H
#define MODES_H

#include <FastLED.h>
#include <ESP8266WiFi.h>

class lightstick {
  public:
    void Solid(CRGB leds[], int numLeds, CRGB color);
    void Rainbow(CRGB leds[], int numLeds, int speed);
    void Blink(CRGB leds[], int numLeds, int speed, CRGB color);
    void ChaseUp(CRGB leds[], int numLeds, int speed, CRGB color);
};

#endif
