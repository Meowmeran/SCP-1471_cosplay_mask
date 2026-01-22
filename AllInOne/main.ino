#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <cstdint>
#include <functional>

// ============================================
// FRAMEBUFFER DEFINITIONS
// ============================================

struct RGB
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct MaskFrame
{
  RGB left[4][4];
  RGB right[4][4];

  void clear()
  {
    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        left[y][x] = {0, 0, 0};
        right[y][x] = {0, 0, 0};
      }
    }
  }
};

static MaskFrame getErrorFrame()
{
  static uint32_t lastToggle = 0;
  static bool on = false;

  if (millis() - lastToggle > 1000)
  {
    lastToggle = millis();
    on = !on;
  }

  MaskFrame frame;
  frame.clear();

  if (on)
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      frame.left[i][i] = {255, 0, 0};
      frame.right[i][i] = {255, 0, 0};
    }
  }

  return frame;
}

// ============================================
// ORIENTATION HELPER
// ============================================

enum class Orientation
{
  NORMAL = 0,      // 0 degrees
  ROTATED_90 = 1,  // 90 degrees clockwise
  ROTATED_180 = 2, // 180 degrees
  ROTATED_270 = 3  // 270 degrees clockwise
};

class OrientationHelper
{
public:
  static int getPhysicalIndex(int logicalIndex, Orientation orientation)
  {
    int row = logicalIndex / 4;
    int col = logicalIndex % 4;
    return getPhysicalIndex(row, col, orientation);
  }

  static int getPhysicalIndex(int row, int col, Orientation orientation)
  {
    switch (orientation)
    {
    case Orientation::NORMAL:
      return row * 4 + col;
    case Orientation::ROTATED_90:
      return col * 4 + (3 - row);
    case Orientation::ROTATED_180:
      return (3 - row) * 4 + (3 - col);
    case Orientation::ROTATED_270:
      return (3 - col) * 4 + row;
    default:
      return row * 4 + col;
    }
  }

  static int getRotationAngle(Orientation orientation)
  {
    switch (orientation)
    {
    case Orientation::NORMAL:
      return 0;
    case Orientation::ROTATED_90:
      return 90;
    case Orientation::ROTATED_180:
      return 180;
    case Orientation::ROTATED_270:
      return 270;
    default:
      return 0;
    }
  }
};

// ============================================
// EXPRESSIONS
// ============================================

class Expressions
{
public:
  enum class Type
  {
    Neutral,
    Happy,
    Sad,
    Angry,
    Surprised,
    Lovely,
    Rainbow,
    Flashing,
    Dead,
    SIZE
  };

