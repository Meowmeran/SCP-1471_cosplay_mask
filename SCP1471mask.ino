#include <Arduino.h>
#include "Adafruit_NeoPixel.h"
#include "Input/ButtonHandler.h"
#include "Led/LedController.h"
#include "Led/Expressions.h"
#include "Math/Orientation.h"
#include "Core/ModeManager.h"
#include "Core/ExpressionManager.h"

#define BUTTON1_PIN D5
#define BUTTON2_PIN D6
#define BUTTON3_PIN D7 // not touch sensor, will be hidden for necessarry functions

#define NEO_PIN D4          // Define pin for right side LEDs
#define NEO_NUMPIXEL 32     // Number of LEDs per side
#define NEO_NUMPIXEL_PER 16 // Number of LEDs per side

// Global instances
ButtonHandler buttonHandler(300, 700); // 300ms for double tap, 700ms for hold
MaskFrame frame = MaskFrame();
Core::ModeManager modeManager = Core::ModeManager();
Core::ExpressionManager expressionManager = Core::ExpressionManager(&frame);
LedController ledController = LedController(NEO_PIN, NEO_NUMPIXEL * 2);

// Forward declarations
void onButton1Tap();
void onButton1DoubleTap();
void onButton1Hold();
void onButton2Tap();
void onButton2DoubleTap();
void onButton2Hold();
void onButton3Tap();
void onButton3DoubleTap();
void onButton3Hold();

void setup()
{
  Serial.begin(115200);

  // Initialize button handler with pins
  buttonHandler.begin(BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN);

  // Register button actions
  buttonHandler.registerAction(BUTTON1_PIN, ButtonHandler::ButtonEvent::Tap, onButton1Tap);
  buttonHandler.registerAction(BUTTON1_PIN, ButtonHandler::ButtonEvent::DoubleTap, onButton1DoubleTap);
  buttonHandler.registerAction(BUTTON1_PIN, ButtonHandler::ButtonEvent::Hold, onButton1Hold);
  buttonHandler.registerAction(BUTTON2_PIN, ButtonHandler::ButtonEvent::Tap, onButton2Tap);
  buttonHandler.registerAction(BUTTON2_PIN, ButtonHandler::ButtonEvent::DoubleTap, onButton2DoubleTap);
  buttonHandler.registerAction(BUTTON2_PIN, ButtonHandler::ButtonEvent::Hold, onButton2Hold);
  buttonHandler.registerAction(BUTTON3_PIN, ButtonHandler::ButtonEvent::Tap, onButton3Tap);
  buttonHandler.registerAction(BUTTON3_PIN, ButtonHandler::ButtonEvent::DoubleTap, onButton3DoubleTap);
  buttonHandler.registerAction(BUTTON3_PIN, ButtonHandler::ButtonEvent::Hold, onButton3Hold);

  ledController.begin();

  frame.clear();
}

unsigned long lastUpdate = 0;

void loop()
{
  unsigned long currentTime = millis();
  uint32_t deltaTime = currentTime - lastUpdate; // Time since last update
  lastUpdate = currentTime;

  switch (modeManager.getMode())
  {
  case Core::Mode::OFF:
    frame.clear();
    break;
  case Core::Mode::ACTIVE:
    expressionManager.updateFrame();
    break;
  case Core::Mode::MANUAL:
    // In manual mode, expression is controlled by button actions
    break;
  case Core::Mode::ERROR:
    ledController.present(getErrorFrame());
    break;
  default:
    expressionManager.setExpression(Expressions::Type::Neutral);
    break;
  }
  buttonHandler.update(deltaTime);
  ledController.present(frame);

  delay(10); // Small delay for stability
}

// --- BUTTON ACTION HANDLERS ---

void onButton1Tap()
{
  Serial.println("Button 1: Tap");
  cycleMode();
}

void onButton1DoubleTap()
{
  Serial.println("Button 1: Double Tap");
  // Add your double tap action here
}

void onButton1Hold()
{
  Serial.println("Button 1: Hold");
}

void onButton2Tap()
{
  Serial.println("Button 2: Tap");
  cycleBrightness();
}

void onButton2DoubleTap()
{
  Serial.println("Button 2: Double Tap");
  // Add your button 2 double tap action here
}
void onButton2Hold()
{
  Serial.println("Button 2: Hold");
  // Add your button 2 hold action here
}

void onButton3Tap()
{
  Serial.println("Button 3: Tap");
  // Add your button 3 action here
}
void onButton3DoubleTap()
{
  Serial.println("Button 3: Double Tap");
  // Add your button 3 double tap action here
}
void onButton3Hold()
{
  Serial.println("Button 3: Hold");
  // Add your button 3 hold action here
}

// --- HELPER FUNCTIONS ---
void cycleMode()
{
  switch (modeManager.getMode())
  {
  case Core::Mode::OFF:
    modeManager.setMode(Core::Mode::ACTIVE);
    break;
  case Core::Mode::ACTIVE:
    modeManager.setMode(Core::Mode::MANUAL);
    break;
  case Core::Mode::MANUAL:
    modeManager.setMode(Core::Mode::OFF);
    break;
  default:
    modeManager.setMode(Core::Mode::OFF);
    break;
  }
}

void cycleBrightness()
{
  uint8_t step = 51; // 20% of 255
  uint8_t brightness = ledController.getBrightness();
  if (brightness >= 255)
    ledController.setBrightness(step); // Reset to 20%
  else
    ledController.setBrightness(ledController.getBrightness() + step); // Increase brightness by 20%
}
