#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define BUTTON1_PIN 2
#define BUTTON2_PIN 3

#define NEO_PIN_R 6         // Define pin for right side LEDs
#define NEO_PIN_L 7         // Define pin for left side LEDs
#define NEO_NUMPIXEL_PER 16 // Number of LEDs per side
Adafruit_NeoPixel stripR(NEO_NUMPIXEL_PER, NEO_PIN_R, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripL(NEO_NUMPIXEL_PER, NEO_PIN_L, NEO_GRB + NEO_KHZ800);

int brightness = 50;
int color[] = {255, 255, 255}; // Default color: white

// Different operational modes for the cosplay mask
enum Mode
{
  OFF,
  ACTIVE,
  MANUAL,
  ERROR
};
Mode currentMode = OFF;

// Various facial expressions that will be represented by LED patterns
enum Expression
{
  NEUTRAL,
  HAPPY,
  SAD,
  ANGRY
};
Expression currentExpression = NEUTRAL;

void setup()
{

  stripR.begin();
  stripL.begin();
  stripR.setBrightness(brightness);
  stripL.setBrightness(brightness);
  stripR.show(); // Initialize all pixels to 'off'
  stripL.show(); // Initialize all pixels to 'off'
}

void loop()
{
  processLeds();
}

// --- EXPRESSION PROCESSING ---

int expressionTimer = 0;
void processExpressions()
{
  switch (currentExpression)
  {
  case NEUTRAL:
    setNeutralExpression();
    break;
  case HAPPY:
    setHappyExpression();
    break;
  case SAD:
    setSadExpression();
    break;
  case ANGRY:
    setAngryExpression();
    break;
  }
}
int neutralIdlePattern[NEO_NUMPIXEL_PER] = {0, 63, 63, 0,
                                            63, 255, 255, 63,
                                            63, 255, 255, 63,
                                            0, 63, 63, 0};
int neutralBlinkPattern[NEO_NUMPIXEL_PER] = {0, 0, 0, 0,
                                             255, 255, 255, 255,
                                             255, 255, 255, 255,
                                             0, 0, 0, 0};
int happyPattern[NEO_NUMPIXEL_PER] = {63, 255, 255, 63,
                                      255, 63, 63, 255,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0};
int sadPattern[NEO_NUMPIXEL_PER] = {0, 0, 0, 0,
                                    0, 0, 0, 0,
                                    0, 63, 255, 255,
                                    255, 255, 255, 63};
int angryPattern[NEO_NUMPIXEL_PER] = {255, 63, 0, 0,
                                      63, 255, 255, 63,
                                      0, 0, 63, 255,
                                      0, 0, 0, 0};

const int neutralExpressionDuration = 5000;
const int blinkDuration = 500;
void setNeutralExpression()
{
  int *adjustedColor = getColorFromBrightness(color, brightness);
  int currentTime = millis();
  int *currentPattern;
  if (currentTime - expressionTimer < neutralExpressionDuration)
  {
    currentPattern = neutralIdlePattern;
  }
  else if (currentTime - expressionTimer < neutralExpressionDuration + blinkDuration)
  {
    if (currentTime - expressionTimer < neutralExpressionDuration + blinkDuration / 2)
    {
      float t = float(currentTime - (expressionTimer + neutralExpressionDuration)) / (blinkDuration / 2);
      currentPattern = lerpBetweenPatterns(neutralIdlePattern, neutralBlinkPattern, t);
    }
    else
    {
      float t = float(currentTime - (expressionTimer + neutralExpressionDuration + blinkDuration / 2)) / (blinkDuration / 2);
      currentPattern = lerpBetweenPatterns(neutralBlinkPattern, neutralIdlePattern, t);
    }
  }
  else
  {
    expressionTimer = currentTime;
    currentPattern = neutralIdlePattern;
  }
  setLedsFromPattern(currentPattern);
}

void setHappyExpression()
{
  setLedsFromPattern(happyPattern);
}
void setSadExpression()
{
  setLedsFromPattern_Left(sadPattern);
  setLedsFromPattern_Right(getSymetricPattern(sadPattern));
}
void setAngryExpression()
{
  setLedsFromPattern_Left(angryPattern);
  setLedsFromPattern_Right(getSymetricPattern(angryPattern));
}

// --- LED PROCESSING ---


void processLeds()
{
  switch (currentMode)
  {
  case OFF: // Turn off all LEDs
    stripR.clear();
    stripL.clear();
    stripR.show();
    stripL.show();
    break;

  case ACTIVE:
    processExpressions();
    break;
  }
  
}

void setLedsFromPattern(int pattern[])
{
  int *adjustedColor = getColorFromBrightness(color, brightness);
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    int brightnessValue = pattern[i];
    int *ledColor = getColorFromBrightness(adjustedColor, brightnessValue);
    stripR.setPixelColor(i, ledColor[0], ledColor[1], ledColor[2]);
    stripL.setPixelColor(i, ledColor[0], ledColor[1], ledColor[2]);
  }
  stripR.show();
  stripL.show();
}
void setLedsFromPattern_Left(int pattern[])
{
  int *adjustedColor = getColorFromBrightness(color, brightness);
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    int brightnessValue = pattern[i];
    int *ledColor = getColorFromBrightness(adjustedColor, brightnessValue);
    stripL.setPixelColor(i, ledColor[0], ledColor[1], ledColor[2]);
  }
  stripL.show();
}
void setLedsFromPattern_Right(int pattern[])
{
  int *adjustedColor = getColorFromBrightness(color, brightness);
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    int brightnessValue = pattern[i];
    int *ledColor = getColorFromBrightness(adjustedColor, brightnessValue);
    stripR.setPixelColor(i, ledColor[0], ledColor[1], ledColor[2]);
  }
  stripR.show();
}

int *lerpBetweenPatterns(int patternA[], int patternB[], float t)
{
  static int lerpedPattern[NEO_NUMPIXEL_PER];
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    lerpedPattern[i] = (1 - t) * patternA[i] + t * patternB[i];
  }
  return lerpedPattern;
}
int *getSymetricPattern(int pattern[])
{
  static int symmetricPattern[NEO_NUMPIXEL_PER];
  for (size_t i = 0; i < NEO_NUMPIXEL_PER / 2; i++)
  {
    symmetricPattern[i] = pattern[i];
    symmetricPattern[NEO_NUMPIXEL_PER - 1 - i] = pattern[i];
  }
  return symmetricPattern;
}

int *getColorFromBrightness(int baseColor[], int brightness)
{
  static int adjustedColor[3];
  adjustedColor[0] = (baseColor[0] * brightness) / 255;
  adjustedColor[1] = (baseColor[1] * brightness) / 255;
  adjustedColor[2] = (baseColor[2] * brightness) / 255;
  return adjustedColor;
}

// --- BUTTON HANDLING ---



/*
0 1 2 3
4 5 6 7
8 9 10 11
12 13 14 15
*/
