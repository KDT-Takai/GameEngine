#include "pch/pch.h"
#include "Framework.hpp"
#include "System//ImGui/ImGuiManager.hpp"

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
		Engine::Graphics::DX12Renderer::Create();
		if (!Engine::Graphics::DX12Renderer::Get().Initialize(window->GetHWnd(), width, height))
		{
			return false;
		}
		Engine::Graphics::DX12DescriptorHeapManager::Create();
		if (!Engine::Graphics::DX12DescriptorHeapManager::Get().Initialize(Engine::Graphics::DX12Device::Get().GetDevice().Get(), 512))
		{
			return false;
		}

		Engine::System::ImGuiManager::Create();
		if (!Engine::System::ImGuiManager::Get().Initialize(
			*window,
			Engine::Graphics::DX12Device().Get(),
			*Engine::Graphics::DX12Renderer().Get().GetContext(),
			Engine::Graphics::DX12DescriptorHeapManager::Get()
		))
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