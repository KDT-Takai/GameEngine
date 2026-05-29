#include "Framework.hpp"

namespace Engine::System
{
	bool Framework::Initialize(int width, int height, const char* title)
	{
		Engine::Utility::Logger::Create();
		window = std::make_unique<Window>();
		bool InitializeWindowFlag = window->Initialize(std::wstring(title, title + strlen(title)).c_str(), width, height);
		if (!InitializeWindowFlag)
		{
			return false;
		}
		return true;
	}

	void Framework::Run()
	{
		while (true)
		{
			if (!window->ProcessMessage())
			{
				break;
			}
		}
	}

	void Framework::Finalize()
	{
		Engine::Utility::Logger::Delete();
	}
}