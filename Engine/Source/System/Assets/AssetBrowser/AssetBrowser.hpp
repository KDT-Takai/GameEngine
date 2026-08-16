#pragma once
#include "System/Assets/AssetManager/AssetManager.hpp"
#include <string>

namespace Engine::System
{
    class AssetBrowser
    {
    public:
        void Draw();

    private:
        void DrawFilterBar();
        void DrawAssetList();

        // フィルター
        std::string filterType;  // "" = 全表示 / "Texture" / "Model" / "Sound"
        char        searchBuf[256] = {};  // 検索ボックス
    };
} // Engine::System