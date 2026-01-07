#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define BUTTON1_PIN 2
#define BUTTON2_PIN 3
#define BUTTON3_PIN 4 // not touch sensor, will be hidden for necessarry functions

#define NEO_PIN 6           // Define pin for right side LEDs
#define NEO_NUMPIXEL 32     // Number of LEDs per side
#define NEO_NUMPIXEL_PER 16 // Number of LEDs per side
Adafruit_NeoPixel strip(NEO_NUMPIXEL, NEO_PIN, NEO_GRB + NEO_KHZ800);

enum Orientation
{
  NORMAL,
  ROTATED_90,
  ROTATED_180,
  ROTATED_270
};
Orientation currentOrientation_R = NORMAL;
Orientation currentOrientation_L = NORMAL;

int brightness = 50;
int color[] = {255, 255, 255}; // Default color: white
bool persistentColor = false;

// Different operational modes for the cosplay mask
enum Mode
{
  OFF,
  ACTIVE,
  MANUAL,
  ERROR,
  Mode_COUNT
};
Mode currentMode = OFF;

// Various facial expressions that will be represented by LED patterns
enum Expression
{
  NEUTRAL,
  HAPPY,
  SAD,
  ANGRY,
  SHOCKED,
  Expression_COUNT
};
Expression currentExpression = NEUTRAL;
Expression nextExpression = NEUTRAL;
bool expressionPreSelection = false;

void setup()
{

  if (strip.begin())
  {
    Serial.println("NeoPixel strip initialized.");
  }
  else
  {
    Serial.println("Failed to initialize NeoPixel strip!");
  }

  strip.setBrightness(brightness);
  strip.show(); // Initialize all pixels to 'off'
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(BUTTON3_PIN, INPUT_PULLUP);
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
    setShockedExpression();
    break;
  default:
    setNeutralExpression();
    break;
  }
}
int errorPattern[NEO_NUMPIXEL_PER] = {
  255, 0, 0, 0,
  0, 255, 0, 0,
  0, 0, 255, 0,
  0, 0, 0, 255
};
int neutralIdlePattern[NEO_NUMPIXEL_PER] = {
  0, 63, 63, 0,
  63, 255, 255, 63,
  63, 255, 255, 63,
  0, 63, 63, 0
};
int neutralBlinkPattern[NEO_NUMPIXEL_PER] = {
  0, 0, 0, 0,
  255, 255, 255, 255,
  255, 255, 255, 255,
  0, 0, 0, 0
};
int happyPattern[NEO_NUMPIXEL_PER] = {
  63, 255, 255, 63,
  255, 63, 63, 255,
  0, 0, 0, 0,
  0, 0, 0, 0
};
int sadPattern[NEO_NUMPIXEL_PER] = {
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 63, 255, 255,
  255, 255, 255, 63
};
int angryPattern[NEO_NUMPIXEL_PER] = {
  255, 63, 0, 0,
  63, 255, 255, 63,
  0, 0, 63, 255,
  0, 0, 0, 0
};
int shockedPattern[NEO_NUMPIXEL_PER] = {
  0, 0, 0, 0,
  0, 255, 63, 0,
  0, 63, 63, 0,
  0, 0, 0, 0
};

const int neutralExpressionDuration = 5000;
const int blinkDuration = 500;
void setNeutralExpression()
{
  if (!persistentColor)
  {
    color[0] = 255;
    color[1] = 255;
    color[2] = 255;
  }

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
  setLedsFromPattern(currentPattern, currentOrientation_L, currentOrientation_R);
}

