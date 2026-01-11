#include "Expressions.h"

void Expressions::render(Type type, MaskFrame &frame)
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
    default:
        renderNeutral(frame);
        break;
    }
}

void Expressions::renderNeutral(MaskFrame &frame, uint16_t time)
{
    uint8_t r = 255;
    uint8_t g = 0;
    uint8_t b = 20;

    uint16_t cycle = time % 5000;                          // 5 second cycle
    uint16_t blinkPhase = cycle > 2500 ? cycle - 2500 : 0; // 0-500ms blink duration
    float blinkProgress = blinkPhase / 500.0f;             // 0.0 to 1.0

    for (uint8_t y = 0; y < 4; y++)
    {
        for (uint8_t x = 0; x < 4; x++)
        {
            uint8_t pixelIndex = y * 4 + x;
            uint8_t intensity = 0;

            if (blinkProgress > 0.0f)
            {
                // Blinking: pixels 4,5,6,7,8,9,10,11 close
                if (pixelIndex >= 4 && pixelIndex <= 11)
                {
                    intensity = (uint8_t)(255.0f * (1.0f - blinkProgress));
                }
                else
                {
                    intensity = 255;
                }
            }
            else
            {
                // Staring: brighten pupils (5,6,9,10) and surrounding pixels
                if (pixelIndex == 5 || pixelIndex == 6 || pixelIndex == 9 || pixelIndex == 10)
                {
                    intensity = 255;
                }
                else if (pixelIndex == 1 || pixelIndex == 4 || pixelIndex == 7 || pixelIndex == 8 || pixelIndex == 11 || pixelIndex == 14)
                {
                    intensity = 100;
                }
                else
                {
                    intensity = 0;
                }
            }

            frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
            frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        }
    }
}
void Expressions::renderHappy(MaskFrame &frame)
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

            // Main happy pixels: 8, 9, 10, 11
            if (pixelIndex == 5 || pixelIndex == 6 || pixelIndex == 8 || pixelIndex == 11)
            {
                intensity = 255;
            }
            // Anti-aliasing: neighboring pixels
            else if (pixelIndex == 1 || pixelIndex == 2 || pixelIndex == 4 || pixelIndex == 7 || pixelIndex == 9 || pixelIndex == 10 || pixelIndex == 12 || pixelIndex == 13 || pixelIndex == 14 || pixelIndex == 15)
            {
                intensity = 100;
            }

            frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
            frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        }
    }
}

void Expressions::renderSad(MaskFrame &frame)
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

            // Main sad pixels: 4, 5, 6, 7
            if (pixelIndex = 7 || pixelIndex == 9 || pixelIndex == 10 || pixelIndex == 11 || pixelIndex == 12)
            {
                intensity = 255;
            }
            // Anti-aliasing: neighboring pixels
            else if (pixelIndex == 4 || pixelIndex == 6 || pixelIndex == 8 || pixelIndex == 13 || pixelIndex == 14 || pixelIndex == 15)
            {
                intensity = 100;
            }

            frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
            frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        }
    }
}

void Expressions::renderSurprised(MaskFrame &frame)
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

            // Main surprised pixels: 5, 6, 9, 10
            if (pixelIndex == 9)
            {
                intensity = 255;
            }
            // Anti-aliasing: neighboring pixels
            else if (pixelIndex == 9 - 4 || pixelIndex == 9 - 1 || pixelIndex == 9 + 1 || pixelIndex == 9 + 4)
            {
                intensity = 30;
            }

            frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
            frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        }
    }
}

void Expressions::renderAngry(MaskFrame &frame)
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

            // Main angry pixels: 4, 5, 6, 7, 11
            if (pixelIndex == 4 || pixelIndex == 5 || pixelIndex == 6 || pixelIndex == 7 || pixelIndex == 11)
            {
                intensity = 255;
            }
            // Anti-aliasing: neighboring pixels
            else if (pixelIndex == 1 || pixelIndex == 3 || pixelIndex == 8 || pixelIndex == 10 || pixelIndex == 15)
            {
                intensity = 51;
            }

            frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
            frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        }
    }
}

void Expressions::renderLovely(MaskFrame &frame)
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

            // Heart pattern:
            // Row 0: 1,2,5,6     (top bumps)
            // Row 1: 0-7         (middle)
            // Row 2: 4-11        (wider middle)
            // Row 3: 6,9         (bottom point)
            if (pixelIndex == 0 || pixelIndex == 3 || pixelIndex == 4 || pixelIndex == 5 ||
                pixelIndex == 6 || pixelIndex == 7 || pixelIndex == 8 || pixelIndex == 9 || pixelIndex == 10 || pixelIndex == 11 || pixelIndex == 13 || pixelIndex == 14)
            {
                intensity = 255;
            }
            // Anti-aliasing upper part
            else if (pixelIndex == 1 || pixelIndex == 2)
            {
                intensity = 0;
            }
            // Anti-aliasing bottom part
            else if (pixelIndex == 12 || pixelIndex == 15)
            {
                intensity = 0;
            }

            frame.left[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
            frame.right[y][x] = {(uint8_t)(r * intensity / 255), (uint8_t)(g * intensity / 255), (uint8_t)(b * intensity / 255)};
        }
    }
}
