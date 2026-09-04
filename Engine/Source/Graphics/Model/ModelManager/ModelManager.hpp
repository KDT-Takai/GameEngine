#pragma once
#include "Utility/Singleton/Singleton.hpp"
#include "Graphics/Model/ModelID/ModelID.hpp"
#include "Graphics/Model/Model/Model.hpp"
#include "Graphics/Model/ModelLoader/ModelLoader.hpp"
#include <unordered_map>
#include <memory>
#include <string>

namespace Engine::Graphics
{
    class ModelManager : public Engine::Utility::Singleton<ModelManager>
    {
        DECLARE_SINGLETON(ModelManager)
    public:
        bool Initialize();
        void Finalize();

        // ロード・キャッシュ登録・IDを返す
        // 既にロード済みなら即返す
        ModelID Load(const std::wstring& path);

        // 無効IDまたは未登録ならnullptrを返す
        const Model* Get(ModelID id) const;

        void AddContext();

    private:
        std::unordered_map<ModelID, std::unique_ptr<Model>> models;
        std::unique_ptr<ModelLoader>                        modelLoader;
    };
} // Engine::Graphics