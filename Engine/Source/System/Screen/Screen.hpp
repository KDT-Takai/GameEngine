#pragma once

namespace Engine::System
{
    // 仮想ウィンドウサイズの管理
    class Screen
    {
    public:
        // 仮想解像度の設定
        static void SetVirtualSize(float width, float height)
        {
            virtualWidth = width;
            virtualHeight = height;
            aspectRatio = width / height;
        }

        // 仮想解像度の取得
        static float GetVirtualWidth() { return virtualWidth; }
        static float GetVirtualHeight() { return virtualHeight; }
        static float GetAspectRatio() { return aspectRatio; }

    private:
        static inline float virtualWidth = 1920.0f;
        static inline float virtualHeight = 1080.0f;
        static inline float aspectRatio = virtualWidth / virtualHeight;
    };
} // Engine::System