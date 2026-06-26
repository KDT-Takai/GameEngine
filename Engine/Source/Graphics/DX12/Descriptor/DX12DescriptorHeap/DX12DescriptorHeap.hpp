#pragma once
#include "../../Type.hpp"

namespace Engine::Graphics
{
	struct DescriptorHandle
	{
		static constexpr UINT InvalidIndex = static_cast<UINT>(-1);

		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};	// shader visibleでない場合は ptr == 0
		UINT index = InvalidIndex;	// ヒープ内のインデックス

		bool IsValid() const { return index != InvalidIndex; }
		bool IsShaderVisible() const { return gpuHandle.ptr != 0; }
	};

	class DX12DescriptorHeap
	{
	public:
		// ヒープを生成
		bool Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, UINT capacity, bool shaderVisible);

		void Finalize();

		// 次の空きインデックスのハンドル発行
		// 容量超過時はIsValid()==falseのハンドルを返す
		DescriptorHandle Allocate();

		// 指定インデックスのハンドルを再計算
		DescriptorHandle GetHandle(UINT index) const;

		// CPUハンドルから対応するインデックスを逆算する
		UINT GetIndexFromHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle) const;

		// 発行数を０に
		void Reset();

		ID3D12DescriptorHeap* GetHeap() const { return heap.Get(); }
		D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return heapType; }
		UINT GetCapacity() const { return capacity; }
		UINT GetCount() const { return count; }
	private:
		dx12::DescriptorHeap heap;
		D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		UINT descriptorSize = 0;
		UINT capacity = 0;
		UINT count = 0;
		bool shaderVisible = false;

		D3D12_CPU_DESCRIPTOR_HANDLE cpuStart{};
		D3D12_GPU_DESCRIPTOR_HANDLE gpuStart{};
	};
} // Engine::Graphics