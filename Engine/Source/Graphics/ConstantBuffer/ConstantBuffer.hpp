#pragma once
#include "Graphics/DX12/Type.hpp"

namespace Engine::Graphics
{
    template<typename T>
    class ConstantBuffer
    {
    public:
        bool Initialize(ID3D12Device* device);
        void Finalize();

        void Update(const T& data);

        D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const;

    private:
        dx12::ComPtr<ID3D12Resource> resource;
        void* mapped = nullptr;

        static constexpr UINT AlignedSize = (sizeof(T) + 255) & ~255;
    };
} // Engine::Graphics