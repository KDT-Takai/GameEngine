#pragma once
#include "Utility/Singleton/Singleton.hpp"
#include "Graphics/Texture/TextureID/TextureID.hpp"
#include "Graphics/Model/ModelID/ModelID.hpp"
#include <functional>
#include <string>
#include <unordered_map>

namespace Engine::System::Assets
{
    class AssetManager : public Engine::Utility::Singleton<AssetManager>
    {
        DECLARE_SINGLETON(AssetManager)
    public:
        // ローダー登録
        void RegisterLoader(const std::string& extension, std::function<uint64_t(const std::wstring&)> loader);

        // フォルダを再帰的に走査してロード
        void LoadDirectory(const std::wstring& directory);
        void LoadDirectory(const std::wstring& directory, const std::string& filterExt = "");
        void LoadDirectoryOrdered(const std::wstring& directory);

        // 単一ファイルをロード
        void LoadFile(const std::wstring& path);

        // 名前からID取得（拡張子あり・なし・大文字小文字無視）
        Engine::Graphics::TextureID GetTextureID(const std::string& name, const std::wstring& directoryHint = L"") const;

        Engine::Graphics::ModelID GetModelID(const std::string& name) const;
        // SoundID GetSoundID(const std::string& name) const;


        // AssetBrowser用
        struct AssetInfo
        {
            std::wstring fullPath;
            std::string  fileName;   // 拡張子なし
            std::string  extension;
            std::string  type;       // "Texture" / "Model" / "Sound"
            uint64_t     id;
        };
        const std::unordered_map<std::string, AssetInfo>& GetAssets() const
        {
            return assets;
        }

    private:
        // 内部変換
        static std::wstring ToWString(const std::string& str);
        static std::string  ToLower(const std::string& str);
        static std::string  GetExtension(const std::wstring& path);
        static std::string  GetFileName(const std::wstring& path);

        // ファイル名
        std::unordered_map<std::string, AssetInfo> assets;

        // 拡張子
        std::unordered_map<std::string, std::function<uint64_t(const std::wstring&)>>loaders;
    };
} // Engine::System::Assets