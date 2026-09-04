#include "pch/pch.h"
#include "Framework.hpp"
#include "System/ImGui/ImGuiManager.hpp"
#include "Utility/EngineContext/EngineContext.hpp"

namespace Engine::System
{
	bool Framework::Initialize(int width, int height, const char* title)
	{
		Engine::Utility::Logger::Create();

		// EngineContextの生成
		Engine::Utility::EngineContext::Create();

		// 仮想解像度の設定
		Engine::System::Screen::SetVirtualSize(1920.0f, 1080.0f);

		wchar_t currentDir[MAX_PATH];
		GetCurrentDirectoryW(MAX_PATH, currentDir);
		std::wstring wdir(currentDir);
		LOG_INFO("作業ディレクトリ: " + std::string(wdir.begin(), wdir.end()));

		// ECSのレジストリ生成
		Engine::System::ECS::Registry::Create();

		// AssetManagerの生成
		Engine::System::Assets::AssetManager::Create();

		// ウィンドウの生成
		window = std::make_unique<Window>();
		bool InitializeWindowFlag = window->Initialize(std::wstring(title, title + strlen(title)).c_str(), width, height);
		if (!InitializeWindowFlag)
		{
			return false;
		}
		REGISTER_CONTEXT(*window);

		// DX12の初期化
		if (!DX12Initialize(width, height))
		{
			return false;
		}

		// 入力関係の初期化
		Engine::System::Input::InputManager::Create();
		if (!Engine::System::Input::InputManager::GetInstance().Initialize(window->GetHWnd()))
		{
			LOG_ERROR("InputManagerの初期化に失敗");
			return false;
		}

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
			// 入力の更新
			Engine::System::Input::InputManager::GetInstance().Update();

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

			auto cameraData = cameraSystem.GetMainCameraData(
				Engine::System::ECS::Registry::GetInstance().GetRegistry()
			);

			// Triangle（テスト用）
			//angle += 0.01f;
			//auto wvp = DirectX::XMMatrixRotationZ(angle);
			//triangle->SetWVP(wvp);
			//triangle->Draw(context->GetCmdList().Get());

			// スプライト描画
			spriteRenderSystem.Update(
				Engine::System::ECS::Registry::GetInstance().GetRegistry(),
				context->GetCmdList().Get(),
				*spriteRenderer,
				*textureManager,
				cameraData.view,
				cameraData.projection
			);

			// モデル描画
			modelRenderSystem.Update(
				Engine::System::ECS::Registry::GetInstance().GetRegistry(),
				context->GetCmdList().Get(),
				*modelRenderer,
				Engine::Graphics::ModelManager::GetInstance(),
				*textureManager,
				cameraData.view,
				cameraData.projection
			);

#ifdef _DEBUG
			entityInspector.Draw(Engine::System::ECS::Registry::GetInstance().GetRegistry());
			assetBrowser.Draw();
#endif

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
		Engine::System::Input::InputManager::GetInstance().Finalize();
		Engine::System::Input::InputManager::Delete();
		DX12Finalize();
		Engine::System::ECS::Registry::GetInstance().ForceAllClear();
		Engine::System::ECS::Registry::Delete();
		// 先にDX12Deviceを削除しておく
		Engine::Graphics::DX12Device::GetInstance().Finalize();
		Engine::Graphics::DX12Device::Delete();
		Engine::Utility::EngineContext::Delete();
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

		// TextureManager
		textureManager = std::make_unique<Engine::Graphics::TextureManager>();
		if (!textureManager->Initialize())
		{
			LOG_ERROR("TextureManagerの初期化に失敗");
			return false;
		}
		textureManager->AddContext();

		// SpriteRenderer
		spriteRenderer = std::make_unique<Engine::Graphics::SpriteRenderer>();
		if (!spriteRenderer->Initialize(
			*shaderLoader,
			context->GetBackBufferFormat(),
			context->GetDepthBufferFormat()
		))
		{
			LOG_ERROR("SpriteRendererの初期化に失敗");
			return false;
		}

		// ModelManager
		Engine::Graphics::ModelManager::Create();
		if (!Engine::Graphics::ModelManager::GetInstance().Initialize())
		{
			LOG_ERROR("ModelManagerの初期化に失敗");
			return false;
		}
		Engine::Graphics::ModelManager::GetInstance().AddContext();

		// ModelRenderer
		modelRenderer = std::make_unique<Engine::Graphics::ModelRenderer>();
		if (!modelRenderer->Initialize(
			*shaderLoader,
			L"../Engine/Assets/Shader/Model.hlsl",
			L"../Engine/Assets/Shader/Model.hlsl",
			context->GetBackBufferFormat(),
			context->GetDepthBufferFormat()
		))
		{
			LOG_ERROR("ModelRendererの初期化に失敗");
			return false;
		}

		RegisterAssetLoaders();
//		Engine::System::Assets::AssetManager::GetInstance().LoadDirectory(L"../App/Assets/");
		Engine::System::Assets::AssetManager::GetInstance().LoadDirectoryOrdered(L"../App/Assets/");

		// SpriteRenderSystem
		SetupEntities();
		// EntityInspector
#ifdef _DEBUG
		RegisterComponents();
#endif

		return true;
	}

