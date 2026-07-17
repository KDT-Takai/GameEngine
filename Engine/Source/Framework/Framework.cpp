#include "pch/pch.h"
#include "Framework.hpp"
#include "System/ImGui/ImGuiManager.hpp"
#include "Utility/EngineContext/EngineContext.hpp"

namespace Engine::System
{
	bool Framework::Initialize(int width, int height, const char* title)
	{
		Engine::Utility::Logger::Create();

		// EngineContext の生成（最初に作る）
		Engine::Utility::EngineContext::Create();

		wchar_t currentDir[MAX_PATH];
		GetCurrentDirectoryW(MAX_PATH, currentDir);
		std::wstring wdir(currentDir);
		LOG_INFO("作業ディレクトリ: " + std::string(wdir.begin(), wdir.end()));

		// ウィンドウの生成
		window = std::make_unique<Window>();
		bool InitializeWindowFlag = window->Initialize(std::wstring(title, title + strlen(title)).c_str(), width, height);
		if (!InitializeWindowFlag)
		{
			return false;
		}
		// DX12の初期化
		if (!DX12Initialize(width, height))
		{
			return false;
		}
		// ImGuiのテスト
		ImGuiManager::GetInstance().AddDebugUI([]() {
			ImGui::Begin("ImGui");
			ImGui::Text("Test");
			ImGui::End();
			}, "test");
		return true;
	}

	void Framework::Run()
	{
		auto& renderer = Engine::Graphics::DX12Renderer::GetInstance();
		auto* context = renderer.GetContext();
		auto& imgui = Engine::System::ImGuiManager::GetInstance();

		float angle = 0.0f;

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

			angle += 0.01f;
			auto wvp = DirectX::XMMatrixRotationZ(angle);
			triangle->SetWVP(wvp);
			triangle->Draw(context->GetCmdList().Get());

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
		Engine::Graphics::DX12Device::GetInstance().Finalize();
		Engine::Graphics::DX12Device::Delete();

		Engine::Utility::Logger::Delete();
	}

	bool Framework::DX12Initialize(int width, int height)
	{
		// デバイス
		Engine::Graphics::DX12Device::Create();
		if (!Engine::Graphics::DX12Device::GetInstance().Initialize())
		{
			return false;
		}
		// レンダラー
		Engine::Graphics::DX12Renderer::Create();
		if (!Engine::Graphics::DX12Renderer::GetInstance().Initialize(window->GetHWnd(), width, height))
		{
			return false;
		}
		// EngineContextに登録
		Engine::Graphics::DX12Renderer::GetInstance().GetContext()->AddContext();
		// ディスクリプタヒープマネージャ
		Engine::Graphics::DX12DescriptorHeapManager::Create();
		if (!Engine::Graphics::DX12DescriptorHeapManager::GetInstance().Initialize(Engine::Graphics::DX12Device::GetInstance().GetDevice().Get(), 512))
		{
			return false;
		}
		// ImGui
		Engine::System::ImGuiManager::Create();
		if (!Engine::System::ImGuiManager::GetInstance().Initialize(
			*window,
			Engine::Graphics::DX12Device::GetInstance(),
			*Engine::Graphics::DX12Renderer::GetInstance().GetContext(),
			Engine::Graphics::DX12DescriptorHeapManager::GetInstance()
		))
		{
			return false;
		}
		// シェーダローダー・三角形
		auto* device = Engine::Graphics::DX12Device::GetInstance().GetDevice().Get();
		auto* context = Engine::Graphics::DX12Renderer::GetInstance().GetContext();

		shaderLoader = std::make_unique<Engine::Graphics::RuntimeShaderLoader>();
		triangle = std::make_unique<Engine::Graphics::Triangle>();

		if (!triangle->Initialize(
			device,
			*shaderLoader,
			context->GetBackBufferFormat(),
			context->GetDepthBufferFormat()
		))
		{
			LOG_ERROR("Triangleの初期化に失敗");
			return false;
		}

		return true;
	}

	void Framework::DX12Finalize()
	{
		triangle->Finalize();
		triangle.reset();
		shaderLoader.reset();

		Engine::System::ImGuiManager::GetInstance().Finalize();
		Engine::System::ImGuiManager::Delete();
		Engine::Graphics::DX12DescriptorHeapManager::GetInstance().Finalize();
		Engine::Graphics::DX12DescriptorHeapManager::Delete();
		Engine::Graphics::DX12Renderer::GetInstance().Finalize();
		Engine::Graphics::DX12Renderer::Delete();
	}
}