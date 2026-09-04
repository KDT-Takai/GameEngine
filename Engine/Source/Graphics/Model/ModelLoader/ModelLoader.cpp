// Source/Graphics/Model/ModelLoader/ModelLoader.cpp
#include "pch/pch.h"
#include "ModelLoader.hpp"
#include "System/Assets/AssetManager/AssetManager.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace Engine::Graphics
{
    std::unique_ptr<Model> ModelLoader::Load(const std::wstring& path)
    {
        Assimp::Importer importer;

        std::string pathStr(path.begin(), path.end());

        const aiScene* scene = importer.ReadFile(
            pathStr,
            aiProcess_Triangulate |  // 三角形化
            aiProcess_CalcTangentSpace |  // tangent・bitangent自動計算
            aiProcess_FlipUVs |  // UV上下反転（DirectX用）
            aiProcess_JoinIdenticalVertices | // 重複頂点削除
            //aiProcess_GenNormals            // 法線自動生成
            aiProcess_GenSmoothNormals
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            LOG_ERROR("ModelLoader: モデルの読み込みに失敗: {} - {}", pathStr, importer.GetErrorString());
            return nullptr;
        }

        auto model = std::make_unique<Model>();

        // ファイル名をモデル名に設定
        std::filesystem::path p(path);
        model->SetName(p.stem().string());

        // ディレクトリパスを取得（テクスチャロード用）
        std::string directory = std::filesystem::path(pathStr)
            .parent_path().string();

        // 全メッシュを処理
        for (UINT i = 0; i < scene->mNumMeshes; ++i)
        {
            auto mesh = ProcessMesh(scene->mMeshes[i], scene, directory);
            if (mesh)
            {
                model->AddMesh(std::move(mesh));
            }
        }

        LOG_INFO("ModelLoader: モデルの読み込みに成功: {} ({}メッシュ)",
            pathStr, scene->mNumMeshes);

        return model;
    }

    std::unique_ptr<Mesh> ModelLoader::ProcessMesh(
        const aiMesh* aimesh,
        const aiScene* scene,
        const std::string& directory)
    {
        std::vector<ModelVertex> vertices;
        std::vector<uint32_t>    indices;

        // 頂点データの変換
        for (UINT i = 0; i < aimesh->mNumVertices; ++i)
        {
            ModelVertex vertex{};

            // 位置
            vertex.position = {
                aimesh->mVertices[i].x,
                aimesh->mVertices[i].y,
                aimesh->mVertices[i].z
            };

            // 法線
            if (aimesh->HasNormals())
            {
                vertex.normal = {
                    aimesh->mNormals[i].x,
                    aimesh->mNormals[i].y,
                    aimesh->mNormals[i].z
                };
            }

            // UV
            if (aimesh->mTextureCoords[0])
            {
                vertex.uv = {
                    aimesh->mTextureCoords[0][i].x,
                    aimesh->mTextureCoords[0][i].y
                };
            }

            // tangent・bitangent
            if (aimesh->HasTangentsAndBitangents())
            {
                vertex.tangent = {
                    aimesh->mTangents[i].x,
                    aimesh->mTangents[i].y,
                    aimesh->mTangents[i].z
                };
                vertex.bitangent = {
                    aimesh->mBitangents[i].x,
                    aimesh->mBitangents[i].y,
                    aimesh->mBitangents[i].z
                };
            }

            vertices.push_back(vertex);
        }

        // インデックスデータの変換
        for (UINT i = 0; i < aimesh->mNumFaces; ++i)
        {
            const aiFace& face = aimesh->mFaces[i];
            for (UINT j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        // マテリアルの取得
        Material material;
        if (aimesh->mMaterialIndex >= 0)
        {
            material = ProcessMaterial(
                scene->mMaterials[aimesh->mMaterialIndex],
                directory
            );
        }

        // Mesh の生成
        auto mesh = std::make_unique<Mesh>();
        if (!mesh->Initialize(vertices, indices, material))
        {
            LOG_ERROR("ModelLoader: Meshの初期化に失敗");
            return nullptr;
        }

        return mesh;
    }

    Material ModelLoader::ProcessMaterial(
        const aiMaterial* aiMat,
        const std::string& directory)
    {
        Material material;

        // マテリアル名
        aiString name;
        aiMat->Get(AI_MATKEY_NAME, name);
        material.name = name.C_Str();

        aiString materialName;
        aiMat->Get(AI_MATKEY_NAME, materialName);

        static const aiTextureType kColorTextureTypes[] = {
            aiTextureType_DIFFUSE,
            aiTextureType_BASE_COLOR,
            aiTextureType_EMISSIVE,
        };

        aiTextureType colorTextureType = aiTextureType_NONE;
        for (aiTextureType type : kColorTextureTypes)
        {
            if (aiMat->GetTextureCount(type) > 0)
            {
                colorTextureType = type;
                break;
            }
        }

        aiColor4D matchedColor;
        bool gotMatchedColor = false;
        if (colorTextureType == aiTextureType_EMISSIVE)
        {
            gotMatchedColor = (aiMat->Get(AI_MATKEY_COLOR_EMISSIVE, matchedColor) == AI_SUCCESS);
        }
        else if (colorTextureType == aiTextureType_BASE_COLOR)
        {
            gotMatchedColor = (aiMat->Get(AI_MATKEY_BASE_COLOR, matchedColor) == AI_SUCCESS);
        }
        else if (colorTextureType == aiTextureType_DIFFUSE)
        {
            gotMatchedColor = (aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, matchedColor) == AI_SUCCESS);
        }
        if (gotMatchedColor &&
            (matchedColor.r > 0.0f || matchedColor.g > 0.0f || matchedColor.b > 0.0f))
        {
            material.color = { matchedColor.r, matchedColor.g, matchedColor.b, matchedColor.a };
        }
        if (colorTextureType != aiTextureType_NONE)
        {
            aiString texPath;
            aiMat->GetTexture(colorTextureType, 0, &texPath);

            // テクスチャ名でAssetManagerから取得
            std::string rawTexPath = texPath.C_Str();
            std::string texName = rawTexPath;
            auto slashPos = rawTexPath.find_last_of("/\\");
            if (slashPos != std::string::npos)
            {
                texName = rawTexPath.substr(slashPos + 1);
            }

            material.diffuseTexture = Engine::System::Assets::AssetManager::GetInstance().GetTextureID(
                texName,
                std::wstring(directory.begin(), directory.end()));
            LOG_DEBUG("Material: {} Texture: {} ID: {}", material.name, texName, material.diffuseTexture);
        }
        return material;
    }
} // Engine::Graphics