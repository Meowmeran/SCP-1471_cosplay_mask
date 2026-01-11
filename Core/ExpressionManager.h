#include "Led/Expressions.h"
#include "Led/FrameBuffer.h"

#ifndef EXPRESSION_MANAGER_H
#define EXPRESSION_MANAGER_H

namespace Core
{
    class ExpressionManager
    {
    private:
        uint32_t lastUpdateTime;
        Expressions::Type currentExpression;
        MaskFrame *frame;

    public:
        ExpressionManager(MaskFrame* frame) : currentExpression(Expressions::Type::Neutral), frame(frame) {}

        void setExpression(Expressions::Type type);
        Expressions::Type getCurrentExpression() const;
        void updateFrame();
    };
}
#endif // EXPRESSION_MANAGER_H
