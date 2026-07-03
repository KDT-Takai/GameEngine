#include "pch/pch.h"
#include "DX12DescriptorHeapManager.hpp"

namespace Engine::Graphics
{
	bool DX12DescriptorHeapManager::Initialize(ID3D12Device* device, UINT cbvSrvUavCapacity)
	{
		if (!cbvSrvUavAllocator.Initialize(device, cbvSrvUavCapacity))
		{
			LOG_ERROR("CbvSrvUavDescriptorAllocator‚Ì‰Šú‰»‚É¸”s");
			return false;
		}
		LOG_INFO("CbvSrvUavDescriptorAllocator‚Ì‰Šú‰»‚É¬Œ÷");
		return true;
	}

	void DX12DescriptorHeapManager::Finalize()
	{
		cbvSrvUavAllocator.Finalize();
	}
}