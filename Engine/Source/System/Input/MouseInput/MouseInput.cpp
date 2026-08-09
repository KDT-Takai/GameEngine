// Source/System/Input/MouseInput/MouseInput.cpp
#include "pch/pch.h"
#include "MouseInput.hpp"
#include "System/Screen/Screen.hpp"
#include "Utility/EngineContext/EngineContext.hpp"
#include "System/Widnow/Window.hpp"

namespace Engine::System::Input
{
    MouseInput::MouseInput()
    {
        nowInput.fill(false);
        prevInput.fill(false);
    }

    void MouseInput::Update(HWND hwnd)
    {
        // 前回の状態を保存
        prevInput = nowInput;

        // ボタン状態の更新
        nowInput[static_cast<int>(MouseButton::Left)] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        nowInput[static_cast<int>(MouseButton::Right)] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        nowInput[static_cast<int>(MouseButton::Middle)] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
        nowInput[static_cast<int>(MouseButton::X1)] = (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
        nowInput[static_cast<int>(MouseButton::X2)] = (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0;

        // スクリーン座標の更新
        POINT pos{};
        GetCursorPos(&pos);
        ScreenToClient(hwnd, &pos);
        screenPosition = { static_cast<float>(pos.x), static_cast<float>(pos.y) };
    }

    bool MouseInput::IsPress(MouseButton button) const
    {
        int i = static_cast<int>(button);
        return nowInput[i] && !prevInput[i];
    }

    bool MouseInput::IsHold(MouseButton button) const
    {
        return nowInput[static_cast<int>(button)];
    }

    bool MouseInput::IsRelease(MouseButton button) const
    {
        int i = static_cast<int>(button);
        return !nowInput[i] && prevInput[i];
    }

    DirectX::XMFLOAT2 MouseInput::GetScreenPosition() const
    {
        return screenPosition;
    }

    DirectX::XMFLOAT2 MouseInput::GetVirtualPosition() const
    {
        // EngineContext から Window を取得してウィンドウサイズを取得
        auto* window = GET_CONTEXT(Engine::System::Window);
        if (!window) return { 0.0f, 0.0f };

        float scaleX = Screen::GetVirtualWidth() / static_cast<float>(window->GetWidth());
        float scaleY = Screen::GetVirtualHeight() / static_cast<float>(window->GetHeight());

        // 仮想座標は中央原点なので変換
        float vx = screenPosition.x * scaleX - Screen::GetVirtualWidth() * 0.5f;
        float vy = -(screenPosition.y * scaleY - Screen::GetVirtualHeight() * 0.5f);

        return { vx, vy };
    }

    float MouseInput::GetScrollDelta() const
    {
        return scrollDelta;
    }

    void MouseInput::RenderImGui()
    {
    }
} // Engine::System::Input