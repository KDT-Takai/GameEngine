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

        TextureID Load(const std::wstring& path);
        const Texture* Get(TextureID id) const;

        void AddContext();

    private:
        bool CreateFallback();
        void SubmitAndWait();

        std::unordered_map<TextureID, std::unique_ptr<Texture>> textures;
        std::unique_ptr<Texture> fallback;

        dx12::CmdAllocator    cmdAllocator;
        dx12::GraphicsCmdList cmdList;
        dx12::Fence           fence;
        HANDLE                fenceEvent = nullptr;
        UINT64                fenceValue = 0;
    };
} // Engine::Graphics