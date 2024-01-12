#include "modes.h"

//for Wemos D1 Mini
// #define buttonPin D8

//for ESP-12
#define buttonPin 4

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

      int buttonState = digitalRead(buttonPin);
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
      buttonState = digitalRead(buttonPin);
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

      buttonState = digitalRead(buttonPin);
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
    buttonState = digitalRead(buttonPin);
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

    buttonState = digitalRead(buttonPin);
    if ((WiFi.softAPgetStationNum() != 0) || ((buttonState == HIGH) && (prevButtonState == LOW))) {
      isRunning = 0;
    }

    prevButtonState = buttonState;
  }
}

void lightstick::ChaseDown(CRGB leds[], int numLeds, int speed, CRGB color) {
  int isRunning = 1;
  int ledPosition = 0;  // Initialize the position of the leading LED

  while (isRunning == 1) {
    static unsigned long previousMillis = 0;
    static int buttonState = LOW;
    static int prevButtonState = LOW;

    unsigned long interval = (101 - speed) * 50;

    unsigned long currentMillis = millis();
    buttonState = digitalRead(buttonPin);
    Serial.println(buttonState);

    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Clear the entire LED strip
      fill_solid(leds, numLeds, CRGB::Black);

      // Turn on the leading LED
      leds[ledPosition] = color;

      // Calculate the position of the trailing LED
      int trailPosition = (ledPosition + 1 + numLeds) % numLeds;

      // Turn on the trailing LED
      leds[trailPosition] = color;

      FastLED.show();
      FastLED.show();

      // Move the leading LED position to the next location
      ledPosition = (ledPosition - 1) % numLeds;
    }

    buttonState = digitalRead(buttonPin);
    if ((WiFi.softAPgetStationNum() != 0) || ((buttonState == HIGH) && (prevButtonState == LOW))) {
      isRunning = 0;
    }

    prevButtonState = buttonState;
  }
}

void lightstick::Fade(CRGB leds[], int numLeds, int speed, CRGB color)
{
  int isRunning = 1;
  int fadeAmount = 3; // How much to fade the LED by
  CRGB currentColor = color; // Use CRGB for currentColor

  while (isRunning == 1)
  {
    static unsigned long previousMillis = 0;
    static int buttonState = LOW;
    static int prevButtonState = LOW;

    unsigned long interval = (101 - speed) * 50;
    unsigned long currentMillis = millis();
    buttonState = digitalRead(buttonPin);
    Serial.println(buttonState);

    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;

      for (int i = 0; i < numLeds; i++) {
        leds[i].fadeToBlackBy(fadeAmount);
      }

      // Set the LEDs to the current faded color
      FastLED.show();
      FastLED.show();

      // If the color is almost black, start fading back to the original color
      if (currentColor.r < fadeAmount && currentColor.g < fadeAmount && currentColor.b < fadeAmount) {
        fadeAmount = -fadeAmount; // Change fadeAmount direction to fade back to the original color
      }

      // If the color is almost the original color, start fading to black again
      if (currentColor.r == color.r && currentColor.g == color.g && currentColor.b == color.b) {
        fadeAmount = -fadeAmount; // Change fadeAmount direction to fade to black
      }

      // Apply the fade amount to the current color
      currentColor += CRGB(fadeAmount, fadeAmount, fadeAmount); // Add the fadeAmount to each channel of the color
      fill_solid(leds, numLeds, currentColor); // Set the LEDs to the new color
      FastLED.show();
      FastLED.show();
    }

    buttonState = digitalRead(buttonPin);
    if ((WiFi.softAPgetStationNum() != 0) || ((buttonState == HIGH) && (prevButtonState == LOW)))
    {
      isRunning = 0;
    }

    prevButtonState = buttonState;
  }
}
