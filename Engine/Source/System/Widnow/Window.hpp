#pragma once
#define NOMINMAX	// minの定義を抑制するためのマクロ
#include <windows.h>	// Windows APIのヘッダーファイル
#include "Utility/Singleton/Singleton.hpp"

// ウィンドウの生成
class Window : public Singleton<Window>
{
	DECLARE_SINGLETON(Window)
private:
	Window();
	~Window();
public:
	// 初期化

	// 

};