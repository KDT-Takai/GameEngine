#pragma once
#include "../Type.hpp"
#include "../Descriptor/RtvDescriptorAllocator/RtvDescriptorAllocator.hpp"
#include "../Descriptor/DsvDescriptorAllocator/DsvDescriptorAllocator.hpp"

namespace Engine::Graphics
{
	// バックバッファ数
	inline constexpr UINT FRAME_COUNT = 2;

	// フレームごとのリソース
	struct FrameResource
	{
		dx12::CmdAllocator cmdAllocator;	// コマンドアロケーター
		dx12::Resource backBuffer;		// バックバッファ
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle{};	// レンダーターゲットビューのハンドル
		UINT64 fenceValue = 0;	// フェンス値
	};

	class DX12RendererContext
	{
	public:
		// 初期化
		bool Initialize(HWND hwnd, UINT width, UINT height);
		// 終了
		void Finalize();

		// フレーム処理
		bool BeginFrame();
		bool EndFrame();

		// レンダーターゲット
		void SetRenderTarget();
		void ClearRenderTarget();

		// GPU同期
		void WaitGPU();
		// 次フレームへ移行
		void MoveToNextFrame();

		// ビューポート・シザー矩形の設定
		void SetViewPortAndScissor(UINT width, UINT height);

		// ゲッター
		dx12::GraphicsCmdList GetCmdList() const { return cmdList; }
		dx12::CmdQueue GetCmdQueue() const { return cmdQueue; }
		const FrameResource& GetCurrentFrame() const { return frameResources[currentFrameIndex]; }
		UINT GetFrameIndex() const { return currentFrameIndex; }

	private:
		bool CreateCommandQueue();
		bool CreateSwapChain(HWND hwnd, UINT width, UINT height);
		bool CreateRTVHeap();
		bool CreateRenderTargetViews();
		bool CreateDSVHeap();
		bool CreateDepthBuffer(UINT width, UINT height);
		bool CreateCommandAllocators();
		bool CreateCommandList();
		bool CreateFence();

		// デバイス
		ID3D12Device* device = nullptr;

		//  コマンド
		dx12::CmdQueue cmdQueue;
		dx12::GraphicsCmdList cmdList;

		// スワップチェイン 
		dx12::SwapChain swapChain;

		// フレームリソース 
		FrameResource frameResources[FRAME_COUNT];
		UINT          currentFrameIndex = 0;

		// ディスクリプタ
		RtvDescriptorAllocator rtvAllocator;
		DsvDescriptorAllocator dsvAllocator;
		D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle{}; // SetRenderTarget/ClearRenderTargetで使用

		// 深度バッファ 
		dx12::Resource depthBuffer;

		// フェンス（GPU-CPU 同期） 
		dx12::Fence fence;
		HANDLE fenceEvent = nullptr;
		UINT64 nextFenceValue = 1;

		// ビューポート・シザー矩形
		D3D12_VIEWPORT viewport{};
		D3D12_RECT scissorRect{};

		// レンダー設定 
		DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		FLOAT clearColor[4] = { 0.2f, 0.2f, 0.2f, 1.0f };

		// ウィンドウサイズ
		UINT windowWidth = 0;
		UINT windowHeight = 0;

	};
}
