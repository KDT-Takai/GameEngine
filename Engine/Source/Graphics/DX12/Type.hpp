#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <dxgi1_6.h>

namespace dx12
{
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	using Device = ComPtr<ID3D12Device>;
	using Factory6 = ComPtr<IDXGIFactory6>;
	using SwapChain = ComPtr<IDXGISwapChain4>;
	using CmdList = ComPtr<ID3D12CommandList>;
	using CmdQueue = ComPtr<ID3D12CommandQueue>;
	using CmdAllocator = ComPtr<ID3D12CommandAllocator>;
	using Resource = ComPtr<ID3D12Resource>;
	using DescriptorHeap = ComPtr<ID3D12DescriptorHeap>;
	using Fence = ComPtr<ID3D12Fence>;
	using Debug = ComPtr<ID3D12Debug>;
	using DebugDevice = ComPtr<ID3D12DebugDevice>;
	using Adapter = ComPtr<IDXGIAdapter4>;
}