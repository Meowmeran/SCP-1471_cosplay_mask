#pragma once

#include <Arduino.h>

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
  uint32_t timeThreshold = 1000; // 1 second blink interval
  uint32_t lastUpdateTime = millis();
  MaskFrame frame;
  frame.clear();
  if (((millis() - lastUpdateTime) % (2 * timeThreshold)) < timeThreshold)
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      frame.left[i][i] = {255, 0, 0};
      frame.right[i][i] = {255, 0, 0};
    }
    return frame;
  }
  else
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      for (uint8_t j = 0; j < 4; j++)
      {
        frame.left[i][j] = {0, 0, 0};
        frame.right[i][j] = {0, 0, 0};
      }
    }
    lastUpdateTime = millis();
    return frame;
  }
}
