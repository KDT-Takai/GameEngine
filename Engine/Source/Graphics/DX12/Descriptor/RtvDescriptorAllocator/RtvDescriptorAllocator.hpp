#pragma once
#include "../DX12DescriptorHeap/DX12DescriptorHeap.hpp"

namespace Engine::Graphics
{
	class RtvDescriptorAllocator
	{
	public:
		bool Initialize(ID3D12Device* device, UINT capacity);
		void Finalize();
		D3D12_CPU_DESCRIPTOR_HANDLE Allocate();

	private:
		DX12DescriptorHeap heap;
	};
} // Engine::Graphics