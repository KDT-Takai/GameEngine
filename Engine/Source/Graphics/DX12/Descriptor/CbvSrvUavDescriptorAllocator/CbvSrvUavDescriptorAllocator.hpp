#pragma once
#include "../DX12DescriptorHeap/DX12DescriptorHeap.hpp"
#include <vector>

namespace Engine::Graphics
{
	class CbvSrvUavDescriptorAllocator
	{
	public:
		bool Initialize(ID3D12Device* device, UINT capacity);
		void Finalize();

		DescriptorHandle Allocate();
		// ディスクリプタの解放
		void Free(const DescriptorHandle& handle);
		void Free(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);

		ID3D12DescriptorHeap* GetHeap() const { return heap.GetHeap(); }

	private:

		DX12DescriptorHeap heap;
		std::vector<UINT> freeList;
	};
} // Engine::Graphics