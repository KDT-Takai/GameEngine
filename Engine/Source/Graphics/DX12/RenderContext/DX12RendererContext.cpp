#include "pch/pch.h"
#include "DX12RendererContext.hpp"
#include "../Device/DX12Device.hpp"

namespace Engine::Graphics
{
	bool DX12RendererContext::Initialize(HWND hwnd, UINT width, UINT height)
	{
		// デバイスの取得
		device = DX12Device::Get().GetDevice().Get();
		if (!device)
		{
			LOG_ERROR("DX12Deviceが未初期化");
			return false;
		}

		windowWidth = width;
		windowHeight = height;

		if (!CreateCommandQueue())
		{
			LOG_ERROR("CommandQueueの生成に失敗");
			return false;
		}
		if (!CreateSwapChain(hwnd, width, height))
		{
			LOG_ERROR("SwapChainの生成に失敗");
			return false;
		}
		if (!CreateRTVHeap())
		{
			LOG_ERROR("RTVHeapの生成に失敗");
			return false;
		}
		if (!CreateRenderTargetViews())
		{
			LOG_ERROR("RenderTargetViewの生成に失敗");
			return false;
		}
		if (!CreateDSVHeap())
		{
			LOG_ERROR("DSVHeapの生成に失敗");
			return false;
		}
		if (!CreateDepthBuffer(width, height))
		{
			LOG_ERROR("DepthBufferの生成に失敗");
			return false;
		}
		if (!CreateCommandAllocators())
		{
			LOG_ERROR("CommandAllocatorsの生成に失敗");
			return false;
		}
		if (!CreateCommandList())
		{
			LOG_ERROR("CommandListの生成に失敗");
			return false;
		}
		if (!CreateFence())
		{
			LOG_ERROR("Fenceの生成に失敗");
			return false;
		}

		SetViewPortAndScissor(width, height);

		LOG_INFO("DX12RendererContextの初期化に成功");
		return true;
	}

	void DX12RendererContext::Finalize()
	{
		// GPUの処理待ち
		WaitGPU();

		if (fenceEvent)
		{
			CloseHandle(fenceEvent);
			fenceEvent = nullptr;
		}

		cmdList.Reset();
		for (auto& frame : frameResources)
		{
			frame.cmdAllocator.Reset();
			frame.backBuffer.Reset();
		}
		fence.Reset();
		dsvAllocator.Finalize();
		rtvAllocator.Finalize();
		depthBuffer.Reset();
		swapChain.Reset();
		cmdQueue.Reset();
	}

	bool DX12RendererContext::BeginFrame()
	{
		auto& frame = frameResources[currentFrameIndex];

		// コマンドアロケーターのリセット
		auto hr = frame.cmdAllocator->Reset();
		if (FAILED(hr))
		{
			LOG_ERROR("コマンドアロケーターのリセットに失敗");
			return false;
		}
		// コマンドリストのリセット
		hr = cmdList->Reset(frame.cmdAllocator.Get(), nullptr);
		if (FAILED(hr))
		{
			LOG_ERROR("コマンドリストのリセットに失敗");
			return false;
		}

		cmdList->RSSetViewports(1, &viewport);
		cmdList->RSSetScissorRects(1, &scissorRect);

		// バリア
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = frame.backBuffer.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cmdList->ResourceBarrier(1, &barrier);

		return true;
	}

	bool DX12RendererContext::EndFrame()
	{
		auto& frame = frameResources[currentFrameIndex];

		// バリア
		D3D12_RESOURCE_BARRIER barrier{};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = frame.backBuffer.Get();
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		cmdList->ResourceBarrier(1, &barrier);

		// コマンドリストのクローズ
		auto hr = cmdList->Close();
		if (FAILED(hr))
		{
			LOG_ERROR("コマンドリストのクローズに失敗");
			return false;
		}

		// コマンドキューへ送信
		ID3D12CommandList* ppCmdLists[] = {cmdList.Get()};
		cmdQueue->ExecuteCommandLists(_countof(ppCmdLists), ppCmdLists);

		// 画面フリップ
		hr = swapChain->Present(1, 0);
		if (FAILED(hr))
		{
			LOG_ERROR("プリセントに失敗");
			return false;
		}

		// フェンスシグナル・次フレームへ移行
		MoveToNextFrame();

		return true;
	}

	void DX12RendererContext::SetRenderTarget()
	{
		auto& frame = frameResources[currentFrameIndex];

		cmdList->OMSetRenderTargets(1, &frame.rtvHandle, FALSE, &dsvHandle);
	}

	void DX12RendererContext::ClearRenderTarget()
	{
		auto& frame = frameResources[currentFrameIndex];

		cmdList->ClearRenderTargetView(frame.rtvHandle, clearColor, 0, nullptr);
		cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	}

	void DX12RendererContext::WaitGPU()
	{
		const UINT64 signalValue = nextFenceValue;
		cmdQueue->Signal(fence.Get(), signalValue);
		nextFenceValue++;

		if (fence->GetCompletedValue() < signalValue)
		{
			fence->SetEventOnCompletion(signalValue, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
		}
	}

	void DX12RendererContext::MoveToNextFrame()
	{
		const UINT64 currentFenceValue = nextFenceValue;
		cmdQueue->Signal(fence.Get(), currentFenceValue);
		frameResources[currentFrameIndex].fenceValue = currentFenceValue;
		nextFenceValue++;

		// 次のフレームインデックス
		currentFrameIndex = swapChain->GetCurrentBackBufferIndex();

		// 次フレームがまだGPU処理中なら待機
		const UINT64 nextFrameFenceValue = frameResources[currentFrameIndex].fenceValue;
		if (fence->GetCompletedValue() < nextFrameFenceValue)
		{
			fence->SetEventOnCompletion(nextFrameFenceValue, fenceEvent);
			WaitForSingleObject(fenceEvent, INFINITE);
		}
	}

	void DX12RendererContext::SetViewPortAndScissor(UINT width, UINT height)
	{
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<FLOAT>(width);
		viewport.Height = static_cast<FLOAT>(height);
		viewport.MinDepth = D3D12_MIN_DEPTH;
		viewport.MaxDepth = D3D12_MAX_DEPTH;

		scissorRect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
	}

	bool DX12RendererContext::CreateCommandQueue()
	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		auto hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(cmdQueue.GetAddressOf()));
		if (FAILED(hr))
		{
			LOG_ERROR("コマンドキューの生成に失敗");
			return false;
		}