	void Framework::DX12Finalize()
	{
		spriteRenderer->Finalize();
		spriteRenderer.reset();
		textureManager->Finalize();
		textureManager.reset();

		Engine::System::Assets::AssetManager::Delete();

		triangle->Finalize();
		triangle.reset();
		shaderLoader.reset();

		modelRenderer->Finalize();
		modelRenderer.reset();

		Engine::Graphics::ModelManager::GetInstance().Finalize();
		Engine::Graphics::ModelManager::Delete();

		Engine::System::ImGuiManager::GetInstance().Finalize();
		Engine::System::ImGuiManager::Delete();
		Engine::Graphics::DX12DescriptorHeapManager::GetInstance().Finalize();
		Engine::Graphics::DX12DescriptorHeapManager::Delete();
		Engine::Graphics::DX12Renderer::GetInstance().Finalize();
		Engine::Graphics::DX12Renderer::Delete();
	}

	void Framework::SetupEntities()
	{
		auto& reg = Engine::System::ECS::Registry::GetInstance();

		// カメラエンティティ作成
		auto cameraEntity = reg.CreateEntity();
		auto& cameraTransform = reg.AddComponent<Engine::Graphics::TransformComponent>(cameraEntity);
		cameraTransform.position = { 0.0f, 0.0f, -20.0f };
		auto& camera = reg.AddComponent<Engine::System::Camera::CameraComponent>(cameraEntity);
		camera.projectionType = Engine::System::Camera::CameraComponent::ProjectionType::Perspective;
		camera.fov = 60.0f;
		camera.nearClip = 0.1f;
		camera.farClip = 1000.0f;
		reg.EmplaceComponent<Engine::System::ECS::MainCameraTag>(cameraEntity);

		// テスト用エンティティ作成
		auto entity = reg.CreateEntity();
		reg.AddComponent<Engine::Graphics::TransformComponent>(entity);
		auto& sprite = reg.AddComponent<Engine::Graphics::SpriteComponent>(entity);
		sprite.textureID = Engine::System::Assets::AssetManager::GetInstance().GetTextureID("Test");
		sprite.size = { 500.0f, 500.0f };

		// モデルエンティティ作成
		auto modelEntity = reg.CreateEntity();
		auto& modelTransform = reg.AddComponent<Engine::Graphics::TransformComponent>(modelEntity);
		modelTransform.position = { 0.0f, 0.0f, 0.0f };
		modelTransform.scale = { 1.0f, 1.0f, 1.0f };

		auto& modelComp = reg.AddComponent<Engine::Graphics::ModelComponent>(modelEntity);
		modelComp.modelID = Engine::System::Assets::AssetManager::GetInstance().GetModelID("Kipfel");

		// SetupEntities を一時的に変更
		LOG_DEBUG("検索: {}", "Kipfel");
		for (const auto& [key, info] : Engine::System::Assets::AssetManager::GetInstance().GetAssets())
		{
			LOG_DEBUG("登録済み: key={} fileName={} type={}", key, info.fileName, info.type);
		}
	}

#ifdef _DEBUG
	void Framework::RegisterComponents()
	{
		entityInspector.RegisterComponent<Engine::Graphics::TransformComponent>(
			"Transform",
			[](entt::registry& reg, entt::entity entity)
			{
				auto& t = reg.get<Engine::Graphics::TransformComponent>(entity);
				ImGui::DragFloat3("Position", &t.position.x, 1.0f);
				ImGui::DragFloat3("Rotation", &t.rotation.x, 0.01f);
				ImGui::DragFloat3("Scale", &t.scale.x, 0.1f);
			}
		);

		entityInspector.RegisterComponent<Engine::Graphics::SpriteComponent>(
			"Sprite",
			[](entt::registry& reg, entt::entity entity)
			{
				auto& s = reg.get<Engine::Graphics::SpriteComponent>(entity);
				ImGui::DragFloat2("Size", &s.size.x, 1.0f);
				ImGui::DragFloat2("Pivot", &s.pivot.x, 0.01f, 0.0f, 1.0f);
				ImGui::ColorEdit4("Color", &s.color.x);
				ImGui::DragFloat4("UVRect", &s.uvRect.x, 0.01f, 0.0f, 1.0f);
				ImGui::Checkbox("Visible", &s.visible);
			}
		);

		entityInspector.RegisterComponent<Engine::System::Camera::CameraComponent>(
			"Camera",
			[](entt::registry& reg, entt::entity entity)
			{
				auto& c = reg.get<Engine::System::Camera::CameraComponent>(entity);
				int type = static_cast<int>(c.projectionType);
				if (ImGui::Combo("Type", &type, "Orthographic\0Perspective\0"))
				{
					c.projectionType =
						static_cast<Engine::System::Camera::CameraComponent::ProjectionType>(type);
				}
				ImGui::DragFloat("Zoom", &c.zoom, 0.01f, 0.1f, 10.0f);
				ImGui::DragFloat("Near", &c.nearClip, 0.1f);
				ImGui::DragFloat("Far", &c.farClip, 1.0f);
				ImGui::DragFloat("FOV", &c.fov, 1.0f, 1.0f, 179.0f);
				ImGui::DragFloat("Rotation", &c.rotation, 0.01f);
			}
		);
	}
#endif