  static void render(Type type, MaskFrame &frame)
  {
    switch (type)
    {
    case Type::Neutral:
      renderNeutral(frame);
      break;
    case Type::Happy:
      renderHappy(frame);
      break;
    case Type::Sad:
      renderSad(frame);
      break;
    case Type::Angry:
      renderAngry(frame);
      break;
    case Type::Surprised:
      renderSurprised(frame);
      break;
    case Type::Lovely:
      renderLovely(frame);
      break;
    case Type::Rainbow:
      renderRainbow(frame);
      break;
    case Type::Flashing:
      renderFlashing(frame, 255, 255, 255, 200);
      break;
    case Type::Dead:
      renderDead(frame);
      break;
    default:
      renderNeutral(frame);
      break;
    }
  }

private:
  static void renderNeutral(MaskFrame &frame)
  {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;

    uint32_t currentTime = millis();
    uint32_t cycle = currentTime % 3000; // 3 second cycle
    bool isBlinking = cycle < 200;       // Blink for 200ms

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t pixelIndex = y * 4 + x;
        uint8_t intensity = 0;

        if (isBlinking)
        {
          if (pixelIndex == 8 || pixelIndex == 9 || pixelIndex == 10 || pixelIndex == 11)
          {
            intensity = 255;
          }
        }
        else
        {
          if (pixelIndex == 5 || pixelIndex == 6 || pixelIndex == 9 || pixelIndex == 10)
          {
            intensity = 255;
          }
        }

        frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
      }
    }
  }

  static void renderHappy(MaskFrame &frame)
  {
    uint8_t r = 255;
    uint8_t g = 115;
    uint8_t b = 155;

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t pixelIndex = y * 4 + x;
        uint8_t intensity = 0;

        if (pixelIndex == 5 || pixelIndex == 6 || pixelIndex == 8 || pixelIndex == 11)
        {
          intensity = 255;
        }
        else if (pixelIndex == 1 || pixelIndex == 2 || pixelIndex == 4 || pixelIndex == 7 || pixelIndex == 9 || pixelIndex == 10 || pixelIndex == 12 || pixelIndex == 13 || pixelIndex == 14 || pixelIndex == 15)
        {
          intensity = 0;
        }

        frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        frame.right[y][3 - x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
      }
    }
  }

  static void renderSad(MaskFrame &frame)
  {
    uint8_t r = 46;
    uint8_t g = 88;
    uint8_t b = 255;

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t pixelIndex = y * 4 + x;
        uint8_t intensity = 0;

        if (pixelIndex == 7 || pixelIndex == 9 || pixelIndex == 10 || pixelIndex == 11 || pixelIndex == 12)
        {
          intensity = 255;
        }
        else if (pixelIndex == 4 || pixelIndex == 6 || pixelIndex == 8 || pixelIndex == 13 || pixelIndex == 14 || pixelIndex == 15)
        {
          intensity = 0;
        }

        frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        frame.right[y][3 - x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
      }
    }
  }

  static void renderSurprised(MaskFrame &frame)
  {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t pixelIndex = y * 4 + x;
        uint8_t intensity = 0;

        if (pixelIndex == 5)
        {
          intensity = 255;
        }
        /*
        else if (pixelIndex == 9 - 4 || pixelIndex == 9 - 1 || pixelIndex == 9 + 1 || pixelIndex == 9 + 4)
        {
          intensity = 0;
        }*/

        frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        frame.right[y][3 - x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
      }
    }
  }

  static void renderAngry(MaskFrame &frame)
  {
    uint8_t r = 255;
    uint8_t g = 0;
    uint8_t b = 20;

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t pixelIndex = y * 4 + x;
        uint8_t intensity = 0;

        if (pixelIndex == 4 || pixelIndex == 5 || pixelIndex == 6 || pixelIndex == 7 || pixelIndex == 11)
        {
          intensity = 255;
        }
        else if (pixelIndex == 1 || pixelIndex == 3 || pixelIndex == 8 || pixelIndex == 10 || pixelIndex == 15)
        {
          intensity = 0;
        }

        frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        frame.right[y][3 - x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
      }
    }
  }

  static void renderLovely(MaskFrame &frame)
  {
    uint8_t r = 255;
    uint8_t g = 20;
    uint8_t b = 147;

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t pixelIndex = y * 4 + x;
        uint8_t intensity = 0;

        if (pixelIndex == 0 || pixelIndex == 3 || pixelIndex == 4 || pixelIndex == 5 ||
            pixelIndex == 6 || pixelIndex == 7 || pixelIndex == 8 || pixelIndex == 9 ||
            pixelIndex == 10 || pixelIndex == 11 || pixelIndex == 13 || pixelIndex == 14)
        {
          intensity = 255;
        }
        else if (pixelIndex == 1 || pixelIndex == 2)
        {
          intensity = 0;
        }
        else if (pixelIndex == 12 || pixelIndex == 15)
        {
          intensity = 0;
        }

        frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
      }
    }
  }

  static void renderRainbow(MaskFrame &frame)
  {
    uint32_t currentTime = millis();
    uint8_t offset = (currentTime / 50) % 16; // Shift every 50ms, cycle through 16 positions

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t pixelIndex = (y * 4 + x + offset) % 16;
        uint8_t r = (pixelIndex * 16) % 256;
        uint8_t g = ((pixelIndex * 16 + 85) % 256);
        uint8_t b = ((pixelIndex * 16 + 170) % 256);

        frame.left[y][x] = {r, g, b};
        frame.right[y][x] = {r, g, b};
      }
    }
  }
  static void renderFlashing(MaskFrame &frame, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint32_t interval = 500)
  {
    uint32_t currentTime = millis();
    bool isOn = (currentTime / interval) % 2 == 0;

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        if (isOn)
        {
          frame.left[y][x] = {r, g, b};
          frame.right[y][x] = {r, g, b};
        }
        else
        {
          frame.left[y][x] = {0, 0, 0};
          frame.right[y][x] = {0, 0, 0};
        }
      }
    }
  }
  static void renderDead(MaskFrame &frame)
  {
    uint8_t r = 255;
    uint8_t g = 0;
    uint8_t b = 0;

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t intensity = 0;

        // Create X pattern: diagonal from top-left to bottom-right and top-right to bottom-left
        if (x == y || x + y == 3)
        {
          intensity = 255;
        }

        frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
      }
    }
  }
};

