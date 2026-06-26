#include "pch/pch.h"
#include "RtvDescriptorAllocator.hpp"

namespace Engine::Graphics
{
	bool RtvDescriptorAllocator::Initialize(ID3D12Device* device, UINT capacity)
	{
		if (!heap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, capacity, false))
		{
			LOG_ERROR("RTVópÉqÅ[ÉvÇÃèâä˙âªÇ…é∏îs");
			return false;
		}
		return true;
	}

	void RtvDescriptorAllocator::Finalize()
	{
		heap.Finalize();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE RtvDescriptorAllocator::Allocate()
	{
		auto handle = heap.Allocate();
		return handle.cpuHandle;
	}
} // Engine::Graphics