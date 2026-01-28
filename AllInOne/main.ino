#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
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
    Wink,
    Shy,
    Lovely,
    NORMAL_EXPRESSION_END,
    Rainbow,
    Music,
    Flashing,
    Dead,
    Check,
    BigEyes,
    BinaryClock,
    Matrix,
    Loading,
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
    case Type::Wink:
      renderWink(frame);
      break;
    case Type::Shy:
      renderShy(frame);
      break;
    case Type::Lovely:
      renderLovely(frame);
      break;
    case Type::Rainbow:
      renderRainbow(frame);
      break;
    case Type::Music:
      renderMusic(frame);
      break;
    case Type::Flashing:
      renderFlashing(frame, 255, 255, 255, 200);
      break;
    case Type::Dead:
      renderDead(frame);
      break;
    case Type::Check:
      renderCheck(frame);
      break;
    case Type::BigEyes:
      renderBigEyes(frame);
      break;
    case Type::BinaryClock:
      renderBinaryClock(frame, 255, 255, 255, millis());
      break;
    case Type::Matrix:
      renderMatrix(frame);
      break;
    case Type::Loading:
      renderLoading(frame);
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

    static uint32_t nextBlinkTime = 0;
    static uint32_t nextLookTime = 0;
    static uint32_t blinkStartTime = 0;
    static uint32_t lookStartTime = 0;
    static int8_t lookDirection = 0;
    static uint16_t currentLookPeriod = 0;

    uint32_t currentTime = millis();

    uint16_t minBlinkTime = 3000;
    uint16_t maxBlinkTime = 8000;
    uint16_t blinkPeriod = 200;

    uint16_t minLookTime = 3000;
    uint16_t maxLookTime = 10000;
    uint16_t minLookPeriod = 200;
    uint16_t maxLookPeriod = 2300;

    // Initialize timers on first call
    if (nextBlinkTime == 0)
    {
      nextBlinkTime = currentTime + random(minBlinkTime, maxBlinkTime);
    }
    if (nextLookTime == 0)
    {
      nextLookTime = currentTime + random(minLookTime, maxLookTime);
    }

    // Check if we should start blinking
    bool isBlinking = false;
    if (currentTime >= nextBlinkTime && currentTime < nextBlinkTime + blinkPeriod)
    {
      isBlinking = true;
      if (blinkStartTime == 0)
      {
        blinkStartTime = currentTime;
      }
    }
    else if (currentTime >= nextBlinkTime + blinkPeriod)
    {
      blinkStartTime = 0;
      nextBlinkTime = currentTime + random(minBlinkTime, maxBlinkTime);
    }

    // Check if we should start looking
    bool isLooking = false;
    if (currentTime >= nextLookTime && currentTime < nextLookTime + currentLookPeriod)
    {
      isLooking = true;
      if (lookStartTime == 0)
      {
        lookStartTime = currentTime;
        lookDirection = random(0, 8);
      }
    }
    else if (currentTime >= nextLookTime + currentLookPeriod)
    {
      lookStartTime = 0;
      nextLookTime = currentTime + random(minLookTime, maxLookTime);
      currentLookPeriod = random(minLookPeriod, maxLookPeriod);
    }

    int8_t pixelShift = 0;
    if (isLooking && !isBlinking)
    {
      // lookDirection: 0 = right, 1 = left, 2 = down, 3 = up
      // Adjust to support 8 directions: 0-7
      // 0=right, 1=left, 2=down, 3=up, 4=right-down, 5=left-down, 6=right-up, 7=left-up

      switch (lookDirection)
      {
      case 0: // right
        pixelShift = 1;
        break;
      case 1: // left
        pixelShift = -1;
        break;
      case 2: // down
        pixelShift = 4;
        break;
      case 3: // up
        pixelShift = -4;
        break;
      case 4: // right-down
        pixelShift = 5;
        break;
      case 5: // left-down
        pixelShift = 3;
        break;
      case 6: // right-up
        pixelShift = -3;
        break;
      case 7: // left-up
        pixelShift = -5;
        break;
      default:
        pixelShift = 0;
        break;
      }
    }

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
          // Check boundaries before applying pixelShift
          int16_t shiftedPixel1 = 5 + pixelShift;
          int16_t shiftedPixel2 = 6 + pixelShift;
          int16_t shiftedPixel3 = 9 + pixelShift;
          int16_t shiftedPixel4 = 10 + pixelShift;

          if ((shiftedPixel1 >= 0 && shiftedPixel1 < 16 && pixelIndex == shiftedPixel1) ||
              (shiftedPixel2 >= 0 && shiftedPixel2 < 16 && pixelIndex == shiftedPixel2) ||
              (shiftedPixel3 >= 0 && shiftedPixel3 < 16 && pixelIndex == shiftedPixel3) ||
              (shiftedPixel4 >= 0 && shiftedPixel4 < 16 && pixelIndex == shiftedPixel4))
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

    uint32_t currentTime = millis();

    uint16_t minBlinkTime = 1000;
    uint16_t maxBlinkTime = 5000;
    uint16_t blinkPeriod = 200;
    uint16_t blinkingFrequency = 5000;
    uint32_t blinkingCycle = currentTime % blinkingFrequency;
    bool isBlinking = blinkingCycle < blinkPeriod;

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
          if (pixelIndex == 5 || pixelIndex == 6 || pixelIndex == 8 || pixelIndex == 11)
          {
            intensity = 255;
          }
          else if (pixelIndex == 1 || pixelIndex == 2 || pixelIndex == 4 || pixelIndex == 7 || pixelIndex == 9 || pixelIndex == 10 || pixelIndex == 12 || pixelIndex == 13 || pixelIndex == 14 || pixelIndex == 15)
          {
            intensity = 0;
          }
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

        if (pixelIndex == 4 - 4 || pixelIndex == 5 - 4 || pixelIndex == 6 || pixelIndex == 7 || pixelIndex == 11)
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

  static void renderWink(MaskFrame &frame)
  {
    uint8_t r = 255;
    uint8_t g = 215;
    uint8_t b = 0;

    static uint32_t lastWinkSwitch = 0;
    static uint32_t nextWinkTime = 0;
    static uint32_t winkDuration = 0;
    static bool winkLeft = true;
    static bool isWinking = false;
    static const uint32_t MIN_NORMAL_TIME = 500;        // Minimum time staring normally (3 seconds)
    static const uint32_t MAX_NORMAL_TIME = 1000;       // Maximum time staring normally (8 seconds)
    static const uint32_t MIN_WINK_DURATION = 2000;     // Minimum wink duration (200ms)
    static const uint32_t MAX_WINK_DURATION = 5000;     // Maximum wink duration (500ms)
    static const uint32_t WINK_SWITCH_INTERVAL = 10000; // Switch which eye winks every 10 seconds

    uint32_t currentTime = millis();

    // Initialize timers on first call
    if (nextWinkTime == 0)
    {
      nextWinkTime = currentTime + random(MIN_NORMAL_TIME, MAX_NORMAL_TIME);
    }

    // Switch which eye winks every 10 seconds
    if (currentTime - lastWinkSwitch > WINK_SWITCH_INTERVAL)
    {
      lastWinkSwitch = currentTime;
      winkLeft = !winkLeft;
    }

    // Check if we should start winking
    if (!isWinking && currentTime >= nextWinkTime)
    {
      isWinking = true;
      winkDuration = random(MIN_WINK_DURATION, MAX_WINK_DURATION);
      nextWinkTime = currentTime + winkDuration;
    }
    // Check if wink is over
    else if (isWinking && currentTime >= nextWinkTime)
    {
      isWinking = false;
      nextWinkTime = currentTime + random(MIN_NORMAL_TIME, MAX_NORMAL_TIME);
    }

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t pixelIndex = y * 4 + x;
        uint8_t intensity = 0;

        if (isWinking && winkLeft)
        {
          // Left eye winking: diagonal (0,5,10,15) and bottom row (12,13,14,15)
          if (pixelIndex == 0 || pixelIndex == 5 || pixelIndex == 10 ||
              pixelIndex == 12 || pixelIndex == 13 || pixelIndex == 14 || pixelIndex == 15)
          {
            intensity = 255;
          }
        }
        else
        {
          // Normal open eye
          if (pixelIndex == 1 + 4 || pixelIndex == 2 + 4 || pixelIndex == 4 || pixelIndex == 7 ||
              pixelIndex == 13 - 4 || pixelIndex == 14 - 4)
          {
            intensity = 255;
          }
        }

        // Apply to left eye
        frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};

        // Mirror to right eye with opposite wink state
        uint8_t intensityMirrored = 0;
        if (isWinking && !winkLeft)
        {
          // Right eye winking
          if (pixelIndex == 0 || pixelIndex == 5 || pixelIndex == 10 ||
              pixelIndex == 12 || pixelIndex == 13 || pixelIndex == 14 || pixelIndex == 15)
          {
            intensityMirrored = 255;
          }
        }
        else
        {
          // Right eye normal
          if (pixelIndex == 1 + 4 || pixelIndex == 2 + 4 || pixelIndex == 4 || pixelIndex == 7 ||
              pixelIndex == 13 - 4 || pixelIndex == 14 - 4)
          {
            intensityMirrored = 255;
          }
        }

        frame.right[y][3 - x] = {(uint8_t)(r * intensityMirrored / 255), (uint8_t)(g * intensityMirrored / 255), (uint8_t)(b * intensityMirrored / 255)};
      }
    }
  }

  static void renderShy(MaskFrame &frame)
  {
    uint8_t r = 255;
    uint8_t g = 80;
    uint8_t b = 180;

    static uint32_t nextBlinkTime = 0;
    static uint32_t nextLookTime = 0;
    static uint32_t blinkStartTime = 0;
    static uint32_t lookStartTime = 0;
    static int8_t lookDirection = 0;
    static uint16_t currentLookPeriod = 0;

    uint32_t currentTime = millis();

    uint16_t minBlinkTime = 2000;
    uint16_t maxBlinkTime = 6000;
    uint16_t blinkPeriod = 120;

    uint16_t minLookTime = 1000;
    uint16_t maxLookTime = 5000;
    uint16_t minLookPeriod = 200;
    uint16_t maxLookPeriod = 2300;

    // Initialize timers on first call
    if (nextBlinkTime == 0)
    {
      nextBlinkTime = currentTime + random(minBlinkTime, maxBlinkTime);
    }
    if (nextLookTime == 0)
    {
      nextLookTime = currentTime + random(minLookTime, maxLookTime);
    }

    // Check if we should start blinking
    bool isBlinking = false;
    if (currentTime >= nextBlinkTime && currentTime < nextBlinkTime + blinkPeriod)
    {
      isBlinking = true;
      if (blinkStartTime == 0)
      {
        blinkStartTime = currentTime;
      }
    }
    else if (currentTime >= nextBlinkTime + blinkPeriod)
    {
      blinkStartTime = 0;
      nextBlinkTime = currentTime + random(minBlinkTime, maxBlinkTime);
    }

    // Check if we should start looking
    bool isLooking = false;
    if (currentTime >= nextLookTime && currentTime < nextLookTime + currentLookPeriod)
    {
      isLooking = true;
      if (lookStartTime == 0)
      {
        lookStartTime = currentTime;
        lookDirection = random(0, 8);
      }
    }
    else if (currentTime >= nextLookTime + currentLookPeriod)
    {
      lookStartTime = 0;
      nextLookTime = currentTime + random(minLookTime, maxLookTime);
      currentLookPeriod = random(minLookPeriod, maxLookPeriod);
    }

    int8_t pixelShift = 0;
    if (isLooking && !isBlinking)
    {
      switch (lookDirection)
      {
      case 0: // right
        pixelShift = 1;
        break;
      case 1: // left
        pixelShift = -1;
        break;
      case 2: // down
        pixelShift = 4;
        break;
      case 3: // up
        pixelShift = -4;
        break;
      case 4: // right-down
        pixelShift = 5;
        break;
      case 5: // left-down
        pixelShift = 3;
        break;
      case 6: // right-up
        pixelShift = -3;
        break;
      case 7: // left-up
        pixelShift = -5;
        break;
      default:
        pixelShift = 0;
        break;
      }
    }

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
          // Check boundaries before applying pixelShift
          int16_t shiftedPixel1 = 5 + pixelShift;
          int16_t shiftedPixel2 = 6 + pixelShift;
          int16_t shiftedPixel3 = 9 + pixelShift;
          int16_t shiftedPixel4 = 10 + pixelShift;

          if ((shiftedPixel1 >= 0 && shiftedPixel1 < 16 && pixelIndex == shiftedPixel1) ||
              (shiftedPixel2 >= 0 && shiftedPixel2 < 16 && pixelIndex == shiftedPixel2) ||
              (shiftedPixel3 >= 0 && shiftedPixel3 < 16 && pixelIndex == shiftedPixel3) ||
              (shiftedPixel4 >= 0 && shiftedPixel4 < 16 && pixelIndex == shiftedPixel4))
          {
            intensity = 255;
          }
        }

        // Check if this pixel is part of the blush (bottom row)
        bool isBlush = (pixelIndex == 12 || pixelIndex == 13 || pixelIndex == 14 || pixelIndex == 15);

        uint8_t finalR, finalG, finalB;

        if (isBlush)
        {
          // Blush overrides eye - use blush color
          finalR = r;
          finalG = g;
          finalB = b;
        }
        else if (intensity > 0)
        {
          // Eye pixels should be white
          finalR = 255;
          finalG = 255;
          finalB = 255;
        }
        else
        {
          // No eye, no blush - off
          finalR = 0;
          finalG = 0;
          finalB = 0;
        }

        frame.left[y][x] = {finalR, finalG, finalB};
        frame.right[y][x] = {finalR, finalG, finalB};
      }
    }
  }

  static void renderLovely(MaskFrame &frame)
  {
    uint8_t r = 255;
    uint8_t g = 20;
    uint8_t b = 147;

    // Glitter color configuration - easy to modify
    static const uint8_t GLITTER_R = 240; // Red component of glitter
    static const uint8_t GLITTER_G = 0;   // Green component of glitter
    static const uint8_t GLITTER_B = 100; // Blue component of glitter

    static uint32_t lastGlitterTime = 0;
    static bool glitterPositions[32] = {false};          // Track glitter state for all pixels (16 left + 16 right)
    static const uint32_t GLITTER_UPDATE_INTERVAL = 100; // Update glitter every 100ms
    static const uint8_t GLITTER_CHANCE = 3;             // 30% chance per update (out of 10)

    uint32_t currentTime = millis();

    // Update glitter positions
    if (currentTime - lastGlitterTime > GLITTER_UPDATE_INTERVAL)
    {
      lastGlitterTime = currentTime;

      for (uint8_t i = 0; i < 32; i++)
      {
        glitterPositions[i] = (random(0, 10) < GLITTER_CHANCE);
      }
    }

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t pixelIndex = y * 4 + x;

        // Check if this pixel is part of the heart pattern
        bool isHeartPixel = (pixelIndex == 0 || pixelIndex == 3 || pixelIndex == 4 || pixelIndex == 5 ||
                             pixelIndex == 6 || pixelIndex == 7 || pixelIndex == 8 || pixelIndex == 9 ||
                             pixelIndex == 10 || pixelIndex == 11 || pixelIndex == 13 || pixelIndex == 14);

        if (isHeartPixel)
        {
          // Check glitter for left eye
          if (glitterPositions[pixelIndex])
          {
            frame.left[y][x] = {GLITTER_R, GLITTER_G, GLITTER_B};
          }
          else
          {
            frame.left[y][x] = {r, g, b};
          }

          // Check glitter for right eye
          if (glitterPositions[pixelIndex + 16])
          {
            frame.right[y][x] = {GLITTER_R, GLITTER_G, GLITTER_B};
          }
          else
          {
            frame.right[y][x] = {r, g, b};
          }
        }
        else
        {
          frame.left[y][x] = {0, 0, 0};
          frame.right[y][x] = {0, 0, 0};
        }
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
  static void renderMusic(MaskFrame &frame)
  {
    static uint32_t lastUpdateTime = 0;
    static uint8_t pillars[8] = {0};             // Height of each pillar (1-4)
    static const uint16_t UPDATE_INTERVAL = 100; // Update speed in milliseconds

    uint32_t currentTime = millis();

    // Initialize pillars on first call
    if (lastUpdateTime == 0)
    {
      for (uint8_t i = 0; i < 8; i++)
      {
        pillars[i] = random(1, 5); // Initialize to 1-4
      }
    }

    // Update pillar heights
    if (currentTime - lastUpdateTime > UPDATE_INTERVAL)
    {
      lastUpdateTime = currentTime;

      for (uint8_t i = 0; i < 8; i++)
      {
        // More varied random changes, never 0
        if (random(0, 10) < 7) // 70% chance to change
        {
          if (random(0, 10) < 3) // 30% chance to be 1 or 2
          {
            pillars[i] = random(1, 3); // 1 or 2
          }
          else
          {
            pillars[i] = random(1, 5); // 1-4
          }
        }
      }
    }

    // Define colors for each pillar
    const RGB pillarColors[8] = {
        {255, 0, 0},   // Red
        {255, 127, 0}, // Orange
        {255, 255, 0}, // Yellow
        {0, 255, 0},   // Green
        {0, 255, 255}, // Cyan
        {0, 0, 255},   // Blue
        {127, 0, 255}, // Purple
        {255, 0, 255}  // Magenta
    };

    // Render pillars
    for (uint8_t side = 0; side < 2; side++)
    {
      for (uint8_t col = 0; col < 4; col++)
      {
        uint8_t pillarIndex = side * 4 + col;
        uint8_t height = pillars[pillarIndex];
        RGB color = pillarColors[pillarIndex];

        for (uint8_t row = 0; row < 4; row++)
        {
          // Fill from bottom up based on height
          if ((3 - row) < height)
          {
            if (side == 0)
            {
              frame.left[row][col] = color;
            }
            else
            {
              frame.right[row][col] = color;
            }
          }
          else
          {
            if (side == 0)
            {
              frame.left[row][col] = {0, 0, 0};
            }
            else
            {
              frame.right[row][col] = {0, 0, 0};
            }
          }
        }
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
  static void renderCheck(MaskFrame &frame)
  {
    uint8_t r = 0;
    uint8_t g = 255;
    uint8_t b = 0;

    for (uint8_t y = 0; y < 4; y++)
    {
      for (uint8_t x = 0; x < 4; x++)
      {
        uint8_t pixelIndex = y * 4 + x;
        uint8_t intensity = 0;

        // Create check mark pattern
        if (pixelIndex == 0 || pixelIndex == 5 || pixelIndex == 10 || pixelIndex == 7)
        {
          intensity = 255;
        }

        frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
      }
    }
  }
  static void renderBigEyes(MaskFrame &frame)
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

        if (pixelIndex == 1 || pixelIndex == 2 || pixelIndex == 4 || pixelIndex == 7 ||
            pixelIndex == 8 || pixelIndex == 11 || pixelIndex == 13 || pixelIndex == 14)
        {
          intensity = 255;
        }

        frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
      }
    }
  }
  static void renderBinaryClock(MaskFrame &frame, uint8_t r, uint8_t g, uint8_t b, uint32_t currentTime)
  {
    // Get current time components (Note: This shows millis() time, not real clock time)
    // For real time, integrate RTC module
    unsigned long totalSeconds = currentTime / 1000;
    uint8_t hours = (totalSeconds / 3600) % 24;      // Hours (0-23)
    uint8_t minutes = (totalSeconds / 60) % 60;      // Minutes (0-59)
    uint8_t seconds = totalSeconds % 60;             // Seconds (0-59)
    uint8_t centiseconds = (currentTime / 100) % 10; // Centiseconds (0-9), updates every 100ms

    // Define colors for each time component
    uint8_t hourR = 255, hourG = 0, hourB = 0;      // Red for hours
    uint8_t minR = 0, minG = 255, minB = 0;         // Green for minutes
    uint8_t secR = 0, secG = 0, secB = 255;         // Blue for seconds
    uint8_t centiR = 255, centiG = 255, centiB = 0; // Yellow for centiseconds

    // Clear frame
    frame.clear();

    // Render hours (5 bits) on left eye, columns 0-1
    for (uint8_t bit = 0; bit < 5; bit++)
    {
      if (hours & (1 << bit))
      {
        uint8_t col = (bit < 4) ? 1 : 0; // Column 1 for bits 0-3, column 0 for bit 4
        uint8_t row = 3 - (bit % 4);     // Bottom to top (row 3 = LSB, row 0 = bit 3)
        frame.left[row][col] = {hourR, hourG, hourB};
      }
    }

    // Render minutes (6 bits) on left eye, columns 2-3
    for (uint8_t bit = 0; bit < 6; bit++)
    {
      if (minutes & (1 << bit))
      {
        uint8_t col = (bit < 4) ? 3 : 2; // Column 3 for bits 0-3, column 2 for bits 4-5
        uint8_t row = 3 - (bit % 4);     // Bottom to top
        frame.left[row][col] = {minR, minG, minB};
      }
    }

    // Render seconds (6 bits) on right eye, columns 0-1
    for (uint8_t bit = 0; bit < 6; bit++)
    {
      if (seconds & (1 << bit))
      {
        uint8_t col = (bit < 4) ? 1 : 0; // Column 1 for bits 0-3, column 0 for bits 4-5
        uint8_t row = 3 - (bit % 4);     // Bottom to top
        frame.right[row][col] = {secR, secG, secB};
      }
    }

    // Render centiseconds (4 bits) on right eye, columns 2-3
    for (uint8_t bit = 0; bit < 4; bit++)
    {
      if (centiseconds & (1 << bit))
      {
        uint8_t col = 2 + (bit / 4); // Column 2 for bits 0-3, column 3 would be for bit 4 (not used)
        uint8_t row = 3 - (bit % 4); // Bottom to top
        frame.right[row][col] = {centiR, centiG, centiB};
      }
    }
  }
  static void renderMatrix(MaskFrame &frame)
  {
    // Configuration variables
    static const uint16_t UPDATE_INTERVAL = 75;      // Speed: milliseconds between updates (lower = faster)
    static const uint8_t DROP_FREQUENCY = 4;         // Frequency: 0-10 chance per update to start new drop
    static const uint8_t MIN_TRAIL_LENGTH = 2;       // Minimum trail length
    static const uint8_t MAX_TRAIL_LENGTH = 8;       // Maximum trail length
    static const uint8_t HEAD_BRIGHTNESS = 255;      // Brightness of drop head (0-255)
    static const uint8_t TRAIL_BRIGHTNESS_RATIO = 2; // Trail fade divisor (higher = dimmer trail)

    static uint32_t lastUpdateTime = 0;
    static uint8_t dropPositions[8] = {0}; // Track drop position for each column
    static uint8_t dropLengths[8] = {0};   // Track length of each drop
    static bool dropActive[8] = {false};   // Track if drop is active

    uint32_t currentTime = millis();

    // Initialize drops randomly
    if (lastUpdateTime == 0)
    {
      lastUpdateTime = currentTime;
      for (uint8_t i = 0; i < 8; i++)
      {
        dropActive[i] = random(0, 2);
        dropPositions[i] = random(0, 8);
        dropLengths[i] = random(MIN_TRAIL_LENGTH, MAX_TRAIL_LENGTH + 1);
      }
    }

    // Update drops based on UPDATE_INTERVAL
    if (currentTime - lastUpdateTime > UPDATE_INTERVAL)
    {
      lastUpdateTime = currentTime;

      for (uint8_t i = 0; i < 8; i++)
      {
        if (dropActive[i])
        {
          dropPositions[i]++;
          if (dropPositions[i] > 8 + dropLengths[i])
          {
            dropActive[i] = false;
          }
        }
        else if (random(0, 10) < DROP_FREQUENCY)
        {
          dropActive[i] = true;
          dropPositions[i] = 0;
          dropLengths[i] = random(MIN_TRAIL_LENGTH, MAX_TRAIL_LENGTH + 1);
        }
      }
    }

    // Render the matrix effect
    for (uint8_t side = 0; side < 2; side++)
    {
      for (uint8_t col = 0; col < 4; col++)
      {
        uint8_t dropIndex = side * 4 + col;

        if (dropActive[dropIndex])
        {
          for (uint8_t row = 0; row < 4; row++)
          {
            int8_t distanceFromHead = (int8_t)dropPositions[dropIndex] - (int8_t)row;
            uint8_t intensity = 0;

            if (distanceFromHead >= 0 && distanceFromHead <= dropLengths[dropIndex])
            {
              if (distanceFromHead == 0)
              {
                // Brightest at head
                intensity = HEAD_BRIGHTNESS;
              }
              else
              {
                // Dimmer trail with configurable fade
                intensity = HEAD_BRIGHTNESS / (distanceFromHead * TRAIL_BRIGHTNESS_RATIO);
              }
            }

            if (side == 0)
            {
              frame.left[row][col] = {0, intensity, 0};
            }
            else
            {
              frame.right[row][col] = {0, intensity, 0};
            }
          }
        }
        else
        {
          for (uint8_t row = 0; row < 4; row++)
          {
            if (side == 0)
            {
              frame.left[row][col] = {0, 0, 0};
            }
            else
            {
              frame.right[row][col] = {0, 0, 0};
            }
          }
        }
      }
    }
  }
  static void renderLoading(MaskFrame &frame)
  {
    static uint32_t lastUpdateTime = 0;
    static uint8_t position = 0;
    static const uint16_t UPDATE_INTERVAL = 75;
    static const uint8_t TRAIL_LENGTH = 4;
    static const uint8_t EDGE_PATH_LENGTH = 12;

    uint32_t currentTime = millis();

    if (currentTime - lastUpdateTime > UPDATE_INTERVAL)
    {
      lastUpdateTime = currentTime;
      position = (position + 1) % EDGE_PATH_LENGTH;
    }

    // Define the edge path (clockwise around the perimeter) - 12 unique positions
    const uint8_t edgePath[12][2] = {
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, // Top edge
        {1, 3},
        {2, 3},
        {3, 3}, // Right edge
        {3, 2},
        {3, 1},
        {3, 0}, // Bottom edge
        {2, 0},
        {1, 0} // Left edge
    };

    frame.clear();

    // Render the loading pixel and trail
    for (uint8_t i = 0; i < TRAIL_LENGTH; i++)
    {
      uint8_t trailPos = (position + EDGE_PATH_LENGTH - i) % EDGE_PATH_LENGTH;

      uint8_t row = edgePath[trailPos][0];
      uint8_t col = edgePath[trailPos][1];

      uint8_t intensity = 255 - (i * 255 / TRAIL_LENGTH);
      uint8_t r = (255 * intensity) / 255;
      uint8_t g = (140 * intensity) / 255;
      uint8_t b = 0;

      frame.left[row][col] = {r, g, b};
      frame.right[row][col] = {r, g, b};
    }
  }
};

