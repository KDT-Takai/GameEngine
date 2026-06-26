#include "pch/pch.h"
#include "DX12DescriptorHeapManager.hpp"

namespace Engine::Graphics
{
	bool DX12DescriptorHeapManager::Initialize(ID3D12Device* device, UINT cbvSrvUavCapacity)
	{
		if (!cbvSrvUavAllocator.Initialize(device, cbvSrvUavCapacity))
		{
			LOG_ERROR("CbvSrvUavDescriptorAllocatorÇÃèâä˙âªÇ…é∏îs");
			return false;
		}
		return true;
	}

	void DX12DescriptorHeapManager::Finalize()
	{
		cbvSrvUavAllocator.Finalize();
	}
}