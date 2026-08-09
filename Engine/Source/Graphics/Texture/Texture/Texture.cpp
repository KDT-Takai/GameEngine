#include "pch/pch.h"
#include "Texture.hpp"
#include "Graphics/DX12/Device/DX12Device.hpp"
#include "Graphics/DX12/Descriptor/DX12DescriptorHeapManager.hpp"
#include <DirectXTex.h>

namespace Engine::Graphics
{
    bool Texture::Initialize(
        ID3D12GraphicsCommandList* cmdList,
        const std::wstring& path)
    {
        auto device = DX12Device::GetInstance().GetDevice().Get();

        // ファイル読み込み
        DirectX::ScratchImage image;
        HRESULT hr = DirectX::LoadFromWICFile(
            path.c_str(),
            DirectX::WIC_FLAGS_NONE,
            nullptr,
            image
        );
        if (FAILED(hr))
        {
            LOG_HRESULT("テクスチャの読み込みに失敗", hr);
            return false;
        }

        const DirectX::Image* img = image.GetImage(0, 0, 0);
        width = static_cast<UINT>(img->width);
        height = static_cast<UINT>(img->height);

        // DEFAULTヒープにGPUリソース作成
        D3D12_HEAP_PROPERTIES defaultHeap{};
        defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC texDesc{};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = img->format;
        texDesc.SampleDesc.Count = 1;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

        hr = device->CreateCommittedResource(
            &defaultHeap,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(resource.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_HRESULT("テクスチャリソースの生成に失敗", hr);
            return false;
        }

        // UPLOADヒープに一時バッファ作成
        UINT64 uploadSize = 0;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
        device->GetCopyableFootprints(
            &texDesc, 0, 1, 0,
            &footprint, nullptr, nullptr, &uploadSize
        );

        D3D12_HEAP_PROPERTIES uploadHeapProps{};
        uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC uploadDesc{};
        uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadDesc.Width = uploadSize;
        uploadDesc.Height = 1;
        uploadDesc.DepthOrArraySize = 1;
        uploadDesc.MipLevels = 1;
        uploadDesc.SampleDesc.Count = 1;
        uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        hr = device->CreateCommittedResource(
            &uploadHeapProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(uploadBuffer.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_HRESULT("アップロードバッファの生成に失敗", hr);
            return false;
        }

        // アップロードバッファへ書き込み
        void* mapped = nullptr;
        hr = uploadBuffer->Map(0, nullptr, &mapped);
        if (FAILED(hr))
        {
            LOG_HRESULT("アップロードバッファのマップに失敗", hr);
            return false;
        }

        const BYTE* src = img->pixels;
        BYTE* dst = static_cast<BYTE*>(mapped) + footprint.Offset;
        for (UINT y = 0; y < height; ++y)
        {
            memcpy(
                dst + static_cast<UINT64>(y) * footprint.Footprint.RowPitch,
                src + static_cast<UINT64>(y) * img->rowPitch,
                img->rowPitch
            );
        }
        uploadBuffer->Unmap(0, nullptr);

        // DEFAULTヒープへコピーコマンドを積む
        D3D12_TEXTURE_COPY_LOCATION dstLoc{};
        dstLoc.pResource = resource.Get();
        dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLoc.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION srcLoc{};
        srcLoc.pResource = uploadBuffer.Get();
        srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcLoc.PlacedFootprint = footprint;

        cmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

        // リソースバリア（COPY_DEST → PIXEL_SHADER_RESOURCE）
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = resource.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &barrier);

        // SRV作成
        handle = DX12DescriptorHeapManager::GetInstance().GetCbvSrvUavAllocator().Allocate();
        if (!handle.IsValid())
        {
            LOG_ERROR("SRVスロットの確保に失敗");
            return false;
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = img->format;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;

        device->CreateShaderResourceView(
            resource.Get(),
            &srvDesc,
            handle.cpuHandle
        );

        LOG_INFO("テクスチャの読み込みに成功: {}x{}", width, height);
        return true;
    }

    void Texture::Finalize()
    {
        ReleaseUploadBuffer();
        DX12DescriptorHeapManager::GetInstance().GetCbvSrvUavAllocator().Free(handle);
        resource.Reset();
        handle = DescriptorHandle{};
        width = 0;
        height = 0;
    }

    void Texture::ReleaseUploadBuffer()
    {
        uploadBuffer.Reset();
    }
    
    void Texture::InitializeFromResource(dx12::ComPtr<ID3D12Resource> res, const DescriptorHandle& handle, UINT w, UINT h)
    {
        this->resource = std::move(res);
        this->handle = handle;
        this->width = w;
        this->height = h;
    }
} // Engine::Graphics