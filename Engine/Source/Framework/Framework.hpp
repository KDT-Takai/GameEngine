#pragma once
#include <memory>
#include "System/Widnow/Window.hpp"
#include "System/ECS/Registry/Registry.hpp"
#include "System/Screen/Screen.hpp"
#include "System/Input/InputManager/InputManager.hpp"
#include "System/Camera/CameraSystem/CameraSystem.hpp"
#include "System/Assets/AssetManager/AssetManager.hpp"

#ifdef _DEBUG
// Debugger (Debug-only; not linked/built in Release)
#include "System/EntityInspector/EntityInspector.hpp"
#include "System/Assets/AssetBrowser/AssetBrowser.hpp"
#endif

#include "Utility/Singleton/Singleton.hpp"

#include "Graphics/DX12/Device/DX12Device.hpp"
#include "Graphics/DX12/Renderer/DX12Renderer.hpp"
#include "Graphics/DX12/Descriptor/DX12DescriptorHeapManager.hpp"

#include "Graphics/Shader/RuntimeShaderLoader/RuntimeShaderLoader.hpp"
#include "Graphics/Primitive/Triangle/Triangle.hpp"
#include "Graphics/Texture/TextureManager/TextureManager.hpp"
#include "Graphics/Sprite/SpriteRenderer/SpriteRenderer.hpp"
#include "Graphics/Sprite/SpriteRenderSystem/SpriteRenderSystem.hpp"
// もでる
#include "Graphics/Model/ModelManager/ModelManager.hpp"
#include "Graphics/Model/ModelRenderer/ModelRenderer.hpp"
#include "Graphics/Model/ModelRenderSystem/ModelRenderSystem.hpp"
#include "Graphics/Component/ModelComponent/ModelComponent.hpp"

// 前方宣言
namespace Engine::Graphics
{
	class Triangle;
	class RuntimeShaderLoader;
}

namespace Engine::System
{
	class Window;

	class Framework : public Engine::Utility::Singleton<Framework> {
		DECLARE_SINGLETON(Framework)
	public:
		bool Initialize(int width, int height, const char* title);
		void Run();
		void Finalize();
	private:
		bool DX12Initialize(int width, int height);
		void DX12Finalize();
		void SetupEntities();
#ifdef _DEBUG
		void RegisterComponents();
#endif
		void RegisterAssetLoaders();
		// ウィンドウ
		std::unique_ptr<Window> window;

		// 描画テスト用（Triangle表示確認後は適切なSceneやEntityに移す）
		std::unique_ptr<Engine::Graphics::Triangle> triangle;
		std::unique_ptr<Engine::Graphics::RuntimeShaderLoader> shaderLoader;

		// スプライト描画
		std::unique_ptr<Engine::Graphics::TextureManager>       textureManager;
		std::unique_ptr<Engine::Graphics::SpriteRenderer>       spriteRenderer;
		Engine::Graphics::SpriteRenderSystem                    spriteRenderSystem;

		// カメラ
		Engine::System::Camera::CameraSystem cameraSystem;

		// EntityInspector
#ifdef _DEBUG
		Engine::System::Debug::EntityInspector entityInspector;
#endif

		// AssetBrowser
#ifdef _DEBUG
		Engine::System::AssetBrowser assetBrowser;
#endif

		std::unique_ptr<Engine::Graphics::ModelRenderer>  modelRenderer;
		Engine::Graphics::ModelRenderSystem               modelRenderSystem;
	};
}