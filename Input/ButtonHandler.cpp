#include "ButtonHandler.h"
#include <Arduino.h>

ButtonHandler::ButtonHandler(uint32_t doubleTapThreshold, uint32_t holdThreshold)
    : doubleTapThreshold(doubleTapThreshold), holdThreshold(holdThreshold), debounceDelay(50)
{
}

void ButtonHandler::begin(uint8_t pin1, uint8_t pin2, uint8_t pin3)
{
    uint8_t pins[] = {pin1, pin2, pin3};

    for (int i = 0; i < MAX_BUTTONS; ++i)
    {
        buttons[i].pin = pins[i];
        pinMode(pins[i], INPUT_PULLUP);
    }
}

bool ButtonHandler::registerAction(uint8_t buttonPin, ButtonEvent event, ActionCallback action)
{
    // Find first available action slot
    for (int i = 0; i < MAX_ACTIONS; ++i)
    {
        if (actions[i].buttonPin == 0xFF)
        {
            actions[i].buttonPin = buttonPin;
            actions[i].event = event;
            actions[i].callback = action;
            return true;
        }
    }
    return false; // No available slots
}

void ButtonHandler::triggerActions(uint8_t buttonPin, ButtonEvent event)
{
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

void ButtonHandler::readButtons()
{
    for (int i = 0; i < MAX_BUTTONS; ++i)
    {
        ButtonState &state = buttons[i];

        if (state.pin == 0xFF) // Unregistered
            continue;

        // Read physical state (LOW = pressed for INPUT_PULLUP)
        bool physicalState = digitalRead(state.pin) == LOW;

        // Detect edges
        if (physicalState != state.lastPhysicalState)
        {
            state.lastChangeTime = 0; // Start debounce timer
        }
        else if (state.lastChangeTime < debounceDelay)
        {
            // Still debouncing, skip processing
            return;
        }
        else if (physicalState != state.lastPhysicalState)
        {
            state.lastPhysicalState = physicalState;

            if (physicalState) // Button pressed
            {
                state.isPressed = true;
                state.pressTime = 0;
                state.holdTriggered = false;
            }
            else // Button released
            {
                if (state.isPressed)
                {
                    state.isPressed = false;

                    if (!state.holdTriggered && state.pressTime < holdThreshold)
                    {
                        triggerActions(state.pin, ButtonEvent::Tap);
                        checkDoubleTap(state);
                    }

                    triggerActions(state.pin, ButtonEvent::Release);
                    state.lastReleaseTime = 0;
                    state.pressTime = 0;
                }
            }
        }
    }
}
ButtonHandler::ButtonState *ButtonHandler::findButton(uint8_t buttonPin)
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
const ButtonHandler::ButtonState *ButtonHandler::findButton(uint8_t buttonPin) const
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

void ButtonHandler::update(uint32_t deltaTime)
{
    readButtons();

    for (int i = 0; i < MAX_BUTTONS; ++i)
    {
        ButtonState &state = buttons[i];

        // Skip unregistered slots
        if (state.pin == 0xFF)
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
            }
        }
    }
}

void ButtonHandler::checkDoubleTap(ButtonState &state)
{
    if (state.lastReleaseTime > 0 && state.lastReleaseTime < doubleTapThreshold)
    {
        triggerActions(state.pin, ButtonEvent::DoubleTap);
        state.lastReleaseTime = 0;
    }
    else
    {
        state.lastReleaseTime = 0;
    }
}

void ButtonHandler::checkHold(ButtonState &state, uint32_t deltaTime)
{
    if (!state.holdTriggered && state.pressTime >= holdThreshold)
    {
        triggerActions(state.pin, ButtonEvent::Hold);
        state.holdTriggered = true;
    }
}

bool ButtonHandler::isButtonHeld(uint8_t buttonPin) const
{
    const ButtonState *state = findButton(buttonPin);
    if (!state)
        return false;
    return state->isPressed && state->pressTime >= holdThreshold;
}

uint8_t ButtonHandler::getHeldButtons() const
{
    // Return a bitmask of held buttons 
    // Bit 0 = Button 0, Bit 1 = Button 1, etc.
    // Assumes MAX_BUTTONS <= 8    
    if (MAX_BUTTONS > 8)
    {
        Serial.println("getHeldButtons() supports up to 8 buttons only.");
        return 0; // Unsupported
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
