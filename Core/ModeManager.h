#ifndef MODE_MANAGER_H
#define MODE_MANAGER_H

namespace Core
{

    enum class Mode
    {
        ACTIVE,
        MANUAL,
        OFF,
        ERROR
    };

    class ModeManager
    {
    public:
        ModeManager();
        ~ModeManager();

        void setMode(Mode mode);
        Mode getMode() const;

        bool isActive() const;
        bool isManual() const;
        bool isOff() const;
        bool isError() const;

    private:
        Mode currentMode;
    };

} // namespace Core

#endif // MODE_MANAGER_H