void setHappyExpression()
{
  if (!persistentColor)
  {
    color[0] = 255;
    color[1] = 255;
    color[2] = 0;
  }

  setLedsFromPattern(happyPattern, currentOrientation_L, currentOrientation_R);
}
void setSadExpression()
{
  if (!persistentColor)
  {
    color[0] = 0;
    color[1] = 50;
    color[2] = 255;
  }

  setLedsFromPattern_Left(sadPattern, currentOrientation_L);
  setLedsFromPattern_Right(getSymetricPattern(sadPattern), currentOrientation_R);
}
void setAngryExpression()
{
  if (!persistentColor)
  {
    color[0] = 255;
    color[1] = 0;
    color[2] = 0;
  }
  setLedsFromPattern_Left(angryPattern, currentOrientation_L);
  setLedsFromPattern_Right(getSymetricPattern(angryPattern), currentOrientation_R);
}
void setShockedExpression()
{
  if (!persistentColor)
  {
    color[0] = 255;
    color[1] = 255;
    color[2] = 255;
  }
  int currentTime = millis();
  if (currentTime - expressionTimer < 500)
  {
    for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
    {
      int brightnessValue = random(0, 256);
      uint32_t ledColor = getColorFromBrightness(color, brightnessValue);
      strip.setPixelColor(i, ledColor);
      strip.setPixelColor(i + NEO_NUMPIXEL_PER, ledColor);
    }
    strip.show();
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
    setLedsFromPattern(errorPattern, currentOrientation_L, currentOrientation_R);
  }
  else if (currentTime - expressionTimer < 500 + 500)
  {
    strip.clear();
    strip.show();
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
    strip.clear();
    strip.show();
    break;

  case ACTIVE:
    processExpressions();
    break;

  case MANUAL:
    // In manual mode, LEDs are controlled directly via web interface (not implemented here)
    break;
  case ERROR:
    setErrorExpression();
    break;
  default:
    strip.clear();
    strip.show();
    break;
  }
}

void setLedsFromPattern(int pattern[], Orientation currentOrientation_R, Orientation currentOrientation_L)

{
  int *correctedPattern = getCorrectedOrientation(pattern, currentOrientation_R);

  int adjustedColor = getColorFromBrightness(color, brightness);
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    int brightnessValue = correctedPattern[i];
    uint32_t ledColor = getColorFromBrightness(color, brightnessValue);
    strip.setPixelColor(i, ledColor);
  }
  correctedPattern = getCorrectedOrientation(pattern, currentOrientation_L);
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    int brightnessValue = correctedPattern[i];
    uint32_t ledColor = getColorFromBrightness(color, brightnessValue);
    strip.setPixelColor(i + NEO_NUMPIXEL_PER, ledColor);
  }

  strip.show();
}
void setLedsFromPattern_Left(int pattern[], Orientation currentOrientation_L = NORMAL)
{
  pattern = getCorrectedOrientation(pattern, currentOrientation_L);
  uint32_t adjustedColor = getColorFromBrightness(color, brightness);
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    int brightnessValue = pattern[i];
    uint32_t ledColor = getColorFromBrightness(color, brightnessValue);
    strip.setPixelColor(i, ledColor);
  }
  strip.show();
}
void setLedsFromPattern_Right(int pattern[], Orientation currentOrientation_R = NORMAL)
{
  pattern = getCorrectedOrientation(pattern, currentOrientation_R);
  uint32_t adjustedColor = getColorFromBrightness(color, brightness);
  for (size_t i = 0; i < NEO_NUMPIXEL_PER; i++)
  {
    int brightnessValue = pattern[i];
    uint32_t ledColor = getColorFromBrightness(color, brightnessValue);
    strip.setPixelColor(i + NEO_NUMPIXEL_PER, ledColor);
  }
  strip.show();
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

int *getSymetricPattern(int pattern[], Orientation orientation = NORMAL)
{
  pattern = getCorrectedOrientation(pattern, orientation);
  static int symetricPattern[NEO_NUMPIXEL_PER];
  for (int row = 0; row < 4; row++)
  {
    for (int col = 0; col < 4; col++)
    {
      // Mirror columns: 0->3, 1->2, 2->1, 3->0
      symetricPattern[row * 4 + col] = pattern[row * 4 + (3 - col)];
    }
  }
  return symetricPattern;
}

int *getCorrectedOrientation(int pattern[], Orientation orientation = NORMAL)
{
  static int correctedPattern[NEO_NUMPIXEL_PER];
  for (int row = 0; row < 4; row++)
  {
    for (int col = 0; col < 4; col++)
    {
      int newRow, newCol;
      switch (orientation)
      {
      case NORMAL:
        newRow = row;
        newCol = col;
        break;
      case ROTATED_90:
        newRow = col;
        newCol = 3 - row;
        break;
      case ROTATED_180:
        newRow = 3 - row;
        newCol = 3 - col;
        break;
      case ROTATED_270:
        newRow = 3 - col;
        newCol = row;
        break;
      }
      correctedPattern[newRow * 4 + newCol] = pattern[row * 4 + col];
    }
  }
  return correctedPattern;
}

uint32_t getColorFromBrightness(int baseColor[], int brightness)
{
  int r = (baseColor[0] * brightness) / 255;
  int g = (baseColor[1] * brightness) / 255;
  int b = (baseColor[2] * brightness) / 255;
  return strip.Color(r, g, b);
}

// --- BUTTON HANDLING ---

unsigned long button1PressTime = 0;
unsigned long button2PressTime = 0;
unsigned long button3PressTime = 0;
unsigned long button1LastEventTime = 0;
unsigned long button2LastEventTime = 0;
unsigned long button3LastEventTime = 0;
const unsigned long doubleTapWindow = 300;
const unsigned long holdThreshold = 1000;
const unsigned long holdDebounce = 200;
bool button1Held = false;
bool button2Held = false;
bool button3Held = false;

void handleButtons()
{
  handleButton1();
  handleButton2();
  handleButton3();
}

void handleButton1()
{
  unsigned long currentTime = millis();
  bool pressed = digitalRead(BUTTON1_PIN) == LOW;

  if (pressed && button1PressTime == 0)
  {
    button1PressTime = currentTime;
    button1Held = false;
  }
  else if (!pressed && button1PressTime != 0)
  {
    unsigned long pressDuration = currentTime - button1PressTime;

    if (pressDuration < holdThreshold)
    {
      if (currentTime - button1LastEventTime < doubleTapWindow)
      {
        onButton1DoubleTap();
        button1LastEventTime = 0;
      }
      else
      {
        onButton1Tap();
        button1LastEventTime = currentTime;
      }
    }
    button1PressTime = 0;
    button1Held = false;
  }
  else if (pressed && !button1Held && currentTime - button1PressTime >= holdThreshold)
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
    button2Held = false;
  }
  else if (!pressed && button2PressTime != 0)
  {
    unsigned long pressDuration = currentTime - button2PressTime;

    if (pressDuration < holdThreshold)
    {
      if (currentTime - button2LastEventTime < doubleTapWindow)
      {
        onButton2DoubleTap();
        button2LastEventTime = 0;
      }
      else
      {
        onButton2Tap();
        button2LastEventTime = currentTime;
      }
    }
    button2PressTime = 0;
    button2Held = false;
  }
  else if (pressed && !button2Held && currentTime - button2PressTime >= holdThreshold)
  {
    onButton2Hold();
    button2Held = true;
  }
}

