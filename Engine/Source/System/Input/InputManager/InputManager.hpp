#pragma once
#include "Utility/Singleton/Singleton.hpp"
#include "System/Input/KeyInput/KeyInput.hpp"
#include "System/Input/MouseInput/MouseInput.hpp"
#include "System/Input/PadInput/PadInput.hpp"

namespace Engine::System::Input
{
    class InputManager : public Engine::Utility::Singleton<InputManager>
    {
        DECLARE_SINGLETON(InputManager)
    public:
        bool Initialize(HWND hwnd);
        void Finalize();
        void Update();

        // 各入力の取得
        KeyInput& GetKeyInput() { return *keyInput; }
        MouseInput& GetMouseInput() { return *mouseInput; }
        PadInput& GetPadInput() { return *padInput; }

        // ImGui デバッグ表示
        void RenderImGui();

    private:
        HWND hwnd = nullptr;

        std::unique_ptr<KeyInput>   keyInput;
        std::unique_ptr<MouseInput> mouseInput;
        std::unique_ptr<PadInput>   padInput;
    };
} // Engine::System::Input