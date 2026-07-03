#pragma once
#include <memory>
#include "System/Widnow/Window.hpp"

#include "Utility/Singleton/Singleton.hpp"
#include "Graphics/DX12/Device/DX12Device.hpp"
#include "Graphics/DX12/Renderer/DX12Renderer.hpp"
#include "Graphics/DX12/Descriptor/DX12DescriptorHeapManager.hpp"

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
		// ウィンドウ
		std::unique_ptr<Window> window;
	};
}