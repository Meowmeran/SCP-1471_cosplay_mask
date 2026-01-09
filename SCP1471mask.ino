#include <Arduino.h>
#include "Adafruit_NeoPixel.h"

#define BUTTON1_PIN D5
#define BUTTON2_PIN D6
#define BUTTON3_PIN D7 // not touch sensor, will be hidden for necessarry functions

#define NEO_PIN D4          // Define pin for right side LEDs
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

unsigned expressionTimer = 0;
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
    0, 0, 0, 255};
int neutralIdlePattern[NEO_NUMPIXEL_PER] = {
    0, 63, 63, 0,
    63, 255, 255, 63,
    63, 255, 255, 63,
    0, 63, 63, 0};
int neutralBlinkPattern[NEO_NUMPIXEL_PER] = {
    0, 0, 0, 0,
    255, 255, 255, 255,
    255, 255, 255, 255,
    0, 0, 0, 0};
int happyPattern[NEO_NUMPIXEL_PER] = {
    63, 255, 255, 63,
    255, 63, 63, 255,
    0, 0, 0, 0,
    0, 0, 0, 0};
int sadPattern[NEO_NUMPIXEL_PER] = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 63, 255, 255,
    255, 255, 255, 63};
int angryPattern[NEO_NUMPIXEL_PER] = {
    255, 63, 0, 0,
    63, 255, 255, 63,
    0, 0, 63, 255,
    0, 0, 0, 0};
int shockedPattern[NEO_NUMPIXEL_PER] = {
    0, 0, 0, 0,
    0, 255, 63, 0,
    0, 63, 63, 0,
    0, 0, 0, 0};

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

  unsigned currentTime = millis();
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

struct ButtonHandler
{
  static const unsigned long DOUBLE_TAP_WINDOW = 300;
  static const unsigned long HOLD_THRESHOLD = 1000;
  static const unsigned long TAP_EXPIRY = 250;

  unsigned long pressTime = 0;
  unsigned long lastEventTime = 0;
  bool tapPending = false;
  bool isHeld = false;

  void update(bool pressed)
  {
    unsigned long currentTime = millis();

    if (pressed && pressTime == 0)
    {
      pressTime = currentTime;
      isHeld = false;
    }
    else if (!pressed && pressTime != 0)
    {
      unsigned long duration = currentTime - pressTime;

      if (duration < HOLD_THRESHOLD && !isHeld)
      {
        if (currentTime - lastEventTime < DOUBLE_TAP_WINDOW)
        {
          onDoubleTap();
          lastEventTime = 0;
        }
        else
        {
          onTap();
          tapPending = true;
          lastEventTime = currentTime;
        }
      }
      pressTime = 0;
      isHeld = false;
    }
    else if (pressed && !isHeld && currentTime - pressTime >= HOLD_THRESHOLD)
    {
      onHold();
      isHeld = true;
    }
  }

  bool consumeTap()
  {
    if (!tapPending)
      return false;
    if ((millis() - lastEventTime) <= TAP_EXPIRY)
    {
      tapPending = false;
      return true;
    }
    tapPending = false; // Expired
    return false;
  }

  bool isTapped() const { return tapPending; }
  bool isCurrentlyHeld() const { return isHeld; }

  virtual void onTap() {}
  virtual void onDoubleTap() {}
  virtual void onHold() {}
};

class Button1Handler : public ButtonHandler
{
  void onTap() override
  {
    // Reserved for future use
  }

  void onDoubleTap() override
  {
    // Reserved for future use
  }

  void onHold() override
  {
    // Reserved for combinations
  }
};

class Button2Handler : public ButtonHandler
{
  void onTap() override
  {
    // Reserved for future use
  }

  void onDoubleTap() override
  {
    // Reserved for future use
  }

  void onHold() override
  {
    // Reserved for combinations
  }
};

class Button3Handler : public ButtonHandler
{
  void onTap() override
  {
    // Reserved for future use
  }

  void onDoubleTap() override
  {
    // Reserved for future use
  }

  void onHold() override
  {
    // Reserved for future use
  }
};

Button1Handler button1;
Button2Handler button2;
Button3Handler button3;

struct ButtonCombinationHandler
{
  ButtonHandler &buttonA;
  ButtonHandler &buttonB;

  ButtonCombinationHandler(ButtonHandler &a, ButtonHandler &b) : buttonA(a), buttonB(b) {}

  void update()
  {
    if (buttonA.isCurrentlyHeld() && !buttonB.consumeTap())
    {
      on_A_Held();
    }
    if (buttonB.isCurrentlyHeld() && !buttonA.consumeTap())
    {
      on_B_Held();
    }
    if (buttonA.isCurrentlyHeld() && buttonB.isCurrentlyHeld())
    {
      onBothHeld();
    }
  }

  virtual void onBothHeld() {}
  virtual void on_A_Held() {}
  virtual void on_B_Held() {}
};

class Button1And2Handler : public ButtonCombinationHandler
{
public:
  Button1And2Handler(ButtonHandler &a, ButtonHandler &b) : ButtonCombinationHandler(a, b) {}

  void onBothHeld() override
  {
    togglePersistentColor();
  }
};

Button1And2Handler button1And2(button1, button2);

void handleButtons()
{
  button1.update(digitalRead(BUTTON1_PIN) == LOW);
  button2.update(digitalRead(BUTTON2_PIN) == LOW);
  button3.update(digitalRead(BUTTON3_PIN) == LOW);
  button1And2.update();
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
