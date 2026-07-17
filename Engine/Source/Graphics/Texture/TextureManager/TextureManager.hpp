#pragma once
#include "Graphics/Texture/TextureID/TextureID.hpp"
#include "Graphics/Texture/Texture/Texture.hpp"
#include "Graphics/DX12/Type.hpp"
#include <unordered_map>
#include <memory>
#include <string>

namespace Engine::Graphics
{
    class TextureManager
    {
    public:
        bool Initialize();
        void Finalize();

        // ロード・キャッシュ登録・IDを返す
        // 既にロード済みなら即返す
        TextureID Load(const std::wstring& path);

        // 無効IDまたは未登録ならfallbackを返す
        const Texture* Get(TextureID id) const;

    private:
        bool CreateCommandObjects();
        bool CreateFallback();
        void SubmitAndWait();

        // テクスチャキャッシュ
        std::unordered_map<TextureID, std::unique_ptr<Texture>> textures;

        // フォールバック用1x1ホワイトテクスチャ
        std::unique_ptr<Texture> fallback;

        // 転送用コマンド関連
        dx12::CmdAllocator      cmdAllocator;
        dx12::GraphicsCmdList   cmdList;
        dx12::Fence             fence;
        UINT64                  fenceValue = 0;
    };
} // Engine::Graphics