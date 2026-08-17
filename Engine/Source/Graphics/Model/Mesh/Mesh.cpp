#include "pch/pch.h"
#include "Mesh.hpp"
#include "Graphics/DX12/Device/DX12Device.hpp"

namespace Engine::Graphics
{
    bool Mesh::Initialize(
        const std::vector<ModelVertex>& vertices,
        const std::vector<uint32_t>& indices,
        const Material& mat)
    {
        auto device = DX12Device::GetInstance().GetDevice().Get();
        material = mat;
        indexCount = static_cast<UINT>(indices.size());

        LOG_DEBUG("Mesh Initialize: vertices={}, indices={}", vertices.size(), indices.size());

        // 頂点バッファ生成
        {
            const UINT bufferSize = static_cast<UINT>(
                sizeof(ModelVertex) * vertices.size());

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
                LOG_HRESULT("Mesh: VertexBufferの生成に失敗", hr);
                return false;
            }

            void* mapped = nullptr;
            hr = vertexBuffer->Map(0, nullptr, &mapped);
            if (FAILED(hr))
            {
                LOG_HRESULT("Mesh: VertexBufferのマップに失敗", hr);
                return false;
            }
            memcpy(mapped, vertices.data(), bufferSize);
            vertexBuffer->Unmap(0, nullptr);

            vertexBufferView.BufferLocation =
                vertexBuffer->GetGPUVirtualAddress();
            vertexBufferView.SizeInBytes = bufferSize;
            vertexBufferView.StrideInBytes = sizeof(ModelVertex);
        }

        // インデックスバッファ生成
        {
            const UINT bufferSize = static_cast<UINT>(
                sizeof(uint32_t) * indices.size());

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
                LOG_HRESULT("Mesh: IndexBufferの生成に失敗", hr);
                return false;
            }

            void* mapped = nullptr;
            hr = indexBuffer->Map(0, nullptr, &mapped);
            if (FAILED(hr))
            {
                LOG_HRESULT("Mesh: IndexBufferのマップに失敗", hr);
                return false;
            }
            memcpy(mapped, indices.data(), bufferSize);
            indexBuffer->Unmap(0, nullptr);

            indexBufferView.BufferLocation =
                indexBuffer->GetGPUVirtualAddress();
            indexBufferView.SizeInBytes = bufferSize;
            indexBufferView.Format = DXGI_FORMAT_R32_UINT;
        }

        return true;
    }

    void Mesh::Finalize()
    {
        indexBuffer.Reset();
        vertexBuffer.Reset();
        indexCount = 0;
    }

    void Mesh::Draw(ID3D12GraphicsCommandList* cmdList) const
    {
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmdList->IASetVertexBuffers(0, 1, &vertexBufferView);
        cmdList->IASetIndexBuffer(&indexBufferView);
        cmdList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
        //LOG_DEBUG("DrawIndexedInstanced indexCount={}", indexCount);
    }
} // Engine::Graphics