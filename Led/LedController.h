#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "FrameBuffer.h"
#include "Math/Orientation.h"

class LedController {
public:
  LedController(uint8_t pin, uint16_t ledCount);

  // Must be called once in setup()
  void begin();

  // Push a full frame to the LEDs
  void present(const MaskFrame& frame);

  // Optional global brightness control
  void setBrightness(uint8_t brightness);
  uint8_t getBrightness() const;

  MaskFrame getCorrectedFrame(const MaskFrame& frame) const;

private:
  Adafruit_NeoPixel strip;
  Orientation orientation_L = Orientation::NORMAL;
  Orientation orientation_R = Orientation::NORMAL;

  // Mapping helpers (logical → physical)
  uint16_t mapLeft(uint8_t x, uint8_t y) const;
  uint16_t mapRight(uint8_t x, uint8_t y) const;
};
