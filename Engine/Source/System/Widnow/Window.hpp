#pragma once
#define NOMINMAX	// minの定義を抑制するためのマクロ
#include <windows.h>	// Windows APIのヘッダーファイル

// ウィンドウの生成
namespace Engine::System
{
	class Window
	{
	public:
		Window();
		~Window();
		// 初期化
		bool Initialize(const wchar_t* title, LONG width, LONG height);
		// メッセージ処理
		bool ProcessMessage();
		// ゲッター
		HWND GetHWnd() const { return m_hwnd; }
		HINSTANCE GetHInstance() const { return m_wndClass.hInstance; }
		LONG GetWidth()  const { return m_width; }
		LONG GetHeight() const { return m_height; }
	private:
		// ウィンドウクラスの登録
		bool RegisterWindowClass();
		// ウィンドウの生成
		bool CreateNativeWindow(const wchar_t* title, LONG width, LONG height);
	private:
		HWND m_hwnd = nullptr;		// ウィンドウハンドル
		WNDCLASSEX m_wndClass = {};	// ウィンドウクラス
		LONG m_width = 0;
		LONG m_height = 0;
	};
} // Engine::System