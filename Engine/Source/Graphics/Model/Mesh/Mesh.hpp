#pragma once
#include "Graphics/DX12/Type.hpp"
#include "Graphics/Model/ModelVertex/ModelVertex.hpp"
#include "Graphics/Model/Material/Material.hpp"
#include <vector>

namespace Engine::Graphics
{
    class Mesh
    {
    public:
        bool Initialize(
            const std::vector<ModelVertex>& vertices,
            const std::vector<uint32_t>& indices,
            const Material& material
        );
        void Finalize();

        void Draw(ID3D12GraphicsCommandList* cmdList) const;

        const Material& GetMaterial() const { return material; }
        UINT GetIndexCount() const { return indexCount; }

    private:
        dx12::ComPtr<ID3D12Resource> vertexBuffer;
        dx12::ComPtr<ID3D12Resource> indexBuffer;
        D3D12_VERTEX_BUFFER_VIEW     vertexBufferView{};
        D3D12_INDEX_BUFFER_VIEW      indexBufferView{};

        Material material;
        UINT     indexCount = 0;
    };
} // Engine::Graphics