// ============================================
// WEB SERVER & CAPTIVE PORTAL MANAGER
// ============================================

class WebServerManager
{
private:
  ESP8266WebServer server;
  DNSServer dnsServer;
  bool isInitialized = false;
  static const uint16_t DNS_PORT = 53;
  static const uint16_t HTTP_PORT = 80;
  static const char *AP_SSID;
  static const char *AP_PASSWORD;
  static const char *MDNS_NAME;

  // Callback to be invoked when an expression is selected
  std::function<void(Expressions::Type)> expressionCallback;

public:
  WebServerManager() : server(HTTP_PORT) {}

  void begin(std::function<void(Expressions::Type)> onExpressionChange)
  {
    if (isInitialized)
      return;

    expressionCallback = onExpressionChange;

    // Configure Access Point
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    IPAddress apIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, gateway, subnet);

    // Start mDNS
    if (!MDNS.begin(MDNS_NAME))
    {
      Serial.println("[WiFi] mDNS responder failed to start");
    }
    else
    {
      Serial.println("[WiFi] mDNS responder started: mask.local");
      MDNS.addService("http", "tcp", 80);
    }

    // Start DNS server (Captive Portal)
    dnsServer.start(DNS_PORT, "*", apIP);
    Serial.println("[WiFi] Captive Portal DNS server started");

