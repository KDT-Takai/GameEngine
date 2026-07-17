#include "pch/pch.h"
#include "TextureManager.hpp"
#include "Graphics/DX12/Device/DX12Device.hpp"
#include "Graphics/DX12/Descriptor/DX12DescriptorHeapManager.hpp"
#include "Graphics/DX12/RendererContext/DX12RendererContext.hpp"
#include "Utility/EngineContext/EngineContext.hpp"

namespace Engine::Graphics
{
    bool TextureManager::Initialize()
    {
        auto device = DX12Device::GetInstance().GetDevice().Get();

        // cmdAllocator 生成
        HRESULT hr = device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(cmdAllocator.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: CommandAllocatorの生成に失敗: {}", HR_LOG(hr));
            return false;
        }

        // cmdList 生成
        hr = device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            cmdAllocator.Get(),
            nullptr,
            IID_PPV_ARGS(cmdList.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: CommandListの生成に失敗: {}", HR_LOG(hr));
            return false;
        }

        // 生成直後はClose状態にする
        hr = cmdList->Close();
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: CommandListのCloseに失敗: {}", HR_LOG(hr));
            return false;
        }

        // fence 生成
        hr = device->CreateFence(
            0,
            D3D12_FENCE_FLAG_NONE,
            IID_PPV_ARGS(fence.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: Fenceの生成に失敗: {}", HR_LOG(hr));
            return false;
        }

        // fenceEvent 生成
        fenceEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        if (fenceEvent == nullptr)
        {
            LOG_ERROR("TextureManager: FenceEventの生成に失敗");
            return false;
        }

        // フォールバック用1x1ホワイトテクスチャ生成
        if (!CreateFallback())
        {
            LOG_ERROR("TextureManager: Fallbackテクスチャの生成に失敗");
            return false;
        }

        LOG_INFO("TextureManagerの初期化に成功");
        return true;
    }

    void TextureManager::Finalize()
    {
        // 全テクスチャ破棄
        for (auto& [id, texture] : textures)
        {
            texture->Finalize();
        }
        textures.clear();

        // フォールバック破棄
        if (fallback)
        {
            fallback->Finalize();
            fallback.reset();
        }

        // コマンド関連破棄
        if (fenceEvent)
        {
            CloseHandle(fenceEvent);
            fenceEvent = nullptr;
        }
        fence.Reset();
        cmdList.Reset();
        cmdAllocator.Reset();
    }

    TextureID TextureManager::Load(const std::wstring& path)
    {
        // ハッシュで TextureID 生成
        TextureID id = std::hash<std::wstring>{}(path);

        // キャッシュ確認
        if (textures.contains(id))
        {
            return id;
        }

        // cmdAllocator・cmdList リセット
        HRESULT hr = cmdAllocator->Reset();
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: CommandAllocatorのResetに失敗: {}", HR_LOG(hr));
            return InvalidTextureID;
        }

