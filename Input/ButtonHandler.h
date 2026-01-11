#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#define MAX_BUTTONS 3 // Maximum number of buttons supported
#define MAX_ACTIONS 12 // Maximum number of action handlers (3 buttons × 4 events)

#include <cstdint>
#include <functional>

class ButtonHandler {
public:
    enum class ButtonEvent {
        Tap,
        DoubleTap,
        Hold,
        Release
    };

    using ActionCallback = std::function<void()>;

    ButtonHandler(uint32_t doubleTapThreshold = 300, uint32_t holdThreshold = 500);

    // Initialize button pins
    void begin(uint8_t pin1, uint8_t pin2, uint8_t pin3);
    
    // Register an action for a specific button pin and event
    bool registerAction(uint8_t buttonPin, ButtonEvent event, ActionCallback action);
    
    // Main update loop - reads button states and triggers callbacks
    void update(uint32_t deltaTime);
    
    // Read physical button states (call from ISR or main loop)
    void readButtons();
    
    bool isButtonHeld(uint8_t buttonPin) const;
    uint8_t getHeldButtons() const;

private:
    struct ButtonState {
        uint8_t pin = 0xFF; // Pin number (0xFF = unregistered)
        bool isPressed = false; // Physical state
        bool lastPhysicalState = false; // For edge detection
        uint32_t pressTime = 0; // Time the button has been pressed
        uint32_t lastReleaseTime = 0;
        bool holdTriggered = false;
        uint32_t lastChangeTime = 0;
    };
    
    struct Action {
        uint8_t buttonPin = 0xFF;
        ButtonEvent event;
        ActionCallback callback;
    };

    ButtonState buttons[MAX_BUTTONS] = {};
    Action actions[MAX_ACTIONS] = {};
    uint32_t doubleTapThreshold;
    uint32_t holdThreshold;
    uint32_t debounceDelay;

    // Find button state by pin number
    ButtonState* findButton(uint8_t buttonPin);
    const ButtonState* findButton(uint8_t buttonPin) const;
    
    // Trigger actions for a specific button and event
    void triggerActions(uint8_t buttonPin, ButtonEvent event);

    void checkDoubleTap(ButtonState& state);
    void checkHold(ButtonState& state, uint32_t deltaTime);
    void processButtonEdges(ButtonState& state);
};

#endif // BUTTON_HANDLER_H
