// Source/Graphics/Sprite/SpriteRenderer.cpp
#include "pch/pch.h"
#include "SpriteRenderer.hpp"
#include "Graphics/DX12/Device/DX12Device.hpp"
#include "Graphics/DX12/Descriptor/DX12DescriptorHeapManager.hpp"

namespace Engine::Graphics
{
    // 頂点構造体
    struct SpriteVertex
    {
        float position[3];
        float uv[2];
    };

    bool SpriteRenderer::Initialize(
        IShaderLoader& shaderLoader,
        DXGI_FORMAT    rtvFormat,
        DXGI_FORMAT    dsvFormat)
    {
        auto device = DX12Device::GetInstance().GetDevice().Get();

        if (!CreateRootSignature())
        {
            LOG_ERROR("SpriteRenderer: RootSignatureの生成に失敗");
            return false;
        }
        if (!CreatePipelineState(shaderLoader, rtvFormat, dsvFormat))
        {
            LOG_ERROR("SpriteRenderer: PipelineStateの生成に失敗");
            return false;
        }
        if (!CreateQuadBuffer())
        {
            LOG_ERROR("SpriteRenderer: QuadBufferの生成に失敗");
            return false;
        }

        if (!transformBuffer.Initialize(device))
        {
            LOG_ERROR("SpriteRenderer: TransformBufferの初期化に失敗");
            return false;
        }
        if (!spriteBuffer.Initialize(device))
        {
            LOG_ERROR("SpriteRenderer: SpriteBufferの初期化に失敗");
            return false;
        }

        LOG_INFO("SpriteRendererの初期化に成功");
        return true;
    }

    void SpriteRenderer::Finalize()
    {
        spriteBuffer.Finalize();
        transformBuffer.Finalize();
        indexBuffer.Reset();
        vertexBuffer.Reset();
        pipelineState.Reset();
        rootSignature.Reset();
    }

    void SpriteRenderer::Draw(
        ID3D12GraphicsCommandList* cmdList,
        const DirectX::XMMATRIX& wvp,
        const SpriteComponent& sprite,
        const Texture* texture)
    {
        if (!sprite.visible) return;
        if (!texture) return;

        // TransformBuffer 更新
        SpriteTransformBuffer transformData{};
        DirectX::XMStoreFloat4x4(&transformData.wvp, wvp);
        transformBuffer.Update(transformData);

        // SpriteBuffer 更新
        SpriteBuffer spriteData{};
        spriteData.color = sprite.color;
        spriteData.uvRect = sprite.uvRect;
        spriteBuffer.Update(spriteData);

        // RootSignature・PSO
        cmdList->SetGraphicsRootSignature(rootSignature.Get());
        cmdList->SetPipelineState(pipelineState.Get());

        // DescriptorHeap をセット
        auto* heap = DX12DescriptorHeapManager::GetInstance()
            .GetCbvSrvUavAllocator().GetHeap();
        cmdList->SetDescriptorHeaps(1, &heap);

        // b0: TransformBuffer
        cmdList->SetGraphicsRootConstantBufferView(
            0, transformBuffer.GetGPUVirtualAddress());

        // b1: SpriteBuffer
        cmdList->SetGraphicsRootConstantBufferView(
            1, spriteBuffer.GetGPUVirtualAddress());

        // t0: SRV（テクスチャ）
        cmdList->SetGraphicsRootDescriptorTable(
            2, texture->GetHandle().gpuHandle);

        // VB・IB
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
        cmdList->IASetIndexBuffer(&indexBufferView);

        // 描画
        cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }

