#include "pch/pch.h"
#include "ModelRenderer.hpp"
#include "Graphics/DX12/Device/DX12Device.hpp"
#include "Graphics/DX12/Descriptor/DX12DescriptorHeapManager.hpp"

namespace Engine::Graphics
{
    bool ModelRenderer::Initialize(
        IShaderLoader& shaderLoader,
        const std::wstring& vsPath,
        const std::wstring& psPath,
        DXGI_FORMAT         rtvFormat,
        DXGI_FORMAT         dsvFormat)
    {
        auto device = DX12Device::GetInstance().GetDevice().Get();

        if (!CreateRootSignature())
        {
            LOG_ERROR("ModelRenderer: RootSignatureの生成に失敗");
            return false;
        }
        if (!CreatePipelineState(shaderLoader, vsPath, psPath, rtvFormat, dsvFormat))
        {
            LOG_ERROR("ModelRenderer: PipelineStateの生成に失敗");
            return false;
        }
        if (!transformBuffer.Initialize(device))
        {
            LOG_ERROR("ModelRenderer: TransformBufferの初期化に失敗");
            return false;
        }

        LOG_INFO("ModelRendererの初期化に成功");
        return true;
    }

    void ModelRenderer::Finalize()
    {
        transformBuffer.Finalize();
        pipelineState.Reset();
        rootSignature.Reset();
    }

    void ModelRenderer::Draw(
        ID3D12GraphicsCommandList* cmdList,
        const Model* model,
        const DirectX::XMMATRIX& world,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& projection,
        TextureManager& texManager)
    {
        //LOG_DEBUG("ModelRenderer::Draw 開始");
        if (!model) return;

        // TransformBuffer 更新
        ModelTransformBuffer data{};
        DirectX::XMStoreFloat4x4(&data.world, world);
        DirectX::XMStoreFloat4x4(&data.view, view);
        DirectX::XMStoreFloat4x4(&data.projection, projection);

        // RootSignature・PSO
        cmdList->SetGraphicsRootSignature(rootSignature.Get());
        cmdList->SetPipelineState(pipelineState.Get());

        // DescriptorHeap をセット
        auto* heap = DX12DescriptorHeapManager::GetInstance().GetCbvSrvUavAllocator().GetHeap();
        cmdList->SetDescriptorHeaps(1, &heap);

        // メッシュごとに描画
        for (const auto& mesh : model->GetMeshes())
        {
            const auto& material = mesh->GetMaterial();

            // カラーを設定
            data.color = material.color;
            transformBuffer.Update(data);

            // b0: TransformBuffer
            cmdList->SetGraphicsRootConstantBufferView(
                0, transformBuffer.GetGPUVirtualAddress());

            // t0: SRV（テクスチャ）
            const Texture* texture = texManager.Get(material.diffuseTexture);
            cmdList->SetGraphicsRootDescriptorTable(
                1, texture->GetHandle().gpuHandle);

            //LOG_DEBUG("ModelRenderer: mesh描画開始");
            mesh->Draw(cmdList);
        }
    }

    bool ModelRenderer::CreateRootSignature()
    {
        auto device = DX12Device::GetInstance().GetDevice().Get();

        // b0: TransformBuffer
        D3D12_ROOT_PARAMETER params[2]{};
        params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        params[0].Descriptor.ShaderRegister = 0;
        params[0].Descriptor.RegisterSpace = 0;
        params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

        // t0: SRV（DescriptorTable）
        D3D12_DESCRIPTOR_RANGE srvRange{};
        srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        srvRange.NumDescriptors = 1;
        srvRange.BaseShaderRegister = 0;
        srvRange.RegisterSpace = 0;
        srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        params[1].DescriptorTable.NumDescriptorRanges = 1;
        params[1].DescriptorTable.pDescriptorRanges = &srvRange;
        params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

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
                LOG_ERROR("{}", static_cast<const char*>(
                    errBlob->GetBufferPointer()));
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
            LOG_HRESULT("ModelRenderer: RootSignatureの生成に失敗", hr);
            return false;
        }

        return true;
    }

    bool ModelRenderer::CreatePipelineState(
        IShaderLoader& shaderLoader,
        const std::wstring& vsPath,
        const std::wstring& psPath,
        DXGI_FORMAT         rtvFormat,
        DXGI_FORMAT         dsvFormat)
    {
        dx12::ComPtr<ID3DBlob> vsBlob;
        dx12::ComPtr<ID3DBlob> psBlob;

        ShaderLoadDesc vsDesc;
        vsDesc.path = vsPath;
        vsDesc.entryPoint = "VSMain";
        vsDesc.target = "vs_5_0";

        ShaderLoadDesc psDesc;
        psDesc.path = psPath;
        psDesc.entryPoint = "PSMain";
        psDesc.target = "ps_5_0";

        if (!shaderLoader.Load(vsDesc, vsBlob.GetAddressOf()))
        {
            LOG_ERROR("ModelRenderer: 頂点シェーダのコンパイルに失敗");
            return false;
        }
        if (!shaderLoader.Load(psDesc, psBlob.GetAddressOf()))
        {
            LOG_ERROR("ModelRenderer: ピクセルシェーダのコンパイルに失敗");
            return false;
        }

        D3D12_INPUT_ELEMENT_DESC inputLayout[] =
        {
            { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_RASTERIZER_DESC rasterizerDesc{};
        rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
        //rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthClipEnable = TRUE;

        D3D12_BLEND_DESC blendDesc{};
        blendDesc.RenderTarget[0].BlendEnable = FALSE;
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

        //LOG_DEBUG("ModelRenderer PSO作成開始");
        //LOG_DEBUG("RootSignature: {}", (void*)psoDesc.pRootSignature);
        //LOG_DEBUG("VS size: {}", psoDesc.VS.BytecodeLength);
        //LOG_DEBUG("PS size: {}", psoDesc.PS.BytecodeLength);
        //LOG_DEBUG("InputLayout count: {}", psoDesc.InputLayout.NumElements);
        //LOG_DEBUG("RTVFormat: {}", static_cast<uint32_t>(psoDesc.RTVFormats[0]));
        //LOG_DEBUG("DSVFormat: {}", static_cast<uint32_t>(psoDesc.DSVFormat));
        //LOG_DEBUG("NumRenderTargets: {}", psoDesc.NumRenderTargets);

        auto device = DX12Device::GetInstance().GetDevice().Get();
        HRESULT hr = device->CreateGraphicsPipelineState(
            &psoDesc,
            IID_PPV_ARGS(pipelineState.GetAddressOf())
        );
        if (FAILED(hr))
        {
            LOG_HRESULT("ModelRenderer: PipelineStateの生成に失敗", hr);
            return false;
        }

        return true;
    }
} // Engine::Graphics