// ============================================
// LED CONTROLLER
// ============================================

class LedController
{
public:
  LedController(uint8_t pin, uint16_t ledCount)
      : strip(ledCount, pin, NEO_GRB + NEO_KHZ800) {}

  void begin()
  {
    strip.begin();
  }

  void present(const MaskFrame &frame)
  {
    MaskFrame correctedFrame = getCorrectedFrame(frame);

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint16_t idx = mapLeft(x, y);
        const RGB &color = correctedFrame.left[y][x];
        strip.setPixelColor(idx, strip.Color(color.r, color.g, color.b));
      }
    }

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint16_t idx = mapRight(x, y);
        const RGB &color = correctedFrame.right[y][x];
        strip.setPixelColor(idx, strip.Color(color.r, color.g, color.b));
      }
    }

    strip.show();
  }

  MaskFrame getCorrectedFrame(const MaskFrame &frame) const
  {
    MaskFrame correctedFrame;

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        int physIdx = OrientationHelper::getPhysicalIndex(x, y, orientation_L);
        int physX = physIdx % 4;
        int physY = physIdx / 4;
        correctedFrame.left[physY][physX] = frame.left[y][x];
      }
    }

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        int physIdx = OrientationHelper::getPhysicalIndex(x, y, orientation_R);
        int physX = physIdx % 4;
        int physY = physIdx / 4;
        correctedFrame.right[physY][physX] = frame.right[y][x];
      }
    }

    return correctedFrame;
  }

  void setBrightness(uint8_t brightness)
  {
    strip.setBrightness(brightness);
  }

  uint8_t getBrightness() const
  {
    return strip.getBrightness();
  }

private:
  Adafruit_NeoPixel strip;
  Orientation orientation_L = Orientation::NORMAL;
  Orientation orientation_R = Orientation::NORMAL;

  // Convert row/col to zigzag wiring pattern index
  // Even rows (0, 2): left to right (0→1→2→3, 8→9→10→11)
  // Odd rows (1, 3): right to left (7→6→5→4, 15→14→13→12)
  static uint16_t linearToZigzag(uint8_t row, uint8_t col)
  {
    if (row % 2 == 0)
    {
      // Even rows go left to right
      return row * 4 + col;
    }
    else
    {
      // Odd rows go right to left
      return row * 4 + (3 - col);
    }
  }

  uint16_t mapLeft(uint8_t x, uint8_t y) const
  {
    // Apply orientation transformation first
    int physIdx = OrientationHelper::getPhysicalIndex(x, y, orientation_L);
    int physRow = physIdx / 4;
    int physCol = physIdx % 4;
    // Then apply zigzag mapping
    return linearToZigzag(physRow, physCol);
  }

  uint16_t mapRight(uint8_t x, uint8_t y) const
  {
    // Apply orientation transformation first
    int physIdx = OrientationHelper::getPhysicalIndex(x, y, orientation_R);
    int physRow = physIdx / 4;
    int physCol = physIdx % 4;
    // Then apply zigzag mapping, offset by 16 for right side
    return linearToZigzag(physRow, physCol) + 16;
  }
};

// ============================================
// MODE MANAGER
// ============================================

namespace Core
{
  enum class Mode
  {
    ACTIVE,
    MANUAL,
    OFF,
    ERROR
  };

  class ModeManager
  {
  public:
    ModeManager()
        : currentMode(Mode::OFF), lastMode(Mode::OFF)
    {
    }

    ~ModeManager()
    {
    }

    void setMode(Mode mode)
    {
      if (currentMode != Mode::OFF)
      {
        lastMode = currentMode;
      }
      currentMode = mode;

      // Safety check: if we are waking up to OFF, forced transition to ACTIVE
      if (currentMode == Mode::OFF && lastMode == Mode::OFF)
      {
        lastMode = Mode::ACTIVE;
      }
    }

