#pragma once
#include "../DX12DescriptorHeap/DX12DescriptorHeap.hpp"
#include <vector>

namespace Engine::Graphics
{
	class CbvSrvUavDescriptorAllocator
	{
	public:
		bool Initialize(ID3D12Device* device, UINT capcity);
		void Finalize();

		DescriptorHandle Allocate();
		// ディスクリプタの解放
		void Free(const DescriptorHandle& handle);

		ID3D12DescriptorHeap* GetHeap() const { return heap.GetHeap(); }

	private:

		DX12DescriptorHeap heap;
		std::vector<UINT> freeList;
	};
}