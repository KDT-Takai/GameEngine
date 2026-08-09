#include "pch/pch.h"
#include "InputManager.hpp"

namespace Engine::System::Input
{
    bool InputManager::Initialize(HWND hwnd)
    {
        this->hwnd = hwnd;

        keyInput = std::make_unique<KeyInput>();
        mouseInput = std::make_unique<MouseInput>();
        padInput = std::make_unique<PadInput>();

        LOG_INFO("InputManager‚Ì‰Šú‰»‚É¬Œ÷");
        return true;
    }

    void InputManager::Finalize()
    {
        padInput.reset();
        mouseInput.reset();
        keyInput.reset();
    }

    void InputManager::Update()
    {
        keyInput->Update();
        mouseInput->Update(hwnd);
        padInput->Update();
    }

    void InputManager::RenderImGui()
    {
        keyInput->RenderImGui();
        mouseInput->RenderImGui();
        padInput->RenderImGui();
    }
} // Engine::System::Input