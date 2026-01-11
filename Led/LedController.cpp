#include "LedController.h"

LedController::LedController(uint8_t pin, uint16_t ledCount)
    : strip(ledCount, pin, NEO_GRB + NEO_KHZ800) {}
void LedController::begin()
{
    if (strip.begin())
    {
        Serial.println("LedController: NeoPixel strip initialized.");
    }
    else
    {
        Serial.println("LedController: Failed to initialize NeoPixel strip!");
    }
}
void LedController::present(const MaskFrame &frame)
{
    // Update left side
    for (uint8_t y = 0; y < 4; y++)
    {
        for (uint8_t x = 0; x < 4; x++)
        {
            uint16_t idx = mapLeft(x, y);
            const RGB &color = frame.left[y][x];
            strip.setPixelColor(idx, strip.Color(color.r, color.g, color.b));
        }
    }

    // Update right side
    for (uint8_t y = 0; y < 4; y++)
    {
        for (uint8_t x = 0; x < 4; x++)
        {
            uint16_t idx = mapRight(x, y);
            const RGB &color = frame.right[y][x];
            strip.setPixelColor(idx, strip.Color(color.r, color.g, color.b));
        }
    }

    strip.show();
}

MaskFrame LedController::getCorrectedFrame(const MaskFrame &frame) const
{
    MaskFrame correctedFrame;

    // Correct left side
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

    // Correct right side
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

void LedController::setBrightness(uint8_t brightness)
{
    strip.setBrightness(brightness);
}

uint8_t LedController::getBrightness() const
{
    return strip.getBrightness();
}

uint16_t LedController::mapLeft(uint8_t x, uint8_t y) const
{
    int physIdx = OrientationHelper::getPhysicalIndex(x, y, orientation_L);
    return static_cast<uint16_t>(physIdx);
}
uint16_t LedController::mapRight(uint8_t x, uint8_t y) const
{
    int physIdx = OrientationHelper::getPhysicalIndex(x, y, orientation_R);
    return static_cast<uint16_t>(physIdx + 16); // Right side starts after left side
}
