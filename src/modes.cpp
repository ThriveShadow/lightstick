#include "modes.h"

void lightstick::Solid(CRGB leds[], int numLeds, CRGB color) {
  fill_solid(leds, numLeds, color);
    FastLED.show();
    FastLED.show();
}

void lightstick::Rainbow(CRGB leds[], int numLeds, int speed) {
  float hue = 0;
    int isRunning = 1;
    float deltaHue = speed;
    while (isRunning == 1)
    {
      fill_rainbow(leds, numLeds, hue, 5);
      FastLED.show();
      FastLED.show();
      hue = hue + (deltaHue/100);
      if (hue > 255)
      {
        hue = 0;
      }

      Serial.println("The code should've stuck here");

      int buttonState = digitalRead(D8);
      int prevButtonState;
      Serial.println(buttonState);
      Serial.println(prevButtonState);

      if ((WiFi.softAPgetStationNum() != 0) || ((buttonState == HIGH) && (prevButtonState == LOW)))
      {
        isRunning = 0;
      }

      prevButtonState = buttonState;
    }
}

void lightstick::Blink(CRGB leds[], int numLeds, int speed, CRGB color) {
  int isRunning = 1;
    while (isRunning == 1)
    {
      static unsigned long previousMillis = 0;
      static bool ledsOn = false;
      static int buttonState = LOW;
      static int prevButtonState = LOW;

      unsigned long interval = (101 - speed)*50;

      unsigned long currentMillis = millis();
      buttonState = digitalRead(D8);
      Serial.println(buttonState);

      if (currentMillis - previousMillis >= interval)
      {
        previousMillis = currentMillis;
        ledsOn = !ledsOn;

        if (ledsOn)
        {
          fill_solid(leds, numLeds, color);
        }
        else
        {
          fill_solid(leds, numLeds, CRGB::Black);
        }

        FastLED.show();
        FastLED.show();
      }

      buttonState = digitalRead(D8);
      if ((WiFi.softAPgetStationNum() != 0) || ((buttonState == HIGH) && (prevButtonState == LOW)))
      {
        isRunning = 0;
      }

      prevButtonState = buttonState;
    }
  }

void lightstick::ChaseUp(CRGB leds[], int numLeds, int speed, CRGB color) {
  int isRunning = 1;
  int ledPosition = 0;  // Initialize the position of the leading LED

  while (isRunning == 1) {
    static unsigned long previousMillis = 0;
    static int buttonState = LOW;
    static int prevButtonState = LOW;

    unsigned long interval = (101 - speed) * 50;

    unsigned long currentMillis = millis();
    buttonState = digitalRead(D8);
    Serial.println(buttonState);

    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Clear the entire LED strip
      fill_solid(leds, numLeds, CRGB::Black);

      // Turn on the leading LED
      leds[ledPosition] = color;

      // Calculate the position of the trailing LED
      int trailPosition = (ledPosition - 1 + numLeds) % numLeds;

      // Turn on the trailing LED
      leds[trailPosition] = color;

      FastLED.show();
      FastLED.show();

      // Move the leading LED position to the next location
      ledPosition = (ledPosition + 1) % numLeds;
    }

    buttonState = digitalRead(D8);
    if ((WiFi.softAPgetStationNum() != 0) || ((buttonState == HIGH) && (prevButtonState == LOW))) {
      isRunning = 0;
    }

    prevButtonState = buttonState;
  }
}
