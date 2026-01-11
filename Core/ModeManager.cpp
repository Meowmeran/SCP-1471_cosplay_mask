#include "ModeManager.h"

namespace Core
{
    ModeManager::ModeManager()
        : currentMode(Mode::OFF)
    {
    }
    ModeManager::~ModeManager()
    {
    }
    void ModeManager::setMode(Mode mode)
    {
        currentMode = mode;
    }
    Mode ModeManager::getMode() const
    {
        return currentMode;
    }
    bool ModeManager::isActive() const
    {
        return currentMode == Mode::ACTIVE;
    }
    bool ModeManager::isManual() const
    {
        return currentMode == Mode::MANUAL;
    }
    bool ModeManager::isOff() const
    {
        return currentMode == Mode::OFF;
    }
    bool ModeManager::isError() const
    {
        return currentMode == Mode::ERROR;
    }

}
