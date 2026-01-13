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
        Expressions::Type selectedExpression;
        MaskFrame *frame;

    public:
        ExpressionManager(MaskFrame* frame) : currentExpression(Expressions::Type::Neutral), frame(frame) {}

        void setExpression(); // Sets to selectedExpression
        void setExpression(Expressions::Type type);
        Expressions::Type nextExpression();
        Expressions::Type previousExpression();
        void selectExpression(Expressions::Type type);
        Expressions::Type selectNextExpression();
        Expressions::Type selectPreviousExpression();
        Expressions::Type getSelectedExpression() const { return selectedExpression; }
        Expressions::Type getCurrentExpression() const { return currentExpression; }
        void updateFrame();
    };
}
#endif // EXPRESSION_MANAGER_H