    Mode getMode() const
    {
      return currentMode;
    }

    Mode getLastMode() const
    {
      return lastMode;
    }

    bool isActive() const
    {
      return currentMode == Mode::ACTIVE;
    }

    bool isManual() const
    {
      return currentMode == Mode::MANUAL;
    }

    bool isOff() const
    {
      return currentMode == Mode::OFF;
    }

    bool isError() const
    {
      return currentMode == Mode::ERROR;
    }

  private:
    Mode currentMode;
    Mode lastMode;
  };
}

// ============================================
// EXPRESSION MANAGER
// ============================================

namespace Core
{
  class ExpressionManager
  {
  private:
    uint32_t lastUpdateTime = 0;
    Expressions::Type currentExpression;
    Expressions::Type selectedExpression;
    MaskFrame *frame;

  public:
    ExpressionManager(MaskFrame *frame)
        : currentExpression(Expressions::Type::Neutral),
          selectedExpression(Expressions::Type::Neutral),
          frame(frame)
    {
      if (frame == nullptr)
      {
        // Handle null frame pointer
      }
    }

    void setExpression()
    {
      currentExpression = selectedExpression;
    }

    void setExpression(Expressions::Type type)
    {
      currentExpression = type;
    }

    Expressions::Type nextExpression()
    {
      int next = static_cast<int>(currentExpression) + 1;
      if (next >= static_cast<int>(Expressions::Type::SIZE))
        next = 0;

      currentExpression = static_cast<Expressions::Type>(next);
      return currentExpression;
    }

    Expressions::Type previousExpression()
    {
      int prev = static_cast<int>(currentExpression) - 1;
      if (prev < 0)
        prev = static_cast<int>(Expressions::Type::SIZE) - 1;

      currentExpression = static_cast<Expressions::Type>(prev);
      return currentExpression;
    }

    void selectExpression(Expressions::Type type)
    {
      selectedExpression = type;
    }

    Expressions::Type selectNextExpression()
    {
      int next = static_cast<int>(selectedExpression) + 1;
      if (next >= static_cast<int>(Expressions::Type::SIZE))
        next = 0;

      selectedExpression = static_cast<Expressions::Type>(next);
      return selectedExpression;
    }

    Expressions::Type selectPreviousExpression()
    {
      int prev = static_cast<int>(selectedExpression) - 1;
      if (prev < 0)
        prev = static_cast<int>(Expressions::Type::SIZE) - 1;

      selectedExpression = static_cast<Expressions::Type>(prev);
      return selectedExpression;
    }

    Expressions::Type getSelectedExpression() const { return selectedExpression; }
    Expressions::Type getCurrentExpression() const { return currentExpression; }

    void updateFrame()
    {
      if (frame != nullptr)
        Expressions::render(currentExpression, *frame);
    }
  };
}

// ============================================
// BUTTON HANDLER
// ============================================

#define MAX_BUTTONS 3
#define MAX_ACTIONS 12

class ButtonHandler
{
public:
  enum class ButtonEvent
  {
    Tap,
    DoubleTap,
    Hold,
    Release
  };

  using ActionCallback = void (*)();

  ButtonHandler(uint32_t doubleTapThreshold = 300, uint32_t holdThreshold = 500)
      : doubleTapThreshold(doubleTapThreshold), holdThreshold(holdThreshold), debounceDelay(50)
  {
    for (int i = 0; i < MAX_ACTIONS; i++)
    {
      actions[i].buttonPin = 0xFF;
      actions[i].callback = nullptr;
    }

    for (int i = 0; i < MAX_BUTTONS; i++)
    {
      buttons[i].pin = 0xFF;
    }
  }

  void begin(uint8_t pin1, uint8_t pin2, uint8_t pin3)
  {
    uint8_t pins[] = {pin1, pin2, pin3};

    for (int i = 0; i < MAX_BUTTONS; ++i)
    {
      buttons[i].pin = pins[i];
      pinMode(pins[i], INPUT_PULLUP);
    }
  }

  bool registerAction(uint8_t buttonPin, ButtonEvent event, ActionCallback action)
  {
    for (int i = 0; i < MAX_ACTIONS; ++i)
    {
      if (actions[i].buttonPin == 0xFF)
      {
        actions[i].buttonPin = buttonPin;
        actions[i].event = event;
        actions[i].callback = action;
        return true;
      }
    }
    return false;
  }

