#include "pch/pch.h"
#include "AssetBrowser.hpp"
#include <ImGui/imgui.h>

namespace Engine::System
{
    void AssetBrowser::Draw()
    {
        ImGui::Begin("Asset Browser");

        DrawFilterBar();
        ImGui::Separator();
        DrawAssetList();

        ImGui::End();
    }

    void AssetBrowser::DrawFilterBar()
    {
        // 検索ボックス
        ImGui::InputText("Search", searchBuf, sizeof(searchBuf));

        ImGui::SameLine();

        // タイプフィルター
        if (ImGui::Button("All"))    filterType = "";
        ImGui::SameLine();
        if (ImGui::Button("Texture")) filterType = "Texture";
        ImGui::SameLine();
        if (ImGui::Button("Model"))   filterType = "Model";
        ImGui::SameLine();
        if (ImGui::Button("Sound"))   filterType = "Sound";

        // 現在のフィルター表示
        if (!filterType.empty())
        {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0, 1, 1, 1), "[%s]", filterType.c_str());
        }
    }

    void AssetBrowser::DrawAssetList()
    {
        auto& assets = Engine::System::Assets::AssetManager::GetInstance().GetAssets();

        // アセット数表示
        ImGui::Text("Assets (%zu)", assets.size());
        ImGui::Separator();

        // テーブル表示
        if (ImGui::BeginTable("AssetTable", 4,
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY,
            ImVec2(0, 0)))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 120.0f);
            ImGui::TableSetupColumn("Extension", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableHeadersRow();

            std::string search = searchBuf;

            for (const auto& [key, info] : assets)
            {
                // タイプフィルター
                if (!filterType.empty() && info.type != filterType) continue;

                // 検索フィルター
                if (!search.empty())
                {
                    std::string nameLower = info.fileName;
                    std::ranges::transform(nameLower, nameLower.begin(),
                        [](unsigned char c) { return std::tolower(c); });
                    std::string searchLower = search;
                    std::ranges::transform(searchLower, searchLower.begin(),
                        [](unsigned char c) { return std::tolower(c); });

                    if (nameLower.find(searchLower) == std::string::npos) continue;
                }

                ImGui::TableNextRow();

                // Name
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", info.fileName.c_str());

                // Type
                ImGui::TableSetColumnIndex(1);
                if (info.type == "Texture")
                    ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", info.type.c_str());
                else if (info.type == "Model")
                    ImGui::TextColored(ImVec4(0, 0.5f, 1, 1), "%s", info.type.c_str());
                else if (info.type == "Sound")
                    ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", info.type.c_str());
                else
                    ImGui::Text("%s", info.type.c_str());

                // ID
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%llu", info.id);

                // Extension
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%s", info.extension.c_str());
            }

            ImGui::EndTable();
        }
    }
} // Engine::System