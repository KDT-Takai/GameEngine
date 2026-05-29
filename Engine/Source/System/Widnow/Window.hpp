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
		HWND GetHwnd() const { return m_hwnd; }
		HINSTANCE GetHInstance() const { return m_wndClass.hInstance; }
	private:
		// ウィンドウクラスの登録
		bool RegisterWindowClass();
		// ウィンドウの生成
		bool CreateNativeWindow(const wchar_t* title, LONG width, LONG height);
	private:
		HWND m_hwnd = nullptr;		// ウィンドウハンドル
		WNDCLASSEX m_wndClass = {};	// ウィンドウクラス
	};
}