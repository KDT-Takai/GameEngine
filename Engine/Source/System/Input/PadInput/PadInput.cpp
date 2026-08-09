#include "pch/pch.h"
#include "PadInput.hpp"

namespace Engine::System::Input
{
    PadInput::PadInput()
    {
        m_currentReading = {};
        m_previousReading = {};

        m_addedToken = winrt::Windows::Gaming::Input::Gamepad::GamepadAdded(
            { this, &PadInput::OnGamepadAdded });
        m_removedToken = winrt::Windows::Gaming::Input::Gamepad::GamepadRemoved(
            { this, &PadInput::OnGamepadRemoved });

        if (winrt::Windows::Gaming::Input::Gamepad::Gamepads().Size() > 0)
        {
            m_gamepad = winrt::Windows::Gaming::Input::Gamepad::Gamepads().GetAt(0);
        }
    }

    PadInput::~PadInput()
    {
        winrt::Windows::Gaming::Input::Gamepad::GamepadAdded(m_addedToken);
        winrt::Windows::Gaming::Input::Gamepad::GamepadRemoved(m_removedToken);
    }

    void PadInput::Update()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_gamepad)
        {
            m_previousReading = m_currentReading;
            m_currentReading = m_gamepad.GetCurrentReading();
        }
        else
        {
            m_currentReading = {};
            m_previousReading = {};
        }
    }

    bool PadInput::IsConnected() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_gamepad != nullptr;
    }

    bool PadInput::IsPress(PadButton button) const
    {
        auto btn = ToGamepadButton(button);
        return ((m_currentReading.Buttons & btn) == btn) &&
            ((m_previousReading.Buttons & btn) != btn);
    }

    bool PadInput::IsHold(PadButton button) const
    {
        auto btn = ToGamepadButton(button);
        return (m_currentReading.Buttons & btn) == btn;
    }

    bool PadInput::IsRelease(PadButton button) const
    {
        auto btn = ToGamepadButton(button);
        return ((m_currentReading.Buttons & btn) != btn) &&
            ((m_previousReading.Buttons & btn) == btn);
    }

    float PadInput::GetLeftStickX() const
    {
        return ApplyDeadzone(
            static_cast<float>(m_currentReading.LeftThumbstickX), DeadzoneThreshold);
    }

    float PadInput::GetLeftStickY() const
    {
        return ApplyDeadzone(
            static_cast<float>(m_currentReading.LeftThumbstickY), DeadzoneThreshold);
    }

    float PadInput::GetRightStickX() const
    {
        return ApplyDeadzone(
            static_cast<float>(m_currentReading.RightThumbstickX), DeadzoneThreshold);
    }

    float PadInput::GetRightStickY() const
    {
        return ApplyDeadzone(
            static_cast<float>(m_currentReading.RightThumbstickY), DeadzoneThreshold);
    }

    float PadInput::GetLeftTrigger() const
    {
        return static_cast<float>(m_currentReading.LeftTrigger);
    }

    float PadInput::GetRightTrigger() const
    {
        return static_cast<float>(m_currentReading.RightTrigger);
    }

    void PadInput::SetVibration(float leftMotor, float rightMotor)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_gamepad)
        {
            winrt::Windows::Gaming::Input::GamepadVibration vibration;
            vibration.LeftMotor = leftMotor;
            vibration.RightMotor = rightMotor;
            m_gamepad.Vibration(vibration);
        }
    }

    float PadInput::ApplyDeadzone(float value, float deadzone) const
    {
        return std::abs(value) < deadzone ? 0.0f : value;
    }

    winrt::Windows::Gaming::Input::GamepadButtons
        PadInput::ToGamepadButton(PadButton button) const
    {
        using GB = winrt::Windows::Gaming::Input::GamepadButtons;
        switch (button)
        {
        case PadButton::A:         return GB::A;
        case PadButton::B:         return GB::B;
        case PadButton::X:         return GB::X;
        case PadButton::Y:         return GB::Y;
        case PadButton::LB:        return GB::LeftShoulder;
        case PadButton::RB:        return GB::RightShoulder;
        case PadButton::LS:        return GB::LeftThumbstick;
        case PadButton::RS:        return GB::RightThumbstick;
        case PadButton::DPadUp:    return GB::DPadUp;
        case PadButton::DPadDown:  return GB::DPadDown;
        case PadButton::DPadLeft:  return GB::DPadLeft;
        case PadButton::DPadRight: return GB::DPadRight;
        case PadButton::View:      return GB::View;
        case PadButton::Menu:      return GB::Menu;
        default:                   return GB::None;
        }
    }

    void PadInput::OnGamepadAdded(
        winrt::Windows::Foundation::IInspectable const&,
        winrt::Windows::Gaming::Input::Gamepad const& gamepad)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_gamepad)
        {
            m_gamepad = gamepad;
        }
    }

    void PadInput::OnGamepadRemoved(
        winrt::Windows::Foundation::IInspectable const&,
        winrt::Windows::Gaming::Input::Gamepad const& gamepad)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_gamepad == gamepad)
        {
            m_gamepad = nullptr;
            if (winrt::Windows::Gaming::Input::Gamepad::Gamepads().Size() > 0)
            {
                m_gamepad = winrt::Windows::Gaming::Input::Gamepad::Gamepads().GetAt(0);
            }
        }
    }

    void PadInput::RenderImGui()
    {
    }
} // Engine::System::Input