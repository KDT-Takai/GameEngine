#include "pch/pch.h"
#include "CbvSrvUavDescriptorAllocator.hpp"

namespace Engine::Graphics
{
	bool CbvSrvUavDescriptorAllocator::Initialize(ID3D12Device* device, UINT capacity)
	{
		if (!heap.Initialize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, capacity, true))
		{
			LOG_ERROR("CBV_SRV_UAV用ヒープの初期化に失敗");
			return false;
		}
		return true;
	}

	void CbvSrvUavDescriptorAllocator::Finalize()
	{
		heap.Finalize();
		freeList.clear();
	}

	DescriptorHandle CbvSrvUavDescriptorAllocator::Allocate()
	{
		// 解放済みインデックスがあれば再利用
		if (!freeList.empty())
		{
			UINT index = freeList.back();
			freeList.pop_back();
			return heap.GetHandle(index); // 既存インデックスのハンドルを再計算
		}

		// 無ければ新規インデックスを発行
		return heap.Allocate();
	}

	void CbvSrvUavDescriptorAllocator::Free(const DescriptorHandle& handle)
	{
		if (!handle.IsValid())
		{
			LOG_ERROR("無効なハンドルをFreeしようとしました");
			return;
		}

		freeList.push_back(handle.index);
	}
}