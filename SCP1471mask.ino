#include <Arduino.h>
#include "Adafruit_NeoPixel.h"
#include "BMI160.h"

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
  ANGRY,
  SHOCKED
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
  try
  {
    pinMode(BUTTON1_PIN, INPUT_PULLUP);
    pinMode(BUTTON2_PIN, INPUT_PULLUP);
  }
  catch (const std::exception &e)
  {
    currentMode = ERROR;
    expressionTimer = millis();
  }
}

void loop()
{
  processLeds();
  handleButtons();
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
  case SHOCKED:
  }
}
int errorPattern[NEO_NUMPIXEL_PER] = {255, 0, 0, 0,
                                      0, 255, 0, 0,
                                      0, 0, 255, 0,
                                      0, 0, 0, 255};
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
int shockedPattern[NEO_NUMPIXEL_PER] = {0, 0, 0, 0,
                                       0, 255, 63, 0,
                                       0, 63, 63, 0,
                                       0, 0, 0, 0};

const int neutralExpressionDuration = 5000;
const int blinkDuration = 500;
void setNeutralExpression()
{
  color[0] = 255;
  color[1] = 255;
  color[2] = 255;
  uint32_t adjustedColor = getColorFromBrightness(color, brightness);
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
  color[0] = 255;
  color[1] = 255;
  color[2] = 0;
  setLedsFromPattern(happyPattern);
}
void setSadExpression()
{
  color[0] = 0;
  color[1] = 50;
  color[2] = 255;
  setLedsFromPattern_Left(sadPattern);
  setLedsFromPattern_Right(getSymetricPattern(sadPattern));
}
void setAngryExpression()
{
  color[0] = 255;
  color[1] = 0;
  color[2] = 0;
  setLedsFromPattern_Left(angryPattern);
  setLedsFromPattern_Right(getSymetricPattern(angryPattern));
}
void setShockedExpression()
{
  color[0] = 255;
  color[1] = 255;
  color[2] = 255;
  int currentTime = millis();
  if (currentTime - expressionTimer < 500)
  {
    for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
    {
      int brightnessValue = random(0, 256);
      uint32_t ledColor = getColorFromBrightness(color, brightnessValue);
      stripR.setPixelColor(i, ledColor);
      stripL.setPixelColor(i, ledColor);
    }
    stripR.show();
    stripL.show();
  }
  else
  {
    expressionTimer = currentTime;
  }
}
void setErrorExpression()
{
  int currentTime = millis();
  color[0] = 255;
  color[1] = 0;
  color[2] = 0;
  if (currentTime - expressionTimer < 500)
  {
    setLedsFromPattern(errorPattern);
  }
  else if (currentTime - expressionTimer < 500 + 500)
  {
    stripR.clear();
    stripL.clear();
    stripR.show();
    stripL.show();
  }
  else
  {
    expressionTimer = currentTime;
  }
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

  case MANUAL:
    // In manual mode, LEDs are controlled directly via web interface (not implemented here)
    break;
  case ERROR:
  }
}

void setLedsFromPattern(int pattern[])
{
  int adjustedColor = getColorFromBrightness(color, brightness);
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    int brightnessValue = pattern[i];
    uint32_t ledColor = getColorFromBrightness(color, brightnessValue);
    stripR.setPixelColor(i, ledColor);
    stripL.setPixelColor(i, ledColor);
  }
  stripR.show();
  stripL.show();
}
void setLedsFromPattern_Left(int pattern[])
{
  uint32_t adjustedColor = getColorFromBrightness(color, brightness);
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    int brightnessValue = pattern[i];
    uint32_t ledColor = getColorFromBrightness(color, brightnessValue);
    stripL.setPixelColor(i, ledColor);
  }
  stripL.show();
}
void setLedsFromPattern_Right(int pattern[])
{
  uint32_t adjustedColor = getColorFromBrightness(color, brightness);
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    int brightnessValue = pattern[i];
    uint32_t ledColor = getColorFromBrightness(color, brightnessValue);
    stripR.setPixelColor(i, ledColor);
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

uint32_t getColorFromBrightness(int baseColor[], int brightness)
{
  int r = (baseColor[0] * brightness) / 255;
  int g = (baseColor[1] * brightness) / 255;
  int b = (baseColor[2] * brightness) / 255;
  return stripR.Color(r, g, b);
}

// --- BUTTON HANDLING ---

unsigned long button1PressTime = 0;
unsigned long button2PressTime = 0;
unsigned long button1LastEventTime = 0;
unsigned long button2LastEventTime = 0;
const unsigned long doubleTapWindow = 300;
const unsigned long holdThreshold = 1000;
const unsigned long holdDebounce = 200;
bool button1Held = false;
bool button2Held = false;

void handleButtons()
{
  handleButton1();
  handleButton2();
}

void handleButton1()
{
  unsigned long currentTime = millis();
  bool pressed = digitalRead(BUTTON1_PIN) == LOW;

  if (pressed && button1PressTime == 0)
  {
    button1PressTime = currentTime;
  }
  else if (!pressed && button1PressTime != 0)
  {
    unsigned long pressDuration = currentTime - button1PressTime;

    if (pressDuration >= holdThreshold)
    {
      button1Held = false;
    }
    else if (currentTime - button1LastEventTime < doubleTapWindow)
    {
      onButton1DoubleTap();
      button1LastEventTime = 0;
    }
    else
    {
      button1LastEventTime = currentTime;
    }
    button1PressTime = 0;
  }
  else if (pressed && button1Held == false && currentTime - button1PressTime >= holdThreshold)
  {
    onButton1Hold();
    button1Held = true;
  }
}

void handleButton2()
{
  unsigned long currentTime = millis();
  bool pressed = digitalRead(BUTTON2_PIN) == LOW;

  if (pressed && button2PressTime == 0)
  {
    button2PressTime = currentTime;
  }
  else if (!pressed && button2PressTime != 0)
  {
    unsigned long pressDuration = currentTime - button2PressTime;

    if (pressDuration >= holdThreshold)
    {
      button2Held = false;
    }
    else if (currentTime - button2LastEventTime < doubleTapWindow)
    {
      onButton2DoubleTap();
      button2LastEventTime = 0;
    }
    else
    {
      button2LastEventTime = currentTime;
    }
    button2PressTime = 0;
  }
  else if (pressed && button2Held == false && currentTime - button2PressTime >= holdThreshold)
  {
    onButton2Hold();
    button2Held = true;
  }
}

void onButton1Tap() { currentMode = ACTIVE; }
void onButton1DoubleTap() { currentExpression = (Expression)((currentExpression + 1) % 4); }
void onButton1Hold() { currentMode = OFF; }

void onButton2Tap() { brightness = (brightness + 50) % 256; }
void onButton2DoubleTap()
{
  color[0] = 255;
  color[1] = 0;
  color[2] = 0;
}
void onButton2Hold() { brightness = 50; }

/*
0 1 2 3
4 5 6 7
8 9 10 11
12 13 14 15
*/
