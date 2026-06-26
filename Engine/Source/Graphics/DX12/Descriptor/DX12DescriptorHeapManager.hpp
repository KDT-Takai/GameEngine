#pragma once
#include "CbvSrvUavDescriptorAllocator/CbvSrvUavDescriptorAllocator.hpp"
#include "Utility/Singleton/Singleton.hpp"

namespace Engine::Graphics
{
	class DX12DescriptorHeapManager : public Engine::Utility::Singleton<DX12DescriptorHeapManager>
	{
		DECLARE_SINGLETON(DX12DescriptorHeapManager)
	public:
		// cbvSrvUavCapacity: ImGuiのフォント+将来のテクスチャ等を見込んだ総数
		bool Initialize(ID3D12Device* device, UINT cbvSrvUavCapacity);
		void Finalize();

		CbvSrvUavDescriptorAllocator& GetCbvSrvUavAllocator() { return cbvSrvUavAllocator; }

	private:
		CbvSrvUavDescriptorAllocator cbvSrvUavAllocator;
	};
}