  void update(uint32_t deltaTime)
  {
    readButtons();

    for (int i = 0; i < MAX_BUTTONS; ++i)
    {
      ButtonState &state = buttons[i];

      if (state.pin == 0xFF)
        continue;

      state.lastChangeTime += deltaTime;

      if (state.isPressed)
      {
        state.pressTime += deltaTime;
        checkHold(state, deltaTime);
      }
      else
      {
        if (state.lastReleaseTime < doubleTapThreshold)
        {
          state.lastReleaseTime += deltaTime;
        }
      }
    }
  }

  bool isButtonHeld(uint8_t buttonPin) const
  {
    const ButtonState *state = findButton(buttonPin);
    if (!state)
      return false;
    return state->isPressed && state->pressTime >= holdThreshold;
  }

  uint8_t getHeldButtons() const
  {
    if (MAX_BUTTONS > 8)
    {
      Serial.println("getHeldButtons() supports up to 8 buttons only.");
      return 0;
    }

    uint8_t heldButtons = 0;
    for (uint8_t i = 0; i < MAX_BUTTONS; ++i)
    {
      const auto &state = buttons[i];
      if (state.isPressed && state.pressTime >= holdThreshold)
      {
        heldButtons |= (1 << i);
      }
    }
    return heldButtons;
  }

private:
  struct ButtonState
  {
    uint8_t pin = 0xFF;
    bool isPressed = false;
    bool lastPhysicalState = false;
    uint32_t pressTime = 0;
    uint32_t lastReleaseTime = 0;
    bool holdTriggered = false;
    uint32_t lastChangeTime = 0;
  };

  struct Action
  {
    uint8_t buttonPin = 0xFF;
    ButtonEvent event;
    ActionCallback callback = nullptr;
  };

  ButtonState buttons[MAX_BUTTONS] = {};
  Action actions[MAX_ACTIONS] = {};
  uint32_t doubleTapThreshold;
  uint32_t holdThreshold;
  uint32_t debounceDelay;

  ButtonState *findButton(uint8_t buttonPin)
  {
    for (int i = 0; i < MAX_BUTTONS; ++i)
    {
      if (buttons[i].pin == buttonPin)
      {
        return &buttons[i];
      }
    }
    return nullptr;
  }

  const ButtonState *findButton(uint8_t buttonPin) const
  {
    for (int i = 0; i < MAX_BUTTONS; ++i)
    {
      if (buttons[i].pin == buttonPin)
      {
        return &buttons[i];
      }
    }
    return nullptr;
  }

  void triggerActions(uint8_t buttonPin, ButtonEvent event)
  {
    for (int i = 0; i < MAX_ACTIONS; ++i)
    {
      if (actions[i].buttonPin == buttonPin && actions[i].event == event)
      {
        if (actions[i].callback)
        {
          actions[i].callback();
        }
      }
    }
  }

  void readButtons()
  {
    for (int i = 0; i < MAX_BUTTONS; ++i)
    {
      ButtonState &state = buttons[i];
      if (state.pin == 0xFF)
        continue;

      bool physicalState = (digitalRead(state.pin) == LOW);

      // 1. Detect physical movement to reset the debounce timer
      if (physicalState != state.lastPhysicalState)
      {
        state.lastPhysicalState = physicalState;
        state.lastChangeTime = 0;
      }

      // 2. Only proceed if the signal has been stable for 'debounceDelay' ms
      if (state.lastChangeTime >= debounceDelay)
      {
        // 3. Check if the stable state is different from our registered state
        if (physicalState != state.isPressed)
        {
          state.isPressed = physicalState;

          if (state.isPressed) // BUTTON PRESSED
          {
            state.pressTime = 0;
            state.holdTriggered = false;
            // Optional: triggerActions(state.pin, ButtonEvent::Press);
          }
          else // BUTTON RELEASED
          {
            if (!state.holdTriggered && state.pressTime < holdThreshold)
            {
              // It was a short press (Tap)
              triggerActions(state.pin, ButtonEvent::Tap);
              checkDoubleTap(state);
            }
            triggerActions(state.pin, ButtonEvent::Release);
            state.lastReleaseTime = 0;
          }
        }
      }
    }
  }