    bool SpriteRenderer::CreateRootSignature()
    {
        auto device = DX12Device::GetInstance().GetDevice().Get();

        // b0: TransformBuffer
        D3D12_ROOT_PARAMETER params[3]{};
        params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        params[0].Descriptor.ShaderRegister = 0;
        params[0].Descriptor.RegisterSpace = 0;
        params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

        // b1: SpriteBuffer
        params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        params[1].Descriptor.ShaderRegister = 1;
        params[1].Descriptor.RegisterSpace = 0;
        params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // t0: SRV（DescriptorTable）
        D3D12_DESCRIPTOR_RANGE srvRange{};
        srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvRange.NumDescriptors = 1;
        srvRange.BaseShaderRegister = 0;
        srvRange.RegisterSpace = 0;
        srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        params[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[2].DescriptorTable.NumDescriptorRanges = 1;
        params[2].DescriptorTable.pDescriptorRanges = &srvRange;
        params[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        // s0: StaticSampler
        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        D3D12_ROOT_SIGNATURE_DESC desc{};
        desc.NumParameters = _countof(params);
        desc.pParameters = params;
        desc.NumStaticSamplers = 1;
        desc.pStaticSamplers = &sampler;
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        dx12::ComPtr<ID3DBlob> sigBlob;
        dx12::ComPtr<ID3DBlob> errBlob;
        HRESULT hr = D3D12SerializeRootSignature(
            &desc,
            D3D_ROOT_SIGNATURE_VERSION_1,
            sigBlob.GetAddressOf(),
            errBlob.GetAddressOf()
        );
        if (FAILED(hr))
        {
            if (errBlob)
            {
                LOG_ERROR("{}", static_cast<const char*>(errBlob->GetBufferPointer()));
            }
            return false;
        }

        hr = device->CreateRootSignature(
            0,
            sigBlob->GetBufferPointer(),
            sigBlob->GetBufferSize(),
            IID_PPV_ARGS(rootSignature.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_ERROR("SpriteRenderer: RootSignatureの生成に失敗: {}", HR_LOG(hr));
            return false;
        }

        return true;
    }

    bool SpriteRenderer::CreatePipelineState(
        IShaderLoader& shaderLoader,
        DXGI_FORMAT    rtvFormat,
        DXGI_FORMAT    dsvFormat)
    {
        dx12::ComPtr<ID3DBlob> vsBlob;
        dx12::ComPtr<ID3DBlob> psBlob;

        ShaderLoadDesc vsDesc;
        vsDesc.path = L"../Engine/Assets/Shader/Sprite.hlsl";
        vsDesc.entryPoint = "VSMain";
        vsDesc.target = "vs_5_0";

        ShaderLoadDesc psDesc;
        psDesc.path = L"../Engine/Assets/Shader/Sprite.hlsl";
        psDesc.entryPoint = "PSMain";
        psDesc.target = "ps_5_0";

        if (!shaderLoader.Load(vsDesc, vsBlob.GetAddressOf()))
        {
            LOG_ERROR("SpriteRenderer: 頂点シェーダのコンパイルに失敗");
            return false;
        }
        if (!shaderLoader.Load(psDesc, psBlob.GetAddressOf()))
        {
            LOG_ERROR("SpriteRenderer: ピクセルシェーダのコンパイルに失敗");
            return false;
        }

        D3D12_INPUT_ELEMENT_DESC inputLayout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_RASTERIZER_DESC rasterizerDesc{};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;  // スプライトは両面描画
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthClipEnable = TRUE;

        // アルファブレンド（半透明対応）
        D3D12_BLEND_DESC blendDesc{};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
        depthStencilDesc.DepthEnable = TRUE;
        depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
        psoDesc.pRootSignature = rootSignature.Get();
        psoDesc.VS = { vsBlob->GetBufferPointer(), vsBlob->GetBufferSize() };
        psoDesc.PS = { psBlob->GetBufferPointer(), psBlob->GetBufferSize() };
        psoDesc.InputLayout = { inputLayout, _countof(inputLayout) };
        psoDesc.RasterizerState = rasterizerDesc;
        psoDesc.BlendState = blendDesc;
        psoDesc.DepthStencilState = depthStencilDesc;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = rtvFormat;
        psoDesc.DSVFormat = dsvFormat;
        psoDesc.SampleDesc.Count = 1;

        auto device = DX12Device::GetInstance().GetDevice().Get();
        HRESULT hr = device->CreateGraphicsPipelineState(
            &psoDesc,
            IID_PPV_ARGS(pipelineState.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_ERROR("SpriteRenderer: PipelineStateの生成に失敗: {}", HR_LOG(hr));
            return false;
        }

        return true;
    }

    bool SpriteRenderer::CreateQuadBuffer()
    {
        auto device = DX12Device::GetInstance().GetDevice().Get();

        // 頂点データ（中心原点・1x1サイズ）
        SpriteVertex vertices[] =
        {
            { { -0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f } }, // 左上
            { {  0.5f,  0.5f, 0.0f }, { 1.0f, 0.0f } }, // 右上
            { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } }, // 左下
            { {  0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } }, // 右下
        };

        uint16_t indices[] = { 0, 1, 2, 1, 3, 2 };

        // 頂点バッファ生成
        {
            const UINT bufferSize = sizeof(vertices);

            D3D12_HEAP_PROPERTIES heapProp{};
            heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

            D3D12_RESOURCE_DESC bufferDesc{};
            bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bufferDesc.Width = bufferSize;
            bufferDesc.Height = 1;
            bufferDesc.DepthOrArraySize = 1;
            bufferDesc.MipLevels = 1;
            bufferDesc.SampleDesc.Count = 1;
            bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

            HRESULT hr = device->CreateCommittedResource(
                &heapProp,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(vertexBuffer.GetAddressOf())
            );
            if (FAILED(hr))
            {
                LOG_ERROR("SpriteRenderer: RootSignatureの生成に失敗: 0x{:08X}", static_cast<uint32_t>(hr));
                return false;
            }

            void* mapped = nullptr;
            hr = vertexBuffer->Map(0, nullptr, &mapped);
            if (FAILED(hr))
            {
                LOG_ERROR("SpriteRenderer: VertexBufferの生成に失敗: 0x{:08X}", static_cast<uint32_t>(hr));
                return false;
            }
            memcpy(mapped, vertices, bufferSize);
            vertexBuffer->Unmap(0, nullptr);

            vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
            vertexBufferView.SizeInBytes = bufferSize;
            vertexBufferView.StrideInBytes = sizeof(SpriteVertex);
        }

        // インデックスバッファ生成
        {
            const UINT bufferSize = sizeof(indices);

            D3D12_HEAP_PROPERTIES heapProp{};
            heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

            D3D12_RESOURCE_DESC bufferDesc{};
            bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            bufferDesc.Width = bufferSize;
            bufferDesc.Height = 1;
            bufferDesc.DepthOrArraySize = 1;
            bufferDesc.MipLevels = 1;
            bufferDesc.SampleDesc.Count = 1;
            bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

            HRESULT hr = device->CreateCommittedResource(
                &heapProp,
                D3D12_HEAP_FLAG_NONE,
                &bufferDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(indexBuffer.GetAddressOf())
            );
            if (FAILED(hr))
            {
                LOG_ERROR("SpriteRenderer: VertexBufferの生成に失敗: 0x{:08X}", static_cast<uint32_t>(hr));
                return false;
            }

            void* mapped = nullptr;
            hr = indexBuffer->Map(0, nullptr, &mapped);
            if (FAILED(hr))
            {
                LOG_ERROR("SpriteRenderer: IndexBufferの生成に失敗: 0x{:08X}", static_cast<uint32_t>(hr));
                return false;
            }
            memcpy(mapped, indices, bufferSize);
            indexBuffer->Unmap(0, nullptr);

            indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
            indexBufferView.SizeInBytes = bufferSize;
            indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        }

        return true;
    }
} // Engine::Graphics