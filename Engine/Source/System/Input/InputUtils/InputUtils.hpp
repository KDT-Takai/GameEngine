#pragma once

namespace Engine::System::Input
{
    // キーコード
    enum class KeyCode
    {
        // アルファベット
        A, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

        // 数字
        Num0, Num1, Num2, Num3, Num4,
        Num5, Num6, Num7, Num8, Num9,

        // ファンクションキー
        F1, F2, F3, F4, F5, F6,
        F7, F8, F9, F10, F11, F12,

        // 矢印キー
        Up, Down, Left, Right,

        // 修飾キー
        LShift, RShift,
        LCtrl, RCtrl,
        LAlt, RAlt,
        LWin, RWin,

        // テンキー
        Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
        Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
        NumpadAdd,      // +
        NumpadSubtract, // -
        NumpadMultiply, // *
        NumpadDivide,   // /
        NumpadDecimal,  // .
        NumpadEnter,    // Enter
        NumLock,

        // 特殊キー
        Space,
        Enter,
        Escape,
        Backspace,
        Tab,
        Delete,
        Insert,
        Home,
        End,
        PageUp,
        PageDown,
        CapsLock,
        PrintScreen,
        ScrollLock,
        Pause,

        // 記号
        Semicolon,    // ;
        Apostrophe,   // '
        Grave,        // `
        Comma,        // ,
        Period,       // .
        Slash,        // /
        Backslash,    // '\'
        LBracket,     // [
        RBracket,     // ]
        Minus,        // -
        Equal,        // =

        // ナビゲーション
        Menu,

        Count // キー総数
    };

    // マウスボタン
    enum class MouseButton
    {
        Left,
        Right,
        Middle,
        X1,   // サイドボタン1
        X2,   // サイドボタン2

        Count
    };

    // パッドボタン
    enum class PadButton
    {
        A,
        B,
        X,
        Y,
        LB,   // 左バンパー
        RB,   // 右バンパー
        LT,   // 左トリガー（ボタンとして）
        RT,   // 右トリガー（ボタンとして）
        LS,   // 左スティック押し込み
        RS,   // 右スティック押し込み
        DPadUp,
        DPadDown,
        DPadLeft,
        DPadRight,
        View, // Xboxのビューボタン
        Menu, // Xboxのメニューボタン

        Count
    };
} // Engine::System::Input