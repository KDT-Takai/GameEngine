#include "pch/pch.h"
#include "Framework.hpp"
#include "System/ImGui/ImGuiManager.hpp"

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
		if (!DX12Initialize(width, height))
		{
			return false;
		}
		ImGuiManager::Get().AddDebugUI([]() {
			ImGui::Begin("ImGui");
			ImGui::Text("Test");
			ImGui::End();
			}, "test");
		return true;
	}

	void Framework::Run()
	{
		auto& renderer = Engine::Graphics::DX12Renderer::Get();
		auto* context = renderer.GetContext();
		auto& imgui = Engine::System::ImGuiManager::Get();

		while (true)
		{
			if (!window->ProcessMessage())
			{
				break;
			}

			// フレーム開始
			if (!renderer.BeginFrame())
			{
				break;
			}

			// ImGuiフレーム開始
			imgui.NewFrame();

			// レンダーターゲットのクリアと設定
			context->SetRenderTarget();
			context->ClearRenderTarget();

			// ImGuiのUI更新
			imgui.Update();

			// ImGuiの描画
			imgui.EndFrame();

			// フレーム終了
			if (!renderer.EndFrame())
			{
				break;
			}

		}
	}

	void Framework::Finalize()
	{
		DX12Finalize();
		// 先にDX12Deviceを削除しておく
		Engine::Graphics::DX12Device::Get().Finalize();
		Engine::Graphics::DX12Device::Delete();

		Engine::Utility::Logger::Delete();
	}

	bool Framework::DX12Initialize(int width, int height)
	{
		// デバイス
		Engine::Graphics::DX12Device::Create();
		if (!Engine::Graphics::DX12Device::Get().Initialize())
		{
			return false;
		}
		// レンダラー
		Engine::Graphics::DX12Renderer::Create();
		if (!Engine::Graphics::DX12Renderer::Get().Initialize(window->GetHWnd(), width, height))
		{
			return false;
		}
		// ディスクリプタヒープマネージャ
		Engine::Graphics::DX12DescriptorHeapManager::Create();
		if (!Engine::Graphics::DX12DescriptorHeapManager::Get().Initialize(Engine::Graphics::DX12Device::Get().GetDevice().Get(), 512))
		{
			return false;
		}
		// ImGui
		Engine::System::ImGuiManager::Create();
		if (!Engine::System::ImGuiManager::Get().Initialize(
			*window,
			Engine::Graphics::DX12Device::Get(),
			*Engine::Graphics::DX12Renderer::Get().GetContext(),
			Engine::Graphics::DX12DescriptorHeapManager::Get()
		))
		{
			return false;
		}
		return true;
	}

	void Framework::DX12Finalize()
	{
		Engine::System::ImGuiManager::Get().Finalize();
		Engine::System::ImGuiManager::Delete();
		Engine::Graphics::DX12DescriptorHeapManager::Get().Finalize();
		Engine::Graphics::DX12DescriptorHeapManager::Delete();
		Engine::Graphics::DX12Renderer::Get().Finalize();
		Engine::Graphics::DX12Renderer::Delete();
	}
}