	void Framework::RegisterAssetLoaders()
	{
		auto& assetManager = Engine::System::Assets::AssetManager::GetInstance();

		// 各種テクスチャローダー登録
		assetManager.RegisterLoader(".png", [&](const std::wstring& path) -> uint64_t
			{
				return textureManager->Load(path);
			});
		assetManager.RegisterLoader(".jpg", [&](const std::wstring& path) -> uint64_t
			{
				return textureManager->Load(path);
			});
		assetManager.RegisterLoader(".jpeg", [&](const std::wstring& path) -> uint64_t
			{
				return textureManager->Load(path);
			});
		assetManager.RegisterLoader(".bmp", [&](const std::wstring& path) -> uint64_t
			{
				return textureManager->Load(path);
			});
		assetManager.RegisterLoader(".fbx", [&](const std::wstring& path) -> uint64_t
			{
				return Engine::Graphics::ModelManager::GetInstance().Load(path);
			});
		assetManager.RegisterLoader(".obj", [&](const std::wstring& path) -> uint64_t
			{
				return Engine::Graphics::ModelManager::GetInstance().Load(path);
			});
		assetManager.RegisterLoader(".mdl", [&](const std::wstring& path) -> uint64_t
			{
				return Engine::Graphics::ModelManager::GetInstance().Load(path);
			});
	}
}