  void checkDoubleTap(ButtonState &state)
  {
    if (state.lastReleaseTime > 0 && state.lastReleaseTime < doubleTapThreshold)
    {
      triggerActions(state.pin, ButtonEvent::DoubleTap);
      state.lastReleaseTime = 0;
    }
    else
    {
      state.lastReleaseTime = 0;
    }
  }

  void checkHold(ButtonState &state, uint32_t deltaTime)
  {
    if (!state.holdTriggered && state.pressTime >= holdThreshold)
    {
      triggerActions(state.pin, ButtonEvent::Hold);
      state.holdTriggered = true;
    }
  }
};

// ============================================
// MAIN APPLICATION CODE
// ============================================

#define BUTTON1_PIN D1
#define BUTTON2_PIN D2
#define BUTTON3_PIN D7 // Not used currently

#define NEO_PIN D5          // Define pin for right side LEDs
#define NEO_NUMPIXEL 32     // Number of LEDs per side
#define NEO_NUMPIXEL_PER 16 // Number of LEDs per side

// Global instances
const unsigned long DOUBLE_TAP_TIME = 300;               // milliseconds
const unsigned long HOLD_TIME = 700;                     // milliseconds
ButtonHandler buttonHandler(DOUBLE_TAP_TIME, HOLD_TIME); // 300ms for double tap, 700ms for hold
MaskFrame frame = MaskFrame();
Core::ModeManager modeManager = Core::ModeManager();
Core::ExpressionManager expressionManager = Core::ExpressionManager(&frame);
LedController ledController = LedController(NEO_PIN, NEO_NUMPIXEL);

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
  Serial.begin(9600);
  Serial.println("System Initializing...");
  delay(3000); // Give serial time to stabilize
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
  ledController.setBrightness(255);

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

  if (!buttonHandler.isButtonHeld(BUTTON2_PIN))
  {
    wakeUp();
    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
      expressionManager.nextExpression();
      break;
    default:
      break;
    }
  }
  else // Button 2 is held
  {
    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
    case Core::Mode::MANUAL:
      cycleBrightness();
      break;
    default:
      break;
    }
  }
}

void onButton1DoubleTap()
{
  Serial.println("Button 1: Double Tap");
}

void onButton1Hold()
{
  Serial.println("Button 1: Hold");
  toggleOnOffMode();
}

void onButton2Tap()
{
  Serial.println("Button 2: Tap");
  if (!buttonHandler.isButtonHeld(BUTTON1_PIN))
  {
    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
      wakeUp();
      expressionManager.previousExpression();
      break;
    default:
      break;
    }
  }
  else // Button 1 is held
  {
    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
    case Core::Mode::MANUAL:
      break;
    default:
      break;
    }
  }
}

void onButton2DoubleTap()
{
  Serial.println("Button 2: Double Tap");
}

void onButton2Hold()
{
  Serial.println("Button 2: Hold");
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

// --- HELPER FUNCTIONS ---
void cycleMode()
{
  switch (modeManager.getMode())
  {
  case Core::Mode::OFF:
    wakeUp();
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

void wakeUp()
{
  if (modeManager.isOff())
  {
    modeManager.setMode(modeManager.getLastMode());
  }
}

void toggleOnOffMode()
{
  if (modeManager.isOff())
  {
    modeManager.setMode(modeManager.getLastMode());
  }
  else
  {
    modeManager.setMode(Core::Mode::OFF);
  }
}

void cycleBrightness()
{
  const uint8_t BRIGHTNESS_LEVELS[] = {1, 5, 10, 20, 40, 80, 160, 255};
  const uint8_t NUM_LEVELS = sizeof(BRIGHTNESS_LEVELS) / sizeof(BRIGHTNESS_LEVELS[0]);

  uint8_t currentBrightness = ledController.getBrightness();
  uint8_t nextIndex = 0;

  for (uint8_t i = 0; i < NUM_LEVELS; i++)
  {
    if (currentBrightness < BRIGHTNESS_LEVELS[i])
    {
      nextIndex = i;
      break;
    }
    nextIndex = (i + 1) % NUM_LEVELS;
  }

  ledController.setBrightness(BRIGHTNESS_LEVELS[nextIndex]);
}
