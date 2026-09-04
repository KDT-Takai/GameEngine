// Source/Graphics/Model/BinaryModelLoader/BinaryModelLoader.cpp
#include "pch/pch.h"
#include "BinaryModelLoader.hpp"
#include "System/Assets/AssetManager/AssetManager.hpp"
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <cstring>

namespace Engine::Graphics
{
    namespace
    {
        constexpr char kMagic[4] = { 'E', 'M', 'D', 'L' };
        constexpr uint32_t kSupportedVersion = 1;

        bool ReadRaw(std::ifstream& in, void* dst, size_t size)
        {
            in.read(reinterpret_cast<char*>(dst), static_cast<std::streamsize>(size));
            return static_cast<bool>(in);
        }

        bool ReadString(std::ifstream& in, std::string& out)
        {
            uint32_t len = 0;
            if (!ReadRaw(in, &len, sizeof(len))) return false;
            out.resize(len);
            if (len == 0) return true;
            return ReadRaw(in, out.data(), len);
        }
    }

    std::unique_ptr<Model> BinaryModelLoader::Load(const std::wstring& path)
    {
        std::string pathStr(path.begin(), path.end());

        std::ifstream in(path, std::ios::binary);
        if (!in)
        {
            LOG_ERROR("BinaryModelLoader: ファイルを開けません: {}", pathStr);
            return nullptr;
        }

        char magic[4] = {};
        if (!ReadRaw(in, magic, sizeof(magic)) || std::memcmp(magic, kMagic, sizeof(magic)) != 0)
        {
            LOG_ERROR("BinaryModelLoader: 不正なファイル形式: {}", pathStr);
            return nullptr;
        }

        uint32_t version = 0;
        if (!ReadRaw(in, &version, sizeof(version)) || version != kSupportedVersion)
        {
            LOG_ERROR("BinaryModelLoader: 未対応のバージョン({}): {}", version, pathStr);
            return nullptr;
        }

        uint32_t meshCount = 0;
        if (!ReadRaw(in, &meshCount, sizeof(meshCount)))
        {
            LOG_ERROR("BinaryModelLoader: メッシュ数の読み込みに失敗: {}", pathStr);
            return nullptr;
        }

        std::wstring directory = std::filesystem::path(path).parent_path().wstring();

        auto model = std::make_unique<Model>();
        model->SetName(std::filesystem::path(path).stem().string());

        for (uint32_t i = 0; i < meshCount; ++i)
        {
            uint32_t vertexCount = 0;
            uint32_t indexCount = 0;
            if (!ReadRaw(in, &vertexCount, sizeof(vertexCount)) ||
                !ReadRaw(in, &indexCount, sizeof(indexCount)))
            {
                LOG_ERROR("BinaryModelLoader: メッシュヘッダの読み込みに失敗: {}", pathStr);
                return nullptr;
            }

            std::vector<ModelVertex> vertices(vertexCount);
            if (vertexCount > 0 &&
                !ReadRaw(in, vertices.data(), vertexCount * sizeof(ModelVertex)))
            {
                LOG_ERROR("BinaryModelLoader: 頂点データの読み込みに失敗: {}", pathStr);
                return nullptr;
            }

            std::vector<uint32_t> indices(indexCount);
            if (indexCount > 0 &&
                !ReadRaw(in, indices.data(), indexCount * sizeof(uint32_t)))
            {
                LOG_ERROR("BinaryModelLoader: インデックスデータの読み込みに失敗: {}", pathStr);
                return nullptr;
            }

            Material material;
            std::string materialName;
            if (!ReadString(in, materialName))
            {
                LOG_ERROR("BinaryModelLoader: 素材名の読み込みに失敗: {}", pathStr);
                return nullptr;
            }
            material.name = materialName;

            float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            if (!ReadRaw(in, color, sizeof(color)))
            {
                LOG_ERROR("BinaryModelLoader: カラーの読み込みに失敗: {}", pathStr);
                return nullptr;
            }
            material.color = { color[0], color[1], color[2], color[3] };

            std::string textureName;
            if (!ReadString(in, textureName))
            {
                LOG_ERROR("BinaryModelLoader: テクスチャ名の読み込みに失敗: {}", pathStr);
                return nullptr;
            }
            if (!textureName.empty())
            {
                material.diffuseTexture = Engine::System::Assets::AssetManager::GetInstance().GetTextureID(
                    textureName, directory);
            }

            auto mesh = std::make_unique<Mesh>();
            if (!mesh->Initialize(vertices, indices, material))
            {
                LOG_ERROR("BinaryModelLoader: Meshの初期化に失敗: {}", pathStr);
                return nullptr;
            }
            model->AddMesh(std::move(mesh));
        }

        LOG_INFO("BinaryModelLoader: モデルの読み込みに成功: {} ({}メッシュ)", pathStr, meshCount);

        return model;
    }
} // Engine::Graphics
