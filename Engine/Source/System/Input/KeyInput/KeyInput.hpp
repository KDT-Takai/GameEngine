#pragma once
#include "System/Input/InputUtils/InputUtils.hpp"
#include <array>

namespace Engine::System::Input
{
    class KeyInput
    {
    public:
        KeyInput();

        void Update();

        // 押した瞬間
        bool IsPress(KeyCode key) const;
        // 押し続けている
        bool IsHold(KeyCode key) const;
        // 離した瞬間
        bool IsRelease(KeyCode key) const;

        // ImGui デバッグ表示
        void RenderImGui();

    private:
        // KeyCode を仮想キーコードに変換
        int ToVirtualKey(KeyCode key) const;

        static constexpr int KeyCount = static_cast<int>(KeyCode::Count);
        std::array<bool, KeyCount> nowInput;
        std::array<bool, KeyCount> prevInput;
    };
} // Engine::System::Input