        hr = cmdList->Reset(cmdAllocator.Get(), nullptr);
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: CommandListのResetに失敗: {}", HR_LOG(hr));
            return InvalidTextureID;
        }

        // テクスチャ生成
        auto texture = std::make_unique<Texture>();
        if (!texture->Initialize(cmdList.Get(), path))
        {
            LOG_ERROR("TextureManager: テクスチャの初期化に失敗: {}",
                std::string(path.begin(), path.end()));
            return InvalidTextureID;
        }

        // GPU転送・待機
        SubmitAndWait();

        // アップロードバッファ解放
        texture->ReleaseUploadBuffer();

        // キャッシュ登録
        textures[id] = std::move(texture);

        LOG_INFO("TextureManager: テクスチャのロードに成功: {}",
            std::string(path.begin(), path.end()));

        return id;
    }

    const Texture* TextureManager::Get(TextureID id) const
    {
        if (id == InvalidTextureID)
        {
            return fallback.get();
        }

        auto it = textures.find(id);
        if (it == textures.end())
        {
            return fallback.get();
        }

        return it->second.get();
    }

    void TextureManager::SubmitAndWait()
    {
        // cmdList を Close
        HRESULT hr = cmdList->Close();
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: CommandListのCloseに失敗: {}", HR_LOG(hr));
            return;
        }

        // EngineContext から cmdQueue を取得
        auto* ctx = GET_CONTEXT(DX12RendererContext);
        if (!ctx)
        {
            LOG_ERROR("TextureManager: DX12RendererContextが取得できません");
            return;
        }
        auto cmdQueue = ctx->GetCmdQueue();

        // コマンド実行
        ID3D12CommandList* ppCmdLists[] = { cmdList.Get() };
        cmdQueue->ExecuteCommandLists(_countof(ppCmdLists), ppCmdLists);

        // Fence でシグナル・待機
        ++fenceValue;
        hr = cmdQueue->Signal(fence.Get(), fenceValue);
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: Fenceのシグナルに失敗: {}", HR_LOG(hr));
            return;
        }

        if (fence->GetCompletedValue() < fenceValue)
        {
            fence->SetEventOnCompletion(fenceValue, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
        }
    }

    bool TextureManager::CreateFallback()
    {
        // cmdAllocator・cmdList リセット
        HRESULT hr = cmdAllocator->Reset();
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: CommandAllocatorのResetに失敗: {}", HR_LOG(hr));
            return false;
        }

        hr = cmdList->Reset(cmdAllocator.Get(), nullptr);
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: CommandListのResetに失敗: {}", HR_LOG(hr));
            return false;
        }

        auto device = DX12Device::GetInstance().GetDevice().Get();

        // DEFAULTヒープに1x1テクスチャ生成
        D3D12_HEAP_PROPERTIES defaultHeap{};
        defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_RESOURCE_DESC texDesc{};
        texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        texDesc.Width = 1;
        texDesc.Height = 1;
        texDesc.DepthOrArraySize = 1;
        texDesc.MipLevels = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

        dx12::ComPtr<ID3D12Resource> fallbackResource;
        hr = device->CreateCommittedResource(
            &defaultHeap,
            D3D12_HEAP_FLAG_NONE,
            &texDesc,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr,
            IID_PPV_ARGS(fallbackResource.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: Fallbackリソースの生成に失敗: {}", HR_LOG(hr));
            return false;
        }

        // UPLOADヒープに一時バッファ生成
        UINT64 uploadSize = 0;
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
        device->GetCopyableFootprints(
            &texDesc, 0, 1, 0,
            &footprint, nullptr, nullptr, &uploadSize
        );

        D3D12_HEAP_PROPERTIES uploadHeap{};
        uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC uploadDesc{};
        uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadDesc.Width = uploadSize;
        uploadDesc.Height = 1;
        uploadDesc.DepthOrArraySize = 1;
        uploadDesc.MipLevels = 1;
        uploadDesc.SampleDesc.Count = 1;
        uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        dx12::ComPtr<ID3D12Resource> uploadBuffer;
        hr = device->CreateCommittedResource(
            &uploadHeap,
            D3D12_HEAP_FLAG_NONE,
            &uploadDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(uploadBuffer.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: Fallbackアップロードバッファの生成に失敗: {}", HR_LOG(hr));
            return false;
        }

        // ホワイトピクセルを書き込む
        void* mapped = nullptr;
        hr = uploadBuffer->Map(0, nullptr, &mapped);
        if (FAILED(hr))
        {
            LOG_ERROR("TextureManager: Fallbackアップロードバッファのマップに失敗");
            return false;
        }
        uint32_t white = 0xFFFFFFFF;
        memcpy(static_cast<BYTE*>(mapped) + footprint.Offset, &white, sizeof(white));
        uploadBuffer->Unmap(0, nullptr);

        // DEFAULTヒープへコピー
        D3D12_TEXTURE_COPY_LOCATION dstLoc{};
        dstLoc.pResource = fallbackResource.Get();
        dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        dstLoc.SubresourceIndex = 0;

        D3D12_TEXTURE_COPY_LOCATION srcLoc{};
        srcLoc.pResource = uploadBuffer.Get();
        srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        srcLoc.PlacedFootprint = footprint;

        cmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

        // リソースバリア
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = fallbackResource.Get();
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cmdList->ResourceBarrier(1, &barrier);

        // SRV作成
        auto handle = DX12DescriptorHeapManager::GetInstance()
            .GetCbvSrvUavAllocator().Allocate();
        if (!handle.IsValid())
        {
            LOG_ERROR("TextureManager: FallbackのSRVスロット確保に失敗");
            return false;
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Texture2D.MipLevels = 1;

        device->CreateShaderResourceView(
            fallbackResource.Get(),
            &srvDesc,
            handle.cpuHandle
        );

        // GPU転送・待機
        SubmitAndWait();

        // fallback Texture を構築
        fallback = std::make_unique<Texture>();
        fallback->InitializeFromResource(
            std::move(fallbackResource),
            handle,
            1, 1
        );

        LOG_INFO("TextureManager: Fallbackテクスチャの生成に成功");
        return true;
    }

    void TextureManager::AddContext()
    {
        REGISTER_CONTEXT(*this);
    }
} // Engine::Graphics