void handleButton3()
{
  unsigned long currentTime = millis();
  bool pressed = digitalRead(BUTTON3_PIN) == LOW;

  if (pressed && button3PressTime == 0)
  {
    button3PressTime = currentTime;
    button3Held = false;
  }
  else if (!pressed && button3PressTime != 0)
  {
    unsigned long pressDuration = currentTime - button3PressTime;

    if (pressDuration < holdThreshold)
    {
      if (currentTime - button3LastEventTime < doubleTapWindow)
      {
        onButton3DoubleTap();
        button3LastEventTime = 0;
      }
      else
      {
        onButton3Tap();
        button3LastEventTime = currentTime;
      }
    }
    button3PressTime = 0;
    button3Held = false;
  }
  else if (pressed && !button3Held && currentTime - button3PressTime >= holdThreshold)
  {
    onButton3Hold();
    button3Held = true;
  }
}

// Button events
void onButton1Tap()
{
  if (expressionPreSelection)
  {
    selectExpression(nextExpression);
    expressionPreSelection = false;
  }
  else
  {
    cycleExpression();
  }
}
void onButton1DoubleTap()
{
}
void onButton1Hold()
{
  expressionPreSelectToggle();
}
void onButton2Tap()
{
  cycleExpression();
}
void onButton2DoubleTap()
{
}
void onButton2Hold()
{
  cycleMode();
}
void onButton3Tap()
{
}
void onButton3DoubleTap()
{
}
void onButton3Hold()
{
}

// helpers
void cycleMode()
{
  switch (currentMode)
  {
  case OFF:
    currentMode = ACTIVE;
    break;
  case ACTIVE:
    currentMode = MANUAL;
    break;
  case MANUAL:
    currentMode = OFF;
    break;
  default:
    currentMode = OFF;
    break;
  }
}

void cycleExpression()
{
  currentExpression = (Expression)((currentExpression + 1) % Expression_COUNT);
}

void cycleBrightness()
{
  brightness += 51;
  if (brightness > 255)
    brightness = 0;
}

void selectNextExpression()
{
  nextExpression = (Expression)((nextExpression + 1) % Expression_COUNT);
}

void selectExpression(Expression expr)
{
  nextExpression = expr;
}

void expressionPreSelectToggle()
{
  expressionPreSelection = !expressionPreSelection;
}

void togglePersistentColor()
{
  persistentColor = !persistentColor;
}

/*
0 1 2 3
4 5 6 7
8 9 10 11
12 13 14 15
*/
