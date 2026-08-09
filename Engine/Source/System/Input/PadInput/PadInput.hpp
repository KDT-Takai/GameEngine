#pragma once
#include "System/Input/InputUtils/InputUtils.hpp"
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <mutex>

namespace Engine::System::Input
{
    class PadInput
    {
    public:
        PadInput();
        ~PadInput();

        void Update();

        // 接続状態
        bool IsConnected() const;

        // ボタン入力
        bool IsPress(PadButton button) const;    // 押した瞬間
        bool IsHold(PadButton button) const;     // 押し続けている
        bool IsRelease(PadButton button) const;  // 離した瞬間

        // スティック
        float GetLeftStickX()  const;
        float GetLeftStickY()  const;
        float GetRightStickX() const;
        float GetRightStickY() const;

        // トリガー
        float GetLeftTrigger()  const;
        float GetRightTrigger() const;

        // バイブレーション
        void SetVibration(float leftMotor, float rightMotor);

        // ImGui デバッグ表示
        void RenderImGui();

    private:
        winrt::Windows::Gaming::Input::GamepadButtons ToGamepadButton(PadButton button) const;
        float ApplyDeadzone(float value, float deadzone) const;

        void OnGamepadAdded(
            winrt::Windows::Foundation::IInspectable const&,
            winrt::Windows::Gaming::Input::Gamepad const& gamepad);
        void OnGamepadRemoved(
            winrt::Windows::Foundation::IInspectable const&,
            winrt::Windows::Gaming::Input::Gamepad const& gamepad);

        winrt::Windows::Gaming::Input::Gamepad        m_gamepad = nullptr;
        winrt::Windows::Gaming::Input::GamepadReading m_currentReading{};
        winrt::Windows::Gaming::Input::GamepadReading m_previousReading{};

        winrt::event_token m_addedToken;
        winrt::event_token m_removedToken;

        mutable std::mutex m_mutex;

        static constexpr float DeadzoneThreshold = 0.2f;
    };
} // Engine::System::Input