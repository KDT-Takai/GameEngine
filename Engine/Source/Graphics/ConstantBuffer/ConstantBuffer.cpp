#include "pch/pch.h"
#include "Constantbuffer.hpp"
#include "Graphics/ConstantBuffer/TransformBuffer.hpp"
#include "Graphics/Sprite/SpriteRenderer/SpriteRenderer.hpp"

namespace Engine::Graphics
{
    template<typename T>
    bool ConstantBuffer<T>::Initialize(ID3D12Device* device)
    {
        D3D12_HEAP_PROPERTIES heapProp{};
        heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width = AlignedSize;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        HRESULT hr = device->CreateCommittedResource(
            &heapProp,
            D3D12_HEAP_FLAG_NONE,
            &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(resource.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_HRESULT("ConstantBufferの生成に失敗", hr);
            return false;
        }

        hr = resource->Map(0, nullptr, &mapped);
        if (FAILED(hr))
        {
            LOG_HRESULT("ConstantBufferのマップに失敗", hr);
            return false;
        }

        ZeroMemory(mapped, AlignedSize);
        return true;
    }

    template<typename T>
    void ConstantBuffer<T>::Finalize()
    {
        if (resource && mapped)
        {
            resource->Unmap(0, nullptr);
            mapped = nullptr;
        }
        resource.Reset();
    }

    template<typename T>
    void ConstantBuffer<T>::Update(const T& data)
    {
        if (mapped)
        {
            memcpy(mapped, &data, sizeof(T));
        }
    }

    template<typename T>
    D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer<T>::GetGPUVirtualAddress() const
    {
        return resource->GetGPUVirtualAddress();
    }

    // 明示的インスタンス化
    template class ConstantBuffer<TransformBuffer>;
    template class ConstantBuffer<SpriteTransformBuffer>;
    template class ConstantBuffer<SpriteBuffer>;

} // Engine::Graphics