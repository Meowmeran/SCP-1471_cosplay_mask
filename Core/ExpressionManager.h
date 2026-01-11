#include "Core/ExpressionManager.h"

namespace Core
{
    ExpressionManager::ExpressionManager(MaskFrame* frame) : currentExpression(Expressions::Type::Neutral), frame(frame)
    {
        if (frame == nullptr)
        {
            Serial.println("Error: ExpressionManager initialized with null frame pointer.");
        }
        
    }

    void ExpressionManager::setExpression(Expressions::Type type)
    {
        currentExpression = type;
    }

    Expressions::Type ExpressionManager::getCurrentExpression() const
    {
        return currentExpression;
    }

    void ExpressionManager::updateFrame()
    {
        if (frame != nullptr)
        {
            Expressions::render(currentExpression, *frame);
        }
        else
        {
            Serial.println("Error: Cannot update frame, frame pointer is null.");
        }
    }
} // namespace Core
