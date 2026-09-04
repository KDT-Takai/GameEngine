#include "pch/pch.h"
#include "Model.hpp"

namespace Engine::Graphics
{
    void Model::Finalize()
    {
        for (auto& mesh : meshes)
        {
            mesh->Finalize();
        }
        meshes.clear();
    }

    void Model::AddMesh(std::unique_ptr<Mesh> mesh)
    {
        meshes.push_back(std::move(mesh));
    }

    void Model::Draw(ID3D12GraphicsCommandList* cmdList) const
    {
        for (const auto& mesh : meshes)
        {
            mesh->Draw(cmdList);
        }
    }
} // Engine::Graphics