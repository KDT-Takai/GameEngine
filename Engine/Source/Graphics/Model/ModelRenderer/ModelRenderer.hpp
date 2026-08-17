#pragma once
#include "Graphics/DX12/Type.hpp"
#include "Graphics/Shader/IShaderLoader/IShaderLoader.hpp"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/Model/Model/Model.hpp"
#include "Graphics/Texture/TextureManager/TextureManager.hpp"
#include <DirectXMath.h>

namespace Engine::Graphics
{
    struct ModelTransformBuffer
    {
        DirectX::XMFLOAT4X4 world;
        DirectX::XMFLOAT4X4 view;
        DirectX::XMFLOAT4X4 projection;
        DirectX::XMFLOAT4   color;
    };

    class ModelRenderer
    {
    public:
        bool Initialize(
            IShaderLoader& shaderLoader,
            const std::wstring& vsPath,
            const std::wstring& psPath,
            DXGI_FORMAT         rtvFormat,
            DXGI_FORMAT         dsvFormat
        );
        void Finalize();

        void Draw(
            ID3D12GraphicsCommandList* cmdList,
            const Model* model,
            const DirectX::XMMATRIX& world,
            const DirectX::XMMATRIX& view,
            const DirectX::XMMATRIX& projection,
            TextureManager& texManager
        );

    private:
        bool CreateRootSignature();
        bool CreatePipelineState(
            IShaderLoader& shaderLoader,
            const std::wstring& vsPath,
            const std::wstring& psPath,
            DXGI_FORMAT         rtvFormat,
            DXGI_FORMAT         dsvFormat
        );

        dx12::ComPtr<ID3D12RootSignature>     rootSignature;
        dx12::ComPtr<ID3D12PipelineState>     pipelineState;
        ConstantBuffer<ModelTransformBuffer>  transformBuffer;
    };
} // Engine::Graphics