    // Setup web server routes
    setupRoutes();

    server.begin();
    Serial.println("[WiFi] Web server started on http://mask.local or http://192.168.4.1");
    Serial.print("[WiFi] Connect to AP: ");
    Serial.println(AP_SSID);

    isInitialized = true;
  }

  void update()
  {
    if (!isInitialized)
      return;

    dnsServer.processNextRequest();
    server.handleClient();
    MDNS.update();
  }

  bool isRunning() const { return isInitialized; }

private:
  void setupRoutes()
  {
    // Root path - serve captive portal HTML
    server.on("/", HTTP_GET, [this]()
              { server.send(200, "text/html", getHtmlInterface()); });

    // API endpoint to set expression
    server.on("/api/expression", HTTP_POST, [this]()
              {
      if (!server.hasArg("type"))
      {
        server.send(400, "application/json", "{\"error\":\"Missing 'type' parameter\"}");
        return;
      }

      String typeStr = server.arg("type");
      Expressions::Type expressionType = parseExpressionType(typeStr);

      if (expressionCallback)
      {
        expressionCallback(expressionType);
      }

      server.send(200, "application/json", "{\"status\":\"ok\"}"); });

    // API endpoint to get current status
    server.on("/api/status", HTTP_GET, [this]()
              { server.send(200, "application/json", "{\"status\":\"connected\"}"); });

    // Catch-all for captive portal (redirect to root)
    server.onNotFound([this]()
                      {
      server.sendHeader("Location", "http://mask.local/", true);
      server.send(302, "text/plain", ""); });
  }

  Expressions::Type parseExpressionType(const String &typeStr)
  {
    if (typeStr == "neutral")
      return Expressions::Type::Neutral;
    if (typeStr == "happy")
      return Expressions::Type::Happy;
    if (typeStr == "sad")
      return Expressions::Type::Sad;
    if (typeStr == "angry")
      return Expressions::Type::Angry;
    if (typeStr == "surprised")
      return Expressions::Type::Surprised;
    if (typeStr == "wink")
      return Expressions::Type::Wink;
    if (typeStr == "shy")
      return Expressions::Type::Shy;
    if (typeStr == "lovely")
      return Expressions::Type::Lovely;
    if (typeStr == "rainbow")
      return Expressions::Type::Rainbow;
    if (typeStr == "music")
      return Expressions::Type::Music;
    if (typeStr == "flashing")
      return Expressions::Type::Flashing;
    if (typeStr == "dead")
      return Expressions::Type::Dead;
    if (typeStr == "check")
      return Expressions::Type::Check;
    if (typeStr == "bigeyes")
      return Expressions::Type::BigEyes;
    if (typeStr == "binaryclock")
      return Expressions::Type::BinaryClock;
    if (typeStr == "matrix")
      return Expressions::Type::Matrix;
    if (typeStr == "loading")
      return Expressions::Type::Loading;

    return Expressions::Type::Neutral;
  }

  String getHtmlInterface()
  {
    return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SCP-1471 Mask Control</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            padding: 10px;
        }

        .container {
            background: rgba(255, 255, 255, 0.95);
            border-radius: 20px;
            padding: 30px;
            box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
            max-width: 500px;
            width: 100%;
        }

        .header {
            text-align: center;
            margin-bottom: 30px;
        }

        .header h1 {
            color: #333;
            font-size: 28px;
            margin-bottom: 10px;
        }

        .header p {
            color: #666;
            font-size: 14px;
        }

        .expression-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 10px;
            margin-bottom: 20px;
        }

        .btn {
            padding: 15px;
            border: none;
            border-radius: 10px;
            font-size: 14px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);
        }

        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 6px 20px rgba(0, 0, 0, 0.3);
        }

        .btn:active {
            transform: translateY(0);
        }

        .btn-neutral {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }

        .btn-happy {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            color: white;
        }

        .btn-sad {
            background: linear-gradient(135deg, #4facfe 0%, #00f2fe 100%);
            color: white;
        }

        .btn-angry {
            background: linear-gradient(135deg, #fa709a 0%, #fee140 100%);
            color: white;
        }

        .btn-surprised {
            background: linear-gradient(135deg, #30cfd0 0%, #330867 100%);
            color: white;
        }

        .btn-wink {
            background: linear-gradient(135deg, #a8edea 0%, #fed6e3 100%);
            color: #333;
        }

        .btn-shy {
            background: linear-gradient(135deg, #ff9a56 0%, #ff6a88 100%);
            color: white;
        }

        .btn-lovely {
            background: linear-gradient(135deg, #ff6b9d 0%, #c06c84 100%);
            color: white;
        }

        .expression-divider {
            text-align: center;
            margin: 20px 0;
            color: #999;
            font-size: 12px;
            text-transform: uppercase;
            letter-spacing: 2px;
            position: relative;
        }

        .expression-divider:before {
            content: '';
            position: absolute;
            left: 0;
            top: 50%;
            width: 100%;
            height: 1px;
            background: #ddd;
            z-index: 1;
        }

        .expression-divider span {
            background: white;
            position: relative;
            z-index: 2;
            padding: 0 10px;
        }

        .btn-rainbow {
            background: linear-gradient(90deg, red, yellow, lime, cyan, blue, magenta, red);
            color: white;
        }

        .btn-music {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }

        .btn-flashing {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            color: white;
            animation: pulse 1s infinite;
        }

        @keyframes pulse {
            0%, 100% {
                opacity: 1;
            }
            50% {
                opacity: 0.7;
            }
        }

        .btn-dead {
            background: linear-gradient(135deg, #2c3e50 0%, #34495e 100%);
            color: white;
        }

        .btn-check {
            background: linear-gradient(135deg, #11998e 0%, #38ef7d 100%);
            color: white;
        }

        .btn-bigeyes {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }

        .btn-binaryclock {
            background: linear-gradient(135deg, #000 0%, #434343 100%);
            color: #0f0;
            font-family: 'Courier New', monospace;
        }

        .btn-matrix {
            background: linear-gradient(135deg, #000 0%, #1a1a1a 100%);
            color: #0f0;
            font-family: 'Courier New', monospace;
        }

        .btn-loading {
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }

        .status {
            text-align: center;
            padding: 10px;
            border-radius: 8px;
            background: #f0f0f0;
            color: #333;
            font-size: 12px;
            margin-top: 20px;
        }

        .status.connected {
            background: #d4edda;
            color: #155724;
        }

        .status.loading {
            background: #fff3cd;
            color: #856404;
        }

        @media (max-width: 480px) {
            .container {
                padding: 20px;
                border-radius: 15px;
            }

            .header h1 {
                font-size: 24px;
            }

            .btn {
                padding: 12px;
                font-size: 13px;
            }

            .expression-grid {
                gap: 8px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>😀 SCP-1471</h1>
            <p>Mask Expression Control</p>
        </div>

        <div class="expression-grid">
            <button class="btn btn-neutral" onclick="setExpression('neutral')">Neutral</button>
            <button class="btn btn-happy" onclick="setExpression('happy')">Happy</button>
            <button class="btn btn-sad" onclick="setExpression('sad')">Sad</button>
            <button class="btn btn-angry" onclick="setExpression('angry')">Angry</button>
            <button class="btn btn-surprised" onclick="setExpression('surprised')">Surprised</button>
            <button class="btn btn-wink" onclick="setExpression('wink')">Wink</button>
            <button class="btn btn-shy" onclick="setExpression('shy')">Shy</button>
            <button class="btn btn-lovely" onclick="setExpression('lovely')">Lovely</button>
        </div>

        <div class="expression-divider"><span>Misc</span></div>

        <div class="expression-grid">
            <button class="btn btn-rainbow" onclick="setExpression('rainbow')">Rainbow</button>
            <button class="btn btn-music" onclick="setExpression('music')">Music</button>
            <button class="btn btn-flashing" onclick="setExpression('flashing')">Flashing</button>
            <button class="btn btn-dead" onclick="setExpression('dead')">Dead</button>
            <button class="btn btn-check" onclick="setExpression('check')">Check</button>
            <button class="btn btn-bigeyes" onclick="setExpression('bigeyes')">Big Eyes</button>
            <button class="btn btn-binaryclock" onclick="setExpression('binaryclock')">Binary</button>
            <button class="btn btn-matrix" onclick="setExpression('matrix')">Matrix</button>
            <button class="btn btn-loading" onclick="setExpression('loading')">Loading</button>
        </div>

        <div class="status connected" id="status">Connected</div>
    </div>

    <script>
        async function setExpression(type) {
            const statusEl = document.getElementById('status');
            statusEl.textContent = 'Loading...';
            statusEl.classList.remove('connected');
            statusEl.classList.add('loading');

            try {
                const response = await fetch('/api/expression', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded',
                    },
                    body: 'type=' + encodeURIComponent(type)
                });

                if (response.ok) {
                    statusEl.textContent = 'Expression changed: ' + type.charAt(0).toUpperCase() + type.slice(1);
                    statusEl.classList.remove('loading');
                    statusEl.classList.add('connected');
                } else {
                    statusEl.textContent = 'Error changing expression';
                }
            } catch (error) {
                statusEl.textContent = 'Connection error';
                console.error('Error:', error);
            }
        }

        // Check connection status on load
        window.addEventListener('load', async () => {
            try {
                const response = await fetch('/api/status');
                if (response.ok) {
                    document.getElementById('status').textContent = 'Connected';
                    document.getElementById('status').classList.add('connected');
                }
            } catch (error) {
                console.error('Status check failed:', error);
            }
        });
    </script>
</body>
</html>
)rawliteral";
  }
};

// Define static constants
const char *WebServerManager::AP_SSID = "SCP-1471-Mask";
const char *WebServerManager::AP_PASSWORD = "MaskControl123";
const char *WebServerManager::MDNS_NAME = "mask";

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
    MODE_SELECTION,
    OFF,
    SNAKEGAME,
    TOWERGAME,
    ERROR,
    SIZE
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

    bool isSnake() const
    {
      return currentMode == Mode::SNAKEGAME;
    }

    bool isOff() const
    {
      return currentMode == Mode::OFF;
    }

    bool isError() const
    {
      return currentMode == Mode::ERROR;
    }

    void selectNextMode()
    {
      selectedModeIndex = (selectedModeIndex + 1) % static_cast<uint8_t>(Mode::SIZE);
    }
    void selectPreviousMode()
    {
      if (selectedModeIndex == 0)
      {
        selectedModeIndex = static_cast<uint8_t>(Mode::SIZE) - 1;
      }
      else
      {
        selectedModeIndex--;
      }
    }
    bool confirmSelectedMode()
    {
      Mode selectedMode = static_cast<Mode>(selectedModeIndex);
      // Only allow selection of valid modes
      // Don't skip OFF mode to allow user to turn off the device
      if (selectedMode != Mode::MODE_SELECTION &&
          selectedMode != Mode::ERROR)
      {
        setMode(selectedMode);
        return true;
      }
      return false;
    }

    void render(MaskFrame &frame)
    {
      if (currentMode != Mode::MODE_SELECTION)
        return;

      static const uint8_t NUM_MODES = static_cast<uint8_t>(Mode::SIZE);

      // Define colors based on mode type
      const RGB selectableColor = {0, 255, 0};    // Green for selectable modes (ACTIVE, MANUAL, SNAKEGAME, TOWERGAME)
      const RGB nonSelectableColor = {255, 0, 0}; // Red for non-selectable modes (MODE_SELECTION, OFF, ERROR)
      const RGB selectedColor = {100, 255, 255};  // Cyanish for selected mode

      // Determine if a mode is selectable (not MODE_SELECTION, OFF, or ERROR)
      auto isSelectableMode = [](Mode mode) -> bool
      {
        return mode != Mode::MODE_SELECTION &&
               mode != Mode::OFF &&
               mode != Mode::ERROR;
      };

      // Display all modes as pixels (up to 16 pixels available: 4x4 per eye)
      // Left eye: modes 0-3 in column 0, modes 4-7 in column 1
      // Right eye: modes 0-3 in column 0, modes 4-7 in column 1
      for (uint8_t i = 0; i < NUM_MODES; i++)
      {
        Mode mode = static_cast<Mode>(i);
        RGB color;

        // Determine color based on selection and selectability
        if (i == selectedModeIndex)
        {
          color = selectedColor; // Cyan for selected
        }
        else if (isSelectableMode(mode))
        {
          color = selectableColor; // Green for selectable
        }
        else
        {
          color = nonSelectableColor; // Red for non-selectable
        }

        // Calculate position (distribute across both eyes)
        uint8_t row = i % 4;
        uint8_t col = (i / 4) % 2;

        if (i < 8)
        {
          frame.left[row][col] = color;
        }
        else if (i < 16)
        {
          frame.right[row][col] = color;
        }
      }

      // Display selection indicator on far right columns (column 3)
      // Animated arrow pointing at selected mode
      uint8_t indicatorRow = selectedModeIndex % 4;
      bool indicatorOnLeft = selectedModeIndex < 8;

      if (indicatorOnLeft)
      {
        frame.left[indicatorRow][3] = {255, 255, 255};
      }
      else
      {
        frame.right[indicatorRow][3] = {255, 255, 255};
      }
    }

  private:
    Mode currentMode;
    Mode lastMode;
    uint8_t selectedModeIndex = 0; // Index of currently highlighted mode in selection screen
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
    uint32_t expressionTimer = 0;
    uint32_t nextExpressionTime = 0;
    bool expressionTaggedForChange = false;
    Expressions::Type currentExpression;
    Expressions::Type quickExpression;
    MaskFrame *frame;

  public:
    ExpressionManager(MaskFrame *frame)
        : currentExpression(Expressions::Type::Neutral),
          quickExpression(Expressions::Type::Neutral),
          frame(frame)
    {
      if (frame == nullptr)
      {
        // Handle null frame pointer
      }
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

    Expressions::Type nextNormalExpression()
    {
      int next = static_cast<int>(currentExpression) + 1;
      if (next >= static_cast<int>(Expressions::Type::NORMAL_EXPRESSION_END))
        next = 0;

      currentExpression = static_cast<Expressions::Type>(next);
      return currentExpression;
    }

    Expressions::Type previousNormalExpression()
    {
      int prev = static_cast<int>(currentExpression) - 1;
      if (prev < 0)
        prev = static_cast<int>(Expressions::Type::NORMAL_EXPRESSION_END) - 1;
      if (prev >= static_cast<int>(Expressions::Type::NORMAL_EXPRESSION_END))
        prev = static_cast<int>(Expressions::Type::Neutral);
      currentExpression = static_cast<Expressions::Type>(prev);
      return currentExpression;
    }

    Expressions::Type nextMiscExpression()
    {
      int next = static_cast<int>(currentExpression) + 1;
      if (next < static_cast<int>(Expressions::Type::NORMAL_EXPRESSION_END) ||
          next >= static_cast<int>(Expressions::Type::SIZE))
      {
        next = static_cast<int>(Expressions::Type::NORMAL_EXPRESSION_END);
      }

      currentExpression = static_cast<Expressions::Type>(next);
      return currentExpression;
    }

    Expressions::Type previousMiscExpression()
    {
      int prev = static_cast<int>(currentExpression) - 1;
      if (prev < static_cast<int>(Expressions::Type::NORMAL_EXPRESSION_END))
      {
        prev = static_cast<int>(Expressions::Type::SIZE) - 1;
      }

      currentExpression = static_cast<Expressions::Type>(prev);
      return currentExpression;
    }

    void quickSwitch()
    {
      Expressions::Type temp = currentExpression;
      currentExpression = quickExpression;
      quickExpression = temp;
    }

    void tagQuickExpression()
    {
      quickExpression = currentExpression;
    }

    Expressions::Type getCurrentExpression() const { return currentExpression; }
    Expressions::Type getQuickExpression() const { return quickExpression; }

    void setForChange(uint32_t timeFromNow, uint32_t maxTime = 10000)
    {
      if (nextExpressionTime < expressionTimer)
      {
        nextExpressionTime = expressionTimer + timeFromNow > expressionTimer + maxTime ? expressionTimer + maxTime : expressionTimer + timeFromNow;
      }
      else
      {
        nextExpressionTime = nextExpressionTime + timeFromNow > expressionTimer + maxTime ? expressionTimer + maxTime : nextExpressionTime + timeFromNow;
      }
      expressionTaggedForChange = true;
    }

    void updateFrame()
    {
      if (frame != nullptr)
        Expressions::render(currentExpression, *frame);
    }

    void update(uint32_t deltaTime)
    {
      expressionTimer += deltaTime;
      updateFrame();

      if (expressionTaggedForChange && expressionTimer >= nextExpressionTime)
      {
        quickSwitch();
        expressionTaggedForChange = false;
      }
    }
  };
}

// ============================================
// BUTTON HANDLER
// ============================================

#define MAX_BUTTONS 3
#define MAX_ACTIONS 12
#define INVALID_PIN 0xFF

class ButtonHandler
{
public:
  enum class ButtonEvent
  {
    Press, // Fires immediately when button is pressed
    Tap,
    DoubleTap,
    Hold,
    Release
  };

  using ActionCallback = void (*)();

  ButtonHandler(uint32_t doubleTapThreshold = 300, uint32_t holdThreshold = 500)
      : doubleTapThreshold(doubleTapThreshold), holdThreshold(holdThreshold), debounceDelay(20) // Reduced for faster response
  {
    for (int i = 0; i < MAX_ACTIONS; i++)
    {
      actions[i].buttonPin = INVALID_PIN;
      actions[i].callback = nullptr;
    }

    for (int i = 0; i < MAX_BUTTONS; i++)
    {
      buttons[i].pin = INVALID_PIN;
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
      if (actions[i].buttonPin == INVALID_PIN)
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

      if (state.pin == INVALID_PIN)
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

          // Check if we should fire the pending tap (no double-tap occurred)
          if (state.pendingTap && state.lastReleaseTime >= doubleTapThreshold)
          {
            triggerActions(state.pin, ButtonEvent::Tap);
            state.pendingTap = false;
          }
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
      // Cannot represent more than 8 buttons with a single byte bitmask
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
    uint8_t pin = INVALID_PIN;
    bool isPressed = false;
    bool lastPhysicalState = false;
    uint32_t pressTime = 0;
    uint32_t lastReleaseTime = 0;
    bool holdTriggered = false;
    uint32_t lastChangeTime = 0;
    bool pendingTap = false;
    bool combinationTriggered = false;
  };

  struct Action
  {
    uint8_t buttonPin = INVALID_PIN;
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
    // Check if any OTHER button is currently held
    for (int i = 0; i < MAX_BUTTONS; ++i)
    {
      if (buttons[i].pin != INVALID_PIN && buttons[i].pin != buttonPin &&
          buttons[i].isPressed && buttons[i].pressTime >= holdThreshold)
      {
        // Mark that held button as part of a combination
        buttons[i].combinationTriggered = true;
      }
    }

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
      if (state.pin == INVALID_PIN)
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
            state.combinationTriggered = false;
            // Trigger instant press event for fast game response
            triggerActions(state.pin, ButtonEvent::Press);
          }
          else // BUTTON RELEASED
          {
            if (!state.holdTriggered && state.pressTime < holdThreshold)
            {
              // It was a short press - check for double tap first
              checkDoubleTap(state);
            }

            // Only trigger release event if button was held and no combination was performed
            if (state.holdTriggered && !state.combinationTriggered)
            {
              triggerActions(state.pin, ButtonEvent::Release);
            }

            if (!state.holdTriggered && state.pressTime < holdThreshold)
            {
              state.lastReleaseTime = 0;
            }
          }
        }
      }
    }
  }

  void checkDoubleTap(ButtonState &state)
  {
    if (state.lastReleaseTime > 0 && state.lastReleaseTime < doubleTapThreshold)
    {
      // This is the second tap - fire double-tap and cancel any pending single tap
      triggerActions(state.pin, ButtonEvent::DoubleTap);
      state.lastReleaseTime = doubleTapThreshold; // Prevent further taps
      state.pendingTap = false;
    }
    else
    {
      // This is the first tap - mark it as pending and start the timer
      state.lastReleaseTime = 0;
      state.pendingTap = true;
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
// SNAKE GAME
// ============================================

namespace Game
{
  class SnakeGame
  {
    class SnakeSegment
    {
    public:
      uint8_t x;
      uint8_t y;
      SnakeSegment *next;

      SnakeSegment(uint8_t x, uint8_t y)
          : x(x), y(y), next(nullptr) {}
      ~SnakeSegment()
      {
        for (SnakeSegment *seg = next; seg != nullptr;)
        {
          SnakeSegment *nextSeg = seg->next;
          delete seg;
          seg = nextSeg;
        }
      }
    };

  private:
    uint32_t lastUpdateTime;
    SnakeSegment *head;
    SnakeSegment *tail;
    uint8_t direction;    // 0=up, 1=right, 2=down, 3=left
    uint8_t foodLocation; // Encoded as (y * 4 + x), range 0-31

    uint16_t score;
    uint8_t gameSpeed;
    bool isStarted;
    bool isGameOver;
    bool showLeaderboard;

  public:
    SnakeGame()
        : lastUpdateTime(0), head(nullptr), tail(nullptr), direction(1),
          foodLocation(0), score(0), gameSpeed(1), isStarted(false),
          isGameOver(false), showLeaderboard(false) {}
    ~SnakeGame()
    {
      if (head != nullptr)
      {
        delete head;
      }
    }

    void initializeGame(uint8_t level = 1)
    {
      // Initialize game state
      isStarted = false; // Wait for user input to start
      isGameOver = false;
      direction = 1; // Start moving right
      // Base score on level, prevent overflow
      score = (level > 0 && level - 1 <= UINT16_MAX / 32) ? (level - 1) * 32 : 0;
      gameSpeed = level; // Initial speed based on level
      lastUpdateTime = 0;
      showLeaderboard = false;
      // Initialize snake segments
      head = new SnakeSegment(1, 2);
      tail = head;
      // Place initial food
      placeFood();
    }

    void resetGame()
    {
      // Clean up existing snake
      if (head != nullptr)
      {
        delete head;
        head = nullptr;
        tail = nullptr;
      }
      // Reinitialize the game
      initializeGame(1);
    }

    void resumeGame()
    {
      if (!isGameOver)
      {
        isStarted = true;
      }
    }

    void pauseGame()
    {
      if (!isGameOver)
      {
        isStarted = false;
      }
    }

    void update(uint32_t deltaTime)
    {
      if (isStarted && !isGameOver)
      {
        lastUpdateTime += deltaTime;
        // Prevent division by zero
        uint32_t speedInterval = (gameSpeed > 0) ? (1000 / gameSpeed) : 1000;

        if (lastUpdateTime >= speedInterval)
        {
          lastUpdateTime = 0;
          tick();
        }
      }
    }

    void render(MaskFrame &frame)
    {
      const uint8_t snakeR = 25;
      const uint8_t snakeG = 255;
      const uint8_t snakeB = 0;

      const uint8_t foodR = 255;
      const uint8_t foodG = 160;
      const uint8_t foodB = 0;

      frame.clear();

      if (showLeaderboard && isGameOver)
      {
        // Render leaderboard in binary format
        // Score is displayed as binary digits, each column represents a digit (0-9)
        // Rightmost columns are least significant digits
        // Different colors for each digit position

        uint16_t displayScore = score;
        const uint8_t NUM_DIGITS = 4; // Display up to 4 digits (0-9999)

        // Define colors for each digit position (right to left)
        const RGB digitColors[NUM_DIGITS] = {
            {0, 255, 0},   // Ones place - Green
            {0, 255, 255}, // Tens place - Cyan
            {255, 255, 0}, // Hundreds place - Yellow
            {255, 0, 255}  // Thousands place - Magenta
        };

        // Extract each digit and display in binary (4 bits each)
        for (uint8_t digitPos = 0; digitPos < NUM_DIGITS; digitPos++)
        {
          uint8_t digit = displayScore % 10;
          displayScore /= 10;

          RGB color = digitColors[digitPos];

          // Display 4 bits of the digit in a column (right to left)
          // Column index: 3-digitPos for left eye, 7-digitPos for right eye
          uint8_t leftCol = 3 - digitPos;
          uint8_t rightCol = 7 - digitPos;

          for (uint8_t bit = 0; bit < 4; bit++)
          {
            if (digit & (1 << bit))
            {
              if (digitPos < 4)
              {
                // Left eye columns 3,2,1,0
                frame.left[3 - bit][leftCol] = color;
              }
              else
              {
                // Right eye columns 3,2,1,0 (if score > 9999)
                frame.right[3 - bit][rightCol - 4] = color;
              }
            }
          }
        }
      }

      if (!isStarted || isGameOver && millis() % 1000 < 500)
      {
        return;
      }

      // Render snake
      for (SnakeSegment *seg = head; seg != nullptr; seg = seg->next)
      {
        uint8_t x = seg->x;
        uint8_t y = seg->y;

        if (x < 4)
        {
          // Left eye (x = 0-3)
          frame.left[y][x] = {snakeR, snakeG, snakeB};
        }
        else
        {
          // Right eye (x = 4-7)
          frame.right[y][x - 4] = {snakeR, snakeG, snakeB};
        }
      }

      // Render food
      uint8_t foodX = foodLocation % 8;
      uint8_t foodY = foodLocation / 8;

      if (foodX < 4)
      {
        // Left eye
        frame.left[foodY][foodX] = {foodR, foodG, foodB};
      }
      else
      {
        // Right eye
        frame.right[foodY][foodX - 4] = {foodR, foodG, foodB};
      }
    }

    // Helpers
    void tick()
    {
      if (isStarted && !isGameOver)
      {
        if (moveSnake())
        {
          // Successful move
        }
        else
        {
          isStarted = false;
          isGameOver = true;
        }
      }
    }

    bool moveSnake()
    {
      // Calculate new head position based on direction
      uint8_t newX = head->x;
      uint8_t newY = head->y;

      switch (direction)
      {
      case 0: // Up
        newY = (newY == 0) ? 3 : newY - 1;
        break;
      case 1: // Right
        newX = (newX + 1) % 8;
        break;
      case 2: // Down
        newY = (newY + 1) % 4;
        break;
      case 3: // Left
        newX = (newX == 0) ? 7 : newX - 1;
        break;
      }

      // Check for self-collision
      if (hasSnakeAt(newX, newY))
      {
        return false;
      }

      // Create new head segment
      SnakeSegment *newHead = new SnakeSegment(newX, newY);
      newHead->next = head;
      head = newHead;

      // Check for food consumption
      if (foodLocation == (newY * 8 + newX))
      {
        // Prevent score overflow
        if (score < UINT16_MAX)
        {
          score++;
        }
        if (!placeFood())
        {
          initializeGame(++gameSpeed); // Restart if no space for food
        }
      }
      else
      {
        // Remove tail segment
        SnakeSegment *prev = nullptr;
        for (SnakeSegment *seg = head; seg != tail; seg = seg->next)
        {
          prev = seg;
        }
        delete tail;
        tail = prev;
        if (tail)
          tail->next = nullptr;
      }

      return true;
    }

    bool hasSnakeAt(uint8_t x, uint8_t y)
    {
      for (SnakeSegment *seg = head; seg != nullptr; seg = seg->next)
      {
        if (seg->x == x && seg->y == y)
        {
          return true;
        }
      }
      return false;
    }

    bool placeFood()
    {
      uint8_t attempts = 0;
      const uint8_t MAX_ATTEMPTS = 64; // 8x4 = 32 total cells, try twice

      while (attempts < MAX_ATTEMPTS)
      {
        uint8_t x = random(0, 8);
        uint8_t y = random(0, 4);

        if (!hasSnakeAt(x, y))
        {
          foodLocation = y * 8 + x; // Encode for 8-wide grid
          return true;
        }
        attempts++;
      }

      // No valid location found (snake fills the grid)
      return false;
    }

    // Actions
    void leftButtonAction()
    {
      if (!isStarted && !isGameOver)
      {
        isStarted = true;
        return;
      }
      else if (isGameOver)
      {
        resetGame();
        return;
      }

      changeDirectionCounterClockwise();
    }

    void rightButtonAction()
    {
      if (isGameOver)
      {
        showLeaderboard = !showLeaderboard;
        return;
      }

      changeDirectionClockwise();
    }

    void changeDirectionClockwise()
    {
      direction = (direction + 1) % 4;
    }
    void changeDirectionCounterClockwise()
    {
      direction = (direction + 3) % 4;
    }
  };

  // ============================================
  // TOWER GAME
  // ============================================
  /// <summary>
  /// Simple Tower Placement Game, where user tries to stack blocks as high as possible.
  /// </summary>
  class TowerGame
  {
  private:
    uint32_t lastUpdateTime;
    struct gameStats
    {
      uint8_t gameSpeed;               // Speed of block movement
      uint8_t towerHeight;             // Current height of the tower
      uint8_t firstPlaceBlockPosition; // Position of the first block in the current row
      uint8_t currentBlockPosition;    // Current position of the moving block
      bool directionDown;              // Direction of block movement
      bool isGameOver;
      bool isPaused;
    } stats;

  public:
    TowerGame() : lastUpdateTime(0)
    {
      stats.gameSpeed = 1;
      stats.towerHeight = 0;
      stats.firstPlaceBlockPosition = 0;
      stats.currentBlockPosition = 0;
      stats.directionDown = true;
      stats.isGameOver = false;
      stats.isPaused = false;
    }
    ~TowerGame() {}

    void initializeGame(uint8_t level = 1)
    {
      // Initialize game state
      stats.isGameOver = false;
      stats.gameSpeed = level; // Initial speed based on level
      stats.towerHeight = 0;
      lastUpdateTime = 0;
    }

    void resetGame()
    {
      // Reinitialize the game
      initializeGame(1);
    }

    void resumeGame()
    {
      if (!stats.isGameOver)
      {
        stats.isPaused = false;
      }
    }
    void pauseGame()
    {
      if (!stats.isGameOver)
      {
        stats.isPaused = true;
      }
    }

    void tick()
    {
      if (stats.isPaused)
      {
        return;
      }

      if (!stats.isGameOver)
      {
        if (stats.directionDown)
        {
          stats.currentBlockPosition++;
          if (stats.currentBlockPosition > 3)
          {
            stats.currentBlockPosition = 3;
            stats.directionDown = false;
          }
        }
        else
        {
          if (stats.currentBlockPosition == 0)
          {
            stats.currentBlockPosition = 0;
            stats.directionDown = true;
          }
          else
          {
            stats.currentBlockPosition--;
          }
        }
      }
    }
    // Actions
    void leftButtonAction()
    {
      placeBlock();
    }
    void rightButtonAction()
    {
      if (stats.isGameOver || isWin())
      {
        resetGame();
        return;
      }
      if (stats.isPaused)
      {
        resumeGame();
      }
      else
      {
        pauseGame();
      }
    }
    void placeBlock()
    {
      if (stats.isPaused || stats.isGameOver || isWin())
      {
        return;
      }

      if (stats.towerHeight == 0)
      {
        // First block placement
        stats.firstPlaceBlockPosition = stats.currentBlockPosition;
        stats.towerHeight++;
        stats.currentBlockPosition = 0;
        stats.directionDown = true;
        return;
      }

      // Check if block is aligned with the tower
      if (stats.currentBlockPosition == stats.firstPlaceBlockPosition)
      {
        // Successful placement
        stats.towerHeight++;
        stats.firstPlaceBlockPosition = stats.currentBlockPosition;
        stats.currentBlockPosition = 0;
        stats.directionDown = true;
        // Increase speed
        stats.gameSpeed++;
      }
      else
      {
        // Game over
        stats.isGameOver = true;
      }
    }
    // Helpers
    bool isWin() const
    {
      return stats.towerHeight >= 8; // Win condition: tower height of 8, filling the display
    }
    // Update and Render
    void update(uint32_t deltaTime)
    {
      lastUpdateTime += deltaTime;
      // Prevent division by zero
      uint32_t speedInterval = (stats.gameSpeed > 0) ? (1000 / stats.gameSpeed) : 1000;
      if (lastUpdateTime >= speedInterval)
      {
        lastUpdateTime = 0;
        tick();
      }
    }

    void render(MaskFrame &frame)
    {
      frame.clear();

      // Define colors
      const uint8_t towerR = 0, towerG = 0, towerB = 255;          // Blue blocks
      const uint8_t movingR = 255, movingG = 255, movingB = 0;     // Yellow moving block
      const uint8_t gameOverR = 255, gameOverG = 0, gameOverB = 0; // Red for game over
      const uint8_t pausedR = 255, pausedG = 255, pausedB = 255;   // White for paused
      const uint8_t winR = 0, winG = 255, winB = 0;                // Green for win

      if (stats.towerHeight > 0 && !isWin() && !stats.isGameOver && !stats.isPaused)
      {

        if (stats.towerHeight < 4)
        {
          // Draw tower on left eye
          for (uint8_t h = 0; h < stats.towerHeight; h++)
          {
            frame.left[3 - h][stats.firstPlaceBlockPosition] = {towerR, towerG, towerB};
          }
        }
        else
        {
          // Draw tower on both eyes
          for (uint8_t h = 0; h < stats.towerHeight; h++)
          {
            if (h < 4)
            {
              frame.left[3 - h][stats.firstPlaceBlockPosition] = {towerR, towerG, towerB};
            }
            else
            {
              frame.right[7 - h][stats.firstPlaceBlockPosition] = {towerR, towerG, towerB};
            }
          }
        }
      }

      // Draw moving block
      if (stats.towerHeight < 4)
      {
        frame.left[3 - stats.towerHeight][stats.currentBlockPosition] = {movingR, movingG, movingB};
      }
      else
      {
        if (stats.towerHeight == 4)
        {
          frame.right[3][stats.currentBlockPosition] = {movingR, movingG, movingB};
        }
        else
        {
          frame.right[7 - (stats.towerHeight - 4)][stats.currentBlockPosition] = {movingR, movingG, movingB};
        }
      }
      // Indicate game over, paused, or win state
      if (stats.isGameOver)
      {
        // Flash the tower in red
        for (uint8_t h = 0; h < stats.towerHeight; h++)
        {
          if (h < 4)
          {
            frame.left[3 - h][stats.firstPlaceBlockPosition] = {gameOverR, gameOverG, gameOverB};
          }
          else
          {
            frame.right[7 - h][stats.firstPlaceBlockPosition] = {gameOverR, gameOverG, gameOverB};
          }
        }
      }
      else if (stats.isPaused)
      {
        // Flash the moving block in white
        if (stats.towerHeight < 4)
        {
          frame.left[3 - stats.towerHeight][stats.currentBlockPosition] = {pausedR, pausedG, pausedB};
        }
        else
        {
          if (stats.towerHeight == 4)
          {
            frame.right[3][stats.currentBlockPosition] = {pausedR, pausedG, pausedB};
          }
          else
          {
            frame.right[7 - (stats.towerHeight - 4)][stats.currentBlockPosition] = {pausedR, pausedG, pausedB};
          }
        }
      }
      else if (isWin())
      {
        // Flash the tower in green
        for (uint8_t h = 0; h < stats.towerHeight; h++)
        {
          if (h < 4)
          {
            frame.left[3 - h][stats.firstPlaceBlockPosition] = {winR, winG, winB};
          }
          else
          {
            frame.right[7 - h][stats.firstPlaceBlockPosition] = {winR, winG, winB};
          }
        }
      }
    };
  };
}

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
constexpr unsigned long DOUBLE_TAP_TIME = 300;           // milliseconds
constexpr unsigned long HOLD_TIME = 700;                 // milliseconds
ButtonHandler buttonHandler(DOUBLE_TAP_TIME, HOLD_TIME); // 300ms for double tap, 700ms for hold
MaskFrame frame = MaskFrame();
Core::ModeManager modeManager = Core::ModeManager();
Core::ExpressionManager expressionManager = Core::ExpressionManager(&frame);
LedController ledController = LedController(NEO_PIN, NEO_NUMPIXEL);
WebServerManager webServer;

Game::SnakeGame snakeGame = Game::SnakeGame();
Game::TowerGame towerGame = Game::TowerGame();

// Forward declarations
void onButton1Press();
void onButton1Tap();
void onButton1DoubleTap();
void onButton1Hold();
void onButton1Release();
void onButton2Press();
void onButton2Tap();
void onButton2DoubleTap();
void onButton2Hold();
void onButton2Release();

void setup()
{
  Serial.begin(9600);
  // Serial.println("System Initializing...");
  delay(1000); // Give serial time to stabilize

  // Initialize random seed for better randomness
  randomSeed(analogRead(A0));

  // Initialize button handler with pins
  buttonHandler.begin(BUTTON1_PIN, BUTTON2_PIN, BUTTON3_PIN);

  // Register button actions
  buttonHandler.registerAction(BUTTON1_PIN, ButtonHandler::ButtonEvent::Press, onButton1Press);
  buttonHandler.registerAction(BUTTON1_PIN, ButtonHandler::ButtonEvent::Tap, onButton1Tap);
  buttonHandler.registerAction(BUTTON1_PIN, ButtonHandler::ButtonEvent::DoubleTap, onButton1DoubleTap);
  buttonHandler.registerAction(BUTTON1_PIN, ButtonHandler::ButtonEvent::Hold, onButton1Hold);
  buttonHandler.registerAction(BUTTON1_PIN, ButtonHandler::ButtonEvent::Release, onButton1Release);
  buttonHandler.registerAction(BUTTON2_PIN, ButtonHandler::ButtonEvent::Press, onButton2Press);
  buttonHandler.registerAction(BUTTON2_PIN, ButtonHandler::ButtonEvent::Tap, onButton2Tap);
  buttonHandler.registerAction(BUTTON2_PIN, ButtonHandler::ButtonEvent::DoubleTap, onButton2DoubleTap);
  buttonHandler.registerAction(BUTTON2_PIN, ButtonHandler::ButtonEvent::Hold, onButton2Hold);
  buttonHandler.registerAction(BUTTON2_PIN, ButtonHandler::ButtonEvent::Release, onButton2Release);

  ledController.begin();
  ledController.setBrightness(5);

  frame.clear();
  expressionManager.setExpression(Expressions::Type::Neutral);
  modeManager.setMode(Core::Mode::ACTIVE);

  webServer.begin([](Expressions::Type type)
                  { expressionManager.setExpression(type); });
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
    expressionManager.update(deltaTime);
    break;
  case Core::Mode::MANUAL:
    // In manual mode, expression is controlled by button actions
    break;
  case Core::Mode::MODE_SELECTION:
    modeManager.render(frame);
    break;
  case Core::Mode::SNAKEGAME:
    snakeGame.update(deltaTime);
    snakeGame.render(frame);
    break;
  case Core::Mode::TOWERGAME:
    towerGame.update(deltaTime);
    towerGame.render(frame);
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
  webServer.update();

  delay(10); // Small delay for stability
}

// --- BUTTON ACTION HANDLERS ---
// Press handlers (instant response for games)
void onButton1Press()
{
  // Instant response for games - fires immediately on press
  switch (modeManager.getMode())
  {
  case Core::Mode::SNAKEGAME:
    snakeGame.leftButtonAction();
    break;
  case Core::Mode::TOWERGAME:
    towerGame.leftButtonAction();
    break;
  default:
    // No instant action for other modes
    break;
  }
}

void onButton2Press()
{
  // Instant response for games - fires immediately on press
  switch (modeManager.getMode())
  {
  case Core::Mode::SNAKEGAME:
    snakeGame.rightButtonAction();
    break;
  case Core::Mode::TOWERGAME:
    towerGame.rightButtonAction();
    break;
  default:
    // No instant action for other modes
    break;
  }
}

// Tap handlers
void onButton1Tap()
{
  Serial.println("Button 1: Tap");

  if (!buttonHandler.isButtonHeld(BUTTON2_PIN))
  {
    if (wakeUp())
    {
      return;
    }

    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
      expressionManager.nextNormalExpression();
      break;
    case Core::Mode::MODE_SELECTION:
      modeManager.selectNextMode();
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

void onButton2Tap()
{
  Serial.println("Button 2: Tap");
  if (!buttonHandler.isButtonHeld(BUTTON1_PIN))
  {
    if (wakeUp())
    {
      return;
    }
    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
      expressionManager.previousNormalExpression();
      break;
    case Core::Mode::MODE_SELECTION:
      modeManager.selectPreviousMode();
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

// Double tap handlers
void onButton1DoubleTap()
{
  Serial.println("Button 1: Double Tap");
  if (!buttonHandler.isButtonHeld(BUTTON2_PIN))
  {
    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
      setForQuickExpressionChange();
      break;
    case Core::Mode::MODE_SELECTION:
      if (!modeManager.confirmSelectedMode())
      {
        modeManager.setMode(Core::Mode::OFF); // Invalid selection, turn off
      }
      break;
    default:
      break;
    }
  }
  else
  {
    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
      expressionManager.nextMiscExpression();
      break;

    default:
      break;
    }
  }
}

void onButton2DoubleTap()
{
  Serial.println("Button 2: Double Tap");
  if (!buttonHandler.isButtonHeld(BUTTON1_PIN))
  {
    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
      expressionManager.quickSwitch();
      break;

    default:
      break;
    }
  }
  else
  {
    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
      expressionManager.previousMiscExpression();
      break;

    default:
      break;
    }
  }
}

// Hold handlers
void onButton1Hold()
{
  Serial.println("Button 1: Hold");
  if (!buttonHandler.isButtonHeld(BUTTON2_PIN))
  {
    // Button 2 is not held
  }
  else
  {
    switch (modeManager.getMode())
    {
    case Core::Mode::ACTIVE:
    case Core::Mode::MANUAL:
      modeManager.setMode(Core::Mode::MODE_SELECTION);
      break;
    case Core::Mode::SNAKEGAME:
      snakeGame.pauseGame();
      modeManager.setMode(Core::Mode::MODE_SELECTION);
      break;
    case Core::Mode::TOWERGAME:
      towerGame.pauseGame();
      modeManager.setMode(Core::Mode::MODE_SELECTION);
      break;
    default:
      break;
    }
  }
}

void onButton2Hold()
{
  Serial.println("Button 2: Hold");
}

// Release handlers
void onButton1Release()
{
  Serial.println("Button 1: Release");
  switch (modeManager.getMode())
  {
  case Core::Mode::OFF:
    wakeUp();
    break;
  case Core::Mode::ACTIVE:
    toggleOnOffMode();
    break;

  default:
    break;
  }
}

void onButton2Release()
{
  Serial.println("Button 2: Release");
  switch (modeManager.getMode())
  {
  case Core::Mode::OFF:
    break;
  case Core::Mode::ACTIVE:
    expressionManager.tagQuickExpression();
    break;
  default:
    break;
  }
}

// --- HELPER FUNCTIONS ---

bool wakeUp()
{
  if (modeManager.isOff())
  {
    modeManager.setMode(modeManager.getLastMode());
    return true;
  }
  else
  {
    return false;
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

void setForQuickExpressionChange()
{
  expressionManager.setForChange(2000, 15000);
}
