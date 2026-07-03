#include "pch/pch.h"
#include "ImGuiManager.hpp"

#include "Graphics/DX12/Device/DX12Device.hpp"
#include "Graphics/DX12/RendererContext/DX12RendererContext.hpp"
#include "Graphics/DX12/Descriptor/DX12DescriptorHeapManager.hpp"
#include "System/Widnow/Window.hpp"

// ImGui
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Engine::System
{
	namespace
	{
		void SrvDescriptorAlloc(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* outCpu, D3D12_GPU_DESCRIPTOR_HANDLE* outGpu)
		{
			auto* allocator = static_cast<Engine::Graphics::CbvSrvUavDescriptorAllocator*>(info->UserData);
			auto handle = allocator->Allocate();
			*outCpu = handle.cpuHandle;
			*outGpu = handle.gpuHandle;
		}

		void SrvDescriptorFree(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu, D3D12_GPU_DESCRIPTOR_HANDLE gpu)
		{
			auto* allocator = static_cast<Engine::Graphics::CbvSrvUavDescriptorAllocator*>(info->UserData);
			allocator->Free(cpu, gpu);
		}
	}

	bool ImGuiManager::Initialize(
		Window& window,
		Engine::Graphics::DX12Device& device,
		Engine::Graphics::DX12RendererContext& context,
		Engine::Graphics::DX12DescriptorHeapManager& heapMgr)
	{
		rendererContext = &context;
		heapManager = &heapMgr;

		IMGUI_CHECKVERSION();
		imguiContext = ImGui::CreateContext();
		// 複数モジュール（DLL）構成にする場合は、各関数の先頭で
		// ImGui::SetCurrentContext(imguiContext) を呼ぶ必要がある。
		// 現状は単一モジュール構成のため省略している

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImGui::StyleColorsDark();

		if (!ImGui_ImplWin32_Init(window.GetHWnd()))
		{
			LOG_ERROR("ImGui_ImplWin32_Initに失敗");
			return false;
		}

		auto& cbvSrvUavAllocator = heapManager->GetCbvSrvUavAllocator();

		ImGui_ImplDX12_InitInfo initInfo{};
		initInfo.Device = device.GetDevice().Get();
		initInfo.CommandQueue = context.GetCmdQueue().Get();
		initInfo.NumFramesInFlight = static_cast<int>(Engine::Graphics::FRAME_COUNT);
		initInfo.RTVFormat = context.GetBackBufferFormat();
		initInfo.DSVFormat = context.GetDepthBufferFormat();
		initInfo.UserData = &cbvSrvUavAllocator;
		initInfo.SrvDescriptorHeap = cbvSrvUavAllocator.GetHeap();
		initInfo.SrvDescriptorAllocFn = SrvDescriptorAlloc;
		initInfo.SrvDescriptorFreeFn = SrvDescriptorFree;

		if (!ImGui_ImplDX12_Init(&initInfo))
		{
			LOG_ERROR("ImGui_ImplDX12_Initに失敗");
			return false;
		}

		initialized = true;
		LOG_INFO("ImGuiManagerの初期化に成功");
		return true;
	}

	void ImGuiManager::Finalize()
	{
		if (!initialized)
		{
			return;
		}

		ClearDebugUI();

		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		rendererContext = nullptr;
		heapManager = nullptr;
		imguiContext = nullptr;
		initialized = false;
	}

	void ImGuiManager::NewFrame()
	{
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiManager::Update()
	{
		for (auto& [key, func] : debugUIFunctions)
		{
			if (func)
			{
				func();
			}
		}
	}

	void ImGuiManager::EndFrame()
	{
		ImGui::Render();

		// ImGui用のSRVヒープをセットする
		// 他の描画で別のCBV_SRV_UAVヒープを使っている場合、ここで上書きされる点に注意
		auto cmdList = rendererContext->GetCmdList();
		ID3D12DescriptorHeap* heaps[] = { heapManager->GetCbvSrvUavAllocator().GetHeap() };
		cmdList->SetDescriptorHeaps(1, heaps);

		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList.Get());

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void ImGuiManager::AddDebugUI(std::function<void()> guiFunc, const std::string& key)
	{
		debugUIFunctions[key] = std::move(guiFunc);
	}

	void ImGuiManager::RemoveDebugUI(const std::string& key)
	{
		debugUIFunctions.erase(key);
	}

	bool ImGuiManager::HasDebugUI(const std::string& key) const
	{
		return debugUIFunctions.find(key) != debugUIFunctions.end();
	}

	void ImGuiManager::ClearDebugUI()
	{
		debugUIFunctions.clear();
	}

	LRESULT ImGuiManager::WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam);
	}
} // Engine::System