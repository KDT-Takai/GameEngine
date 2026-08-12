#pragma once
#include <entt/entt.hpp>
#include <functional>
#include <string>
#include <vector>

namespace Engine::System::Debug
{
    class EntityInspector
    {
    public:
        // コンポーネント情報
        struct ComponentInfo
        {
            std::string name;
            std::function<bool(entt::registry&, entt::entity)> hasComponent;
            std::function<void(entt::registry&, entt::entity)> drawComponent;
        };

        // コンポーネントの登録
        template<typename T>
        void RegisterComponent(
            const std::string& name,
            std::function<void(entt::registry&, entt::entity)> drawFunc)
        {
            ComponentInfo info;
            info.name = name;
            info.hasComponent = [](entt::registry& reg, entt::entity entity)
                {
                    return reg.all_of<T>(entity);
                };
            info.drawComponent = drawFunc;
            components.push_back(info);
        }

        // ImGui描画
        void Draw(entt::registry& registry);

    private:
        void DrawEntityList(entt::registry& registry);
        void DrawComponentList(entt::registry& registry);

        std::vector<ComponentInfo> components;
        entt::entity selectedEntity = entt::null;
    };
} // Engine::System::Debug