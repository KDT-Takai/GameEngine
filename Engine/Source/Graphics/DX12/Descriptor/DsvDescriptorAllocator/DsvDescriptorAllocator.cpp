#include "pch/pch.h"
#include "DsvDescriptorAllocator.hpp"

namespace Engine::Graphics
{
	bool DsvDescriptorAllocator::Initialize(ID3D12Device* device, UINT capacity)
	{
		if (!heap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, capacity, false))
		{
			LOG_ERROR("DSVópÉqÅ[ÉvÇÃèâä˙âªÇ…é∏îs");
			return false;
		}
		return true;
	}

	void DsvDescriptorAllocator::Finalize()
	{
		heap.Finalize();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DsvDescriptorAllocator::Allocate()
	{
		auto handle = heap.Allocate();
		return handle.cpuHandle;
	}
}