		return true;
	}

	bool DX12RendererContext::CreateSwapChain(HWND hwnd, UINT width, UINT height)
	{
		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.Width = width;
		desc.Height = height;
		desc.Format = backBufferFormat;
		desc.Stereo = FALSE;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FRAME_COUNT;
		desc.Scaling = DXGI_SCALING_STRETCH;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		desc.Flags = 0;

		auto factory = DX12Device::Get().GetFactory();

		dx12::ComPtr<IDXGISwapChain1> swapChain1;
		auto hr = factory->CreateSwapChainForHwnd(
			cmdQueue.Get(),
			hwnd,
			&desc,
			nullptr,
			nullptr,
			swapChain1.GetAddressOf()
		);
		if (FAILED(hr))
		{
			LOG_ERROR("スワップチェインの生成に失敗");
			return false;
		}

		factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

		hr = swapChain1.As(&swapChain);
		if (FAILED(hr))
		{
			LOG_ERROR("IDXGISwapChain4へのキャストに失敗");
		}

		currentFrameIndex = swapChain->GetCurrentBackBufferIndex();

		return true;
	}

	bool DX12RendererContext::CreateRTVHeap()
	{
		// RTV用アロケータをフレーム数分の容量で初期化
		if (!rtvAllocator.Initialize(device, FRAME_COUNT))
		{
			LOG_ERROR("RtvDescriptorAllocatorの初期化に失敗");
			return false;
		}

		return true;
	}

	bool DX12RendererContext::CreateRenderTargetViews()
	{
		for (UINT i = 0; i < FRAME_COUNT; i++)
		{
			// スワップチェインからバックバッファリソースを取得
			auto hr = swapChain->GetBuffer(i, IID_PPV_ARGS(frameResources[i].backBuffer.GetAddressOf()));
			if (FAILED(hr))
			{
				LOG_ERROR("バックバッファの取得に失敗");
				return false;
			}
			// アロケータからRTV用ディスクリプタを確保
			auto handle = rtvAllocator.Allocate();
			if (handle.ptr == 0)
			{
				LOG_ERROR("RTV用ディスクリプタの確保に失敗");
				return false;
			}
			// RTVを作成
			device->CreateRenderTargetView(frameResources[i].backBuffer.Get(), nullptr, handle);
			// ハンドルを保存
			frameResources[i].rtvHandle = handle;
		}
		return true;
	}

	bool DX12RendererContext::CreateDSVHeap()
	{
		// DSV用アロケータを容量1で初期化
		if (!dsvAllocator.Initialize(device, 1))
		{
			LOG_ERROR("DsvDescriptorAllocatorの初期化に失敗");
			return false;
		}

		return true;

	}

	bool DX12RendererContext::CreateDepthBuffer(UINT width, UINT height)
	{
		// リソース記述子
		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = 0;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		// 最適クリア値
		D3D12_CLEAR_VALUE clearValue{};
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		// ヒーププロパティ GPU
		D3D12_HEAP_PROPERTIES heapProp{};
		heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.CreationNodeMask = 1;
		heapProp.VisibleNodeMask = 1;

		// リソースの生成
		auto hr = device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(depthBuffer.GetAddressOf())
		);

		if (FAILED(hr))
		{
			LOG_ERROR("深度バッファの生成に失敗");
			return false;
		}

		// DSVの生成
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.Texture2D.MipSlice = 0;

		device->CreateDepthStencilView(
			depthBuffer.Get(),
			&dsvDesc,
			dsvHandle
		);

		return true;
	}

	bool DX12RendererContext::CreateCommandAllocators()
	{
		for (UINT i = 0; i < FRAME_COUNT; i++)
		{
			auto hr = device->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(frameResources[i].cmdAllocator.GetAddressOf())
			);
			if (FAILED(hr))
			{
				LOG_ERROR("CommandAllocatorの生成に失敗");
				return false;
			}
		}
		return true;
	}

	bool DX12RendererContext::CreateCommandList()
	{
		// フレーム０のアロケーターを使って生成
		auto hr = device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			frameResources[0].cmdAllocator.Get(),
			nullptr,
			IID_PPV_ARGS(cmdList.GetAddressOf())
		);
		if (FAILED(hr))
		{
			LOG_ERROR("CommandListの生成に失敗");
			return false;
		}
		// OpenからCloseにする
		hr = cmdList->Close();
		if (FAILED(hr))
		{
			LOG_ERROR("CommandListのクローズに失敗");
			return false;
		}
		return true;
	}

	bool DX12RendererContext::CreateFence()
	{
		auto hr = device->CreateFence(
			0,
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(fence.GetAddressOf())
		);
		if (FAILED(hr))
		{
			LOG_ERROR("フェンスの生成に失敗");
			return false;
		}
		// CPU側の待機用イベントを生成
		fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
		if (fenceEvent == nullptr)
		{
			LOG_ERROR("フェンスイベントの生成に失敗");
			return false;
		}
		return true;
	}

} // namespace Engine::Graphics