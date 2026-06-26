#include "pch/pch.h"
#include "Framework.hpp"
#include "System//ImGui/ImGuiManager.hpp"
#include "Graphics/DX12/Render/DX12Renderer.hpp"

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
		Engine::Graphics::DX12Device::Create();
		if (!Engine::Graphics::DX12Device::Get().Initialize())
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
		// æ‚ÉDX12Device‚ğíœ‚µ‚Ä‚¨‚­
		Engine::Graphics::DX12Device::Get().Finalize();
		// DX12Device‚Ìíœ
		Engine::Graphics::DX12Device::Delete();
	}
}