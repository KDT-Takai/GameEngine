#pragma once
#include "Graphics/DX12/Type.hpp"
#include "Graphics/DX12/Descriptor/DX12DescriptorHeap/DX12DescriptorHeap.hpp"
#include <string>

namespace Engine::Graphics
{
    class Texture
    {
    public:
        bool Initialize(
            ID3D12GraphicsCommandList* cmdList,
            const std::wstring& path
        );
        void Finalize();

        void ReleaseUploadBuffer();
        const DescriptorHandle& GetHandle() const { return handle; }
        UINT GetWidth()  const { return width; }
        UINT GetHeight() const { return height; }
        void InitializeFromResource(dx12::ComPtr<ID3D12Resource> res, const DescriptorHandle& handle, UINT w, UINT h);
    private:
        dx12::ComPtr<ID3D12Resource> resource;
        dx12::ComPtr<ID3D12Resource> uploadBuffer;  
        DescriptorHandle             handle{};
        UINT                         width = 0;
        UINT                         height = 0;
    };
} // Engine::Graphics