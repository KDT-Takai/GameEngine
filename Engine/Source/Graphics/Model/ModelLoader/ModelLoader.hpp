#pragma once
#include "Graphics/Model/Model/Model.hpp"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <memory>
#include <string>

namespace Engine::Graphics
{
    class ModelLoader
    {
    public:
        // モデルファイルを読み込んでModelを返す
        std::unique_ptr<Model> Load(const std::wstring& path);

    private:
        // aiMeshをMeshに変換
        std::unique_ptr<Mesh> ProcessMesh(
            const struct aiMesh* mesh,
            const struct aiScene* scene,
            const std::string& directory
        );

        // マテリアル情報を取得
        Material ProcessMaterial(
            const struct aiMaterial* material,
            const std::string& directory
        );
    };
} // Engine::Graphics