#include "pch/pch.h"
#include "DX12DescriptorHeap.hpp"

namespace Engine::Graphics
{
	bool DX12DescriptorHeap::Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT capacity, bool shaderVisible)
	{
		if ((type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV) && shaderVisible)
		{
			LOG_ERROR("RTV/DSVヒープはshader visibleにできません");
			return false;
		}

		heapType = type;
		this->capacity = capacity;
		this->shaderVisible = shaderVisible;

		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = type;
		desc.NumDescriptors = capacity;
		desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		auto hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(heap.GetAddressOf()));
		if (FAILED(hr))
		{
			LOG_ERROR("DescriptorHeapの生成に失敗");
			return false;
		}
		descriptorSize = device->GetDescriptorHandleIncrementSize(type);
		cpuStart = heap->GetCPUDescriptorHandleForHeapStart();
		if (shaderVisible)
		{
			gpuStart = heap->GetGPUDescriptorHandleForHeapStart();
		}
		count = 0;
		return true;
	}

	void DX12DescriptorHeap::Finalize()
	{
		heap.Reset();
		count = 0;
	}

	DescriptorHandle DX12DescriptorHeap::Allocate()
	{
		if (count >= capacity)
		{
			LOG_ERROR("DescriptorHeapの容量が不足しています");
			return DescriptorHandle{};
		}

		UINT index = count;
		count++;

		return GetHandle(index);
	}

	DescriptorHandle DX12DescriptorHeap::GetHandle(UINT index) const
	{
		if (index >= capacity)
		{
			LOG_ERROR("DescriptorHeapの範囲外インデックスです");
			return DescriptorHandle{};
		}

		DescriptorHandle handle;
		handle.index = index;
		handle.cpuHandle.ptr = cpuStart.ptr + static_cast<SIZE_T>(index) * descriptorSize;
		if (shaderVisible)
		{
			handle.gpuHandle.ptr = gpuStart.ptr + static_cast<UINT64>(index) * descriptorSize;
		}
		return handle;
	}

	void DX12DescriptorHeap::Reset()
	{
		count = 0;
	}
}