#include "pch/pch.h"
#include "AssetManager.hpp"
#include <filesystem>

namespace Engine::System::Assets
{
    void AssetManager::RegisterLoader(
        const std::string& extension,
        std::function<uint64_t(const std::wstring&)> loader)
    {
        loaders[ToLower(extension)] = loader;
    }

    void AssetManager::LoadDirectory(const std::wstring& directory)
    {
        if (!std::filesystem::exists(directory))
        {
            LOG_WARN("ディレクトリが見つかりません: {}",
                std::string(directory.begin(), directory.end()));
            return;
        }

        for (const auto& entry :
            std::filesystem::recursive_directory_iterator(directory))
        {
            if (!entry.is_regular_file()) continue;
            LoadFile(entry.path().wstring());
        }
    }

    void AssetManager::LoadFile(const std::wstring& path)
    {
        std::string ext = ToLower(GetExtension(path));

        auto it = loaders.find(ext);
        if (it == loaders.end()) return;

        // ロード実行してIDを取得
        uint64_t id = it->second(path);

        // assets に登録
        AssetInfo info;
        info.fullPath = path;
        info.fileName = GetFileName(path);
        info.extension = ext;
        info.id = id;

        // 拡張子でタイプを判定
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
        {
            info.type = "Texture";
        }
        else if (ext == ".fbx" || ext == ".obj" || ext == ".gltf")
        {
            info.type = "Model";
        }
        else if (ext == ".wav" || ext == ".mp3" || ext == ".ogg")
        {
            info.type = "Sound";
        }
        else
        {
            info.type = "Unknown";
        }

        assets[info.fileName] = info;

        LOG_INFO("アセットをロードしました: [{}] {}",
            info.type,
            std::string(path.begin(), path.end()));
    }

    Engine::Graphics::TextureID AssetManager::GetTextureID(
        const std::string& name) const
    {
        std::string lower = ToLower(name);

        // 拡張子なしで検索
        std::string nameWithoutExt = lower;
        auto dotPos = lower.find_last_of('.');
        if (dotPos != std::string::npos)
        {
            nameWithoutExt = lower.substr(0, dotPos);
        }

        // フルパス・ファイル名・拡張子なしで検索
        for (const auto& [key, info] : assets)
        {
            if (info.type != "Texture") continue;

            std::string keyLower = ToLower(key);
            std::string fileLower = ToLower(info.fileName);

            if (keyLower == lower ||
                fileLower == lower ||
                fileLower == nameWithoutExt)
            {
                return static_cast<Engine::Graphics::TextureID>(info.id);
            }
        }

        LOG_WARN("テクスチャが見つかりません: {}", name);
        return Engine::Graphics::InvalidTextureID;
    }

    std::wstring AssetManager::ToWString(const std::string& str)
    {
        return std::wstring(str.begin(), str.end());
    }

    std::string AssetManager::ToLower(const std::string& str)
    {
        std::string result = str;
        std::ranges::transform(result, result.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    std::string AssetManager::GetExtension(const std::wstring& path)
    {
        std::filesystem::path p(path);
        return p.extension().string();
    }

    std::string AssetManager::GetFileName(const std::wstring& path)
    {
        std::filesystem::path p(path);
        return p.stem().string();  // 拡張子なしのファイル名
    }
} // Engine::System::Assets