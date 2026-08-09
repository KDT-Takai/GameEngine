#pragma once
#include "System/Input/InputUtils/InputUtils.hpp"
#include <array>
#include <DirectXMath.h>
#include <Windows.h>

namespace Engine::System::Input
{
    class MouseInput
    {
    public:
        MouseInput();

        void Update(HWND hwnd);

        // 押した瞬間
        bool IsPress(MouseButton button) const;
        // 押し続けている
        bool IsHold(MouseButton button) const;
        // 離した瞬間
        bool IsRelease(MouseButton button) const;

        // スクリーン座標
        DirectX::XMFLOAT2 GetScreenPosition() const;

        // 仮想座標
        DirectX::XMFLOAT2 GetVirtualPosition() const;

        // スクロール量
        float GetScrollDelta() const;

        // ImGui デバッグ表示
        void RenderImGui();

    private:
        int ToVirtualKey(MouseButton button) const;

        static constexpr int ButtonCount = static_cast<int>(MouseButton::Count);
        std::array<bool, ButtonCount> nowInput;
        std::array<bool, ButtonCount> prevInput;

        DirectX::XMFLOAT2 screenPosition = { 0.0f, 0.0f };
        float             scrollDelta = 0.0f;
    };
} // Engine::System::Input