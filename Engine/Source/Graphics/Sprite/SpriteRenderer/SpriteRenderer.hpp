#pragma once
#include "Graphics/Component/SpriteComponent/SpriteComponent.hpp"
#include "Graphics/Component/TransformComponent/TransformComponent.hpp"
#include "Graphics/Texture/Texture/Texture.hpp"
#include "Graphics/Shader/IShaderLoader/IShaderLoader.hpp"
#include "Graphics/ConstantBuffer/ConstantBuffer.hpp"
#include "Graphics/DX12/Type.hpp"
#include <DirectXMath.h>

namespace Engine::Graphics
{
    // 定数バッファ構造体
    struct SpriteTransformBuffer
    {
        DirectX::XMFLOAT4X4 wvp;
    };

    struct SpriteBuffer
    {
        DirectX::XMFLOAT4 color;
        DirectX::XMFLOAT4 uvRect;
    };

    class SpriteRenderer
    {
    public:
        bool Initialize(
            IShaderLoader& shaderLoader,
            DXGI_FORMAT    rtvFormat,
            DXGI_FORMAT    dsvFormat
        );
        void Finalize();

        void Draw(
            ID3D12GraphicsCommandList* cmdList,
            const DirectX::XMMATRIX& wvp,
            const SpriteComponent& sprite,
            const Texture* texture
        );

    private:
        bool CreateRootSignature();
        bool CreatePipelineState(
            IShaderLoader& shaderLoader,
            DXGI_FORMAT    rtvFormat,
            DXGI_FORMAT    dsvFormat
        );
        bool CreateQuadBuffer();

        dx12::ComPtr<ID3D12RootSignature> rootSignature;
        dx12::ComPtr<ID3D12PipelineState> pipelineState;

        // 共有クワッドVB・IB
        dx12::ComPtr<ID3D12Resource> vertexBuffer;
        dx12::ComPtr<ID3D12Resource> indexBuffer;
        D3D12_VERTEX_BUFFER_VIEW     vertexBufferView{};
        D3D12_INDEX_BUFFER_VIEW      indexBufferView{};

        // 定数バッファ
        ConstantBuffer<SpriteTransformBuffer> transformBuffer;
        ConstantBuffer<SpriteBuffer>          spriteBuffer;
    };
} // Engine::Graphics