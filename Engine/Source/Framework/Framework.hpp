#pragma once
#include <memory>
#include "System/Widnow/Window.hpp"

#include "Utility/Singleton/Singleton.hpp"

class Window;

class Framework : public Singleton<Framework>{
	DECLARE_SINGLETON(Framework)
public:
	bool Initialize(int width, int height, const char* title);
	void Run();
	void Finalize();
private:
	// ウィンドウ
	std::unique_ptr<Window> window;
};