#include "pch/pch.h"
#include "KeyInput.hpp"

namespace Engine::System::Input
{
    KeyInput::KeyInput()
    {
        nowInput.fill(false);
        prevInput.fill(false);
    }

    void KeyInput::Update()
    {
        prevInput = nowInput;
        for (int i = 0; i < KeyCount; i++)
        {
            int vk = ToVirtualKey(static_cast<KeyCode>(i));
            if (vk == 0)
            {
                nowInput[i] = false;
                continue;
            }
            nowInput[i] = (GetAsyncKeyState(vk) & 0x8000) != 0;
        }
    }

    bool KeyInput::IsPress(KeyCode key) const
    {
        int i = static_cast<int>(key);
        return nowInput[i] && !prevInput[i];
    }

    bool KeyInput::IsHold(KeyCode key) const
    {
        return nowInput[static_cast<int>(key)];
    }

    bool KeyInput::IsRelease(KeyCode key) const
    {
        int i = static_cast<int>(key);
        return !nowInput[i] && prevInput[i];
    }

    int KeyInput::ToVirtualKey(KeyCode key) const
    {
        switch (key)
        {
            // アルファベット
        case KeyCode::A: return 'A';
        case KeyCode::B: return 'B';
        case KeyCode::C: return 'C';
        case KeyCode::D: return 'D';
        case KeyCode::E: return 'E';
        case KeyCode::F: return 'F';
        case KeyCode::G: return 'G';
        case KeyCode::H: return 'H';
        case KeyCode::I: return 'I';
        case KeyCode::J: return 'J';
        case KeyCode::K: return 'K';
        case KeyCode::L: return 'L';
        case KeyCode::M: return 'M';
        case KeyCode::N: return 'N';
        case KeyCode::O: return 'O';
        case KeyCode::P: return 'P';
        case KeyCode::Q: return 'Q';
        case KeyCode::R: return 'R';
        case KeyCode::S: return 'S';
        case KeyCode::T: return 'T';
        case KeyCode::U: return 'U';
        case KeyCode::V: return 'V';
        case KeyCode::W: return 'W';
        case KeyCode::X: return 'X';
        case KeyCode::Y: return 'Y';
        case KeyCode::Z: return 'Z';

            // 数字
        case KeyCode::Num0: return '0';
        case KeyCode::Num1: return '1';
        case KeyCode::Num2: return '2';
        case KeyCode::Num3: return '3';
        case KeyCode::Num4: return '4';
        case KeyCode::Num5: return '5';
        case KeyCode::Num6: return '6';
        case KeyCode::Num7: return '7';
        case KeyCode::Num8: return '8';
        case KeyCode::Num9: return '9';

            // ファンクションキー
        case KeyCode::F1:  return VK_F1;
        case KeyCode::F2:  return VK_F2;
        case KeyCode::F3:  return VK_F3;
        case KeyCode::F4:  return VK_F4;
        case KeyCode::F5:  return VK_F5;
        case KeyCode::F6:  return VK_F6;
        case KeyCode::F7:  return VK_F7;
        case KeyCode::F8:  return VK_F8;
        case KeyCode::F9:  return VK_F9;
        case KeyCode::F10: return VK_F10;
        case KeyCode::F11: return VK_F11;
        case KeyCode::F12: return VK_F12;

            // 矢印キー
        case KeyCode::Up:    return VK_UP;
        case KeyCode::Down:  return VK_DOWN;
        case KeyCode::Left:  return VK_LEFT;
        case KeyCode::Right: return VK_RIGHT;

            // 修飾キー
        case KeyCode::LShift: return VK_LSHIFT;
        case KeyCode::RShift: return VK_RSHIFT;
        case KeyCode::LCtrl:  return VK_LCONTROL;
        case KeyCode::RCtrl:  return VK_RCONTROL;
        case KeyCode::LAlt:   return VK_LMENU;
        case KeyCode::RAlt:   return VK_RMENU;
        case KeyCode::LWin:   return VK_LWIN;
        case KeyCode::RWin:   return VK_RWIN;

            // テンキー
        case KeyCode::Numpad0:      return VK_NUMPAD0;
        case KeyCode::Numpad1:      return VK_NUMPAD1;
        case KeyCode::Numpad2:      return VK_NUMPAD2;
        case KeyCode::Numpad3:      return VK_NUMPAD3;
        case KeyCode::Numpad4:      return VK_NUMPAD4;
        case KeyCode::Numpad5:      return VK_NUMPAD5;
        case KeyCode::Numpad6:      return VK_NUMPAD6;
        case KeyCode::Numpad7:      return VK_NUMPAD7;
        case KeyCode::Numpad8:      return VK_NUMPAD8;
        case KeyCode::Numpad9:      return VK_NUMPAD9;
        case KeyCode::NumpadAdd:      return VK_ADD;
        case KeyCode::NumpadSubtract: return VK_SUBTRACT;
        case KeyCode::NumpadMultiply: return VK_MULTIPLY;
        case KeyCode::NumpadDivide:   return VK_DIVIDE;
        case KeyCode::NumpadDecimal:  return VK_DECIMAL;
        case KeyCode::NumpadEnter:    return VK_RETURN; // テンキーEnterはVK_RETURNと同じ
        case KeyCode::NumLock:        return VK_NUMLOCK;

            // 特殊キー
        case KeyCode::Space:       return VK_SPACE;
        case KeyCode::Enter:       return VK_RETURN;
        case KeyCode::Escape:      return VK_ESCAPE;
        case KeyCode::Backspace:   return VK_BACK;
        case KeyCode::Tab:         return VK_TAB;
        case KeyCode::Delete:      return VK_DELETE;
        case KeyCode::Insert:      return VK_INSERT;
        case KeyCode::Home:        return VK_HOME;
        case KeyCode::End:         return VK_END;
        case KeyCode::PageUp:      return VK_PRIOR;
        case KeyCode::PageDown:    return VK_NEXT;
        case KeyCode::CapsLock:    return VK_CAPITAL;
        case KeyCode::PrintScreen: return VK_SNAPSHOT;
        case KeyCode::ScrollLock:  return VK_SCROLL;
        case KeyCode::Pause:       return VK_PAUSE;

            // 記号
        case KeyCode::Semicolon:  return VK_OEM_1;
        case KeyCode::Apostrophe: return VK_OEM_7;
        case KeyCode::Grave:      return VK_OEM_3;
        case KeyCode::Comma:      return VK_OEM_COMMA;
        case KeyCode::Period:     return VK_OEM_PERIOD;
        case KeyCode::Slash:      return VK_OEM_2;
        case KeyCode::Backslash:  return VK_OEM_5;
        case KeyCode::LBracket:   return VK_OEM_4;
        case KeyCode::RBracket:   return VK_OEM_6;
        case KeyCode::Minus:      return VK_OEM_MINUS;
        case KeyCode::Equal:      return VK_OEM_PLUS;

            // ナビゲーション
        case KeyCode::Menu: return VK_APPS;

        default: return 0;
        }
    }

    void KeyInput::RenderImGui()
    {
    }
} // Engine::System::Input