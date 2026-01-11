#pragma once

#include "FrameBuffer.h"

class Expressions {
public:
    enum class Type {
        Neutral,
        Happy,
        Sad,
        Angry,
        Surprised,
        Lovely
    };

    // Take a frame, modify it to represent the given expression
    static void render(Type type, MaskFrame& frame);

private:
    static void renderNeutral(MaskFrame& frame, uint16_t time = 0);
    static void renderHappy(MaskFrame& frame);
    static void renderSad(MaskFrame& frame);
    static void renderAngry(MaskFrame& frame);
    static void renderSurprised(MaskFrame& frame);
    static void renderLovely(MaskFrame& frame);
};
