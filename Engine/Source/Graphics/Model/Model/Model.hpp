#pragma once
#include "Graphics/Model/Mesh/Mesh.hpp"
#include <vector>
#include <string>
#include <memory>

namespace Engine::Graphics
{
    class Model
    {
    public:
        void Finalize();

        void AddMesh(std::unique_ptr<Mesh> mesh);
        void Draw(ID3D12GraphicsCommandList* cmdList) const;

        const std::vector<std::unique_ptr<Mesh>>& GetMeshes() const
        {
            return meshes;
        }

        const std::string& GetName() const { return name; }
        void SetName(const std::string& n) { name = n; }

    private:
        std::vector<std::unique_ptr<Mesh>> meshes;
        std::string                        name;

        // è´óàí«â¡
        // std::vector<Animation> animations;
        // std::vector<Bone>      bones;
    };
} // Engine::Graphics