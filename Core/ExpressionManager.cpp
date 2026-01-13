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

    void ExpressionManager::setExpression(){
        currentExpression = selectedExpression;
    }

    Expressions::Type ExpressionManager::nextExpression()
    {
        int next = static_cast<int>(currentExpression) + 1;
        if (next >= static_cast<int>(Expressions::Type::SIZE))
        {
            next = 0;
        }
        currentExpression = static_cast<Expressions::Type>(next);
        return currentExpression;
    }
    Expressions::Type ExpressionManager::previousExpression()
    {
        int prev = static_cast<int>(currentExpression) - 1;
        if (prev < 0)
        {
            prev = static_cast<int>(Expressions::Type::SIZE) - 1;
        }
        currentExpression = static_cast<Expressions::Type>(prev);
        return currentExpression;
    }

    Expressions::Type ExpressionManager::selectNextExpression()
    {
        int next = static_cast<int>(selectedExpression) + 1;
        if (next >= static_cast<int>(Expressions::Type::SIZE))
        {
            next = 0;
        }
        selectedExpression = static_cast<Expressions::Type>(next);
        return selectedExpression;
    }
    Expressions::Type ExpressionManager::selectPreviousExpression()
    {
        int prev = static_cast<int>(selectedExpression) - 1;
        if (prev < 0)
        {
            prev = static_cast<int>(Expressions::Type::SIZE) - 1;
        }
        selectedExpression = static_cast<Expressions::Type>(prev);
        return selectedExpression;
    }
    void ExpressionManager::selectExpression(Expressions::Type type)
    {
        selectedExpression = type;
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
