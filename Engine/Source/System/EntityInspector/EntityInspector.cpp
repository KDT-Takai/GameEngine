#include "pch/pch.h"
#include "EntityInspector.hpp"

namespace Engine::System::Debug
{
    void EntityInspector::Draw(entt::registry& registry)
    {
        ImGui::Begin("Entity Inspector");

        ImGui::BeginChild("EntityList", ImVec2(200, 0), true);
        DrawEntityList(registry);
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("ComponentList", ImVec2(0, 0), true);
        DrawComponentList(registry);
        ImGui::EndChild();

        ImGui::End();
    }

    void EntityInspector::DrawEntityList(entt::registry& registry)
    {
        size_t count = 0;
        auto view = registry.view<entt::entity>();
        for (auto entity : view)
        {
            ++count;
        }
        ImGui::Text("Entities (%zu)", count);

        ImGui::Separator();

        for (auto entity : view)
        {
            std::string label = "Entity " + std::to_string(
                static_cast<uint32_t>(entity));

            bool isSelected = selectedEntity == entity;
            if (ImGui::Selectable(label.c_str(), isSelected))
            {
                selectedEntity = entity;
            }
        }
    }

    void EntityInspector::DrawComponentList(entt::registry& registry)
    {
        if (selectedEntity == entt::null ||
            !registry.valid(selectedEntity))
        {
            ImGui::TextDisabled("Entityを選択してください");
            return;
        }

        ImGui::Text("Entity %u", static_cast<uint32_t>(selectedEntity));
        ImGui::Separator();

        // 登録済みコンポーネントを順番に表示
        for (auto& info : components)
        {
            if (!info.hasComponent(registry, selectedEntity)) continue;

            if (ImGui::CollapsingHeader(
                info.name.c_str(),
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                info.drawComponent(registry, selectedEntity);
            }

            ImGui::Spacing();
        }
    }
} // Engine::System::Debug