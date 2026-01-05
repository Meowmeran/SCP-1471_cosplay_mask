#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define NEO_PIN_R 6 // Define pin for right side LEDs
#define NEO_PIN_L 7 // Define pin for left side LEDs
#define NUMPIXELS_R 16
#define NUMPIXELS_L 16

Adafruit_NeoPixel pixels_R(NUMPIXELS_R, NEO_PIN_R, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixels_L(NUMPIXELS_L, NEO_PIN_L, NEO_GRB + NEO_KHZ800);  

int brightness = 50;

// Different operational modes for the cosplay mask
enum Mode {
  OFF,
  ACTIVE,
  MANUAL,
  ERROR
};
Mode currentMode = OFF;

// Various facial expressions that will be represented by LED patterns
enum Expression {
  NEUTRAL,
  HAPPY,
  SAD,
  ANGRY
};
Expression currentExpression = NEUTRAL;


void setup() {
    
}


void loop() {
    
}
