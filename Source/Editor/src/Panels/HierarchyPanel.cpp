#include "SnowLeopardEditor/Panels/HierarchyPanel.h"
#include "SnowLeopardEditor/EditorContext.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

#include "IconsMaterialDesignIcons.h"
#include <imgui.h>

#include <algorithm>
#include <cctype>

namespace SnowLeopardEngine::Editor
{
    namespace
    {
        constexpr const char* kPayload = "HIERARCHY_ENTITY"; // CoreUUID

        const char* IconOf(Entity entity)
        {
            if (entity.HasComponent<CameraComponent>())
                return ICON_MDI_CAMERA;
            if (entity.HasComponent<DirectionalLightComponent>() || entity.HasComponent<PointLightComponent>())
                return ICON_MDI_LIGHTBULB;
            if (entity.HasComponent<MeshFilterComponent>() || entity.HasComponent<TerrainComponent>())
                return ICON_MDI_CUBE_OUTLINE;
            return ICON_MDI_CIRCLE_OUTLINE;
        }

        std::string Lower(std::string text)
        {
            std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return char(std::tolower(c)); });
            return text;
        }
    } // namespace

    void HierarchyPanel::DrawCreateMenu(Entity parent)
    {
        if (ImGui::MenuItem(ICON_MDI_CIRCLE_OUTLINE " Empty"))
            EditorContext::CreateEntity("Entity", parent);
        if (ImGui::BeginMenu(ICON_MDI_CUBE_OUTLINE " 3D Object"))
        {
            if (ImGui::MenuItem("Cube"))
                EditorContext::CreatePrimitive(MeshPrimitiveType::Cube, "Cube", parent);
            if (ImGui::MenuItem("Sphere"))
                EditorContext::CreatePrimitive(MeshPrimitiveType::Sphere, "Sphere", parent);
            if (ImGui::MenuItem("Capsule"))
                EditorContext::CreatePrimitive(MeshPrimitiveType::Capsule, "Capsule", parent);
            if (ImGui::MenuItem("Quad"))
                EditorContext::CreatePrimitive(MeshPrimitiveType::Quad, "Quad", parent);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_MDI_LIGHTBULB " Light"))
        {
            if (ImGui::MenuItem("Directional Light"))
                EditorContext::CreateLight(true, parent);
            if (ImGui::MenuItem("Point Light"))
                EditorContext::CreateLight(false, parent);
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem(ICON_MDI_CAMERA " Camera"))
            EditorContext::CreateCamera(parent);
    }

    bool HierarchyPanel::MatchesFilter(Entity entity)
    {
        if (m_Filter[0] == '\0')
        {
            return true;
        }
        if (Lower(entity.GetName()).find(Lower(m_Filter)) != std::string::npos)
        {
            return true;
        }
        // Keep the path to a matching descendant.
        auto scene = EditorContext::GetActiveScene();
        for (const auto& childId : entity.GetChildrenUUIDs())
        {
            if (auto child = scene->GetEntityWithCoreUUID(childId); child && MatchesFilter(child))
            {
                return true;
            }
        }
        return false;
    }

    void HierarchyPanel::OnTick(float deltaTime)
    {
        auto& visible = EditorContext::GetPanels().Hierarchy;
        if (!visible)
        {
            return;
        }
        if (!ImGui::Begin(ICON_MDI_FILE_TREE " Hierarchy###Hierarchy", &visible))
        {
            ImGui::End();
            return;
        }

        auto scene = EditorContext::GetActiveScene();
        if (scene == nullptr)
        {
            ImGui::End();
            return;
        }

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##filter", ICON_MDI_MAGNIFY " Filter", m_Filter, sizeof(m_Filter));
        if (EditorContext::GetMode() != EditorMode::Edit)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.2f, 1.0f));
            ImGui::TextWrapped("Playing: changes are discarded on Stop");
            ImGui::PopStyleColor();
        }
        ImGui::Separator();

        ImGui::BeginChild("##entities");
        for (auto entity : scene->GetRootEntities())
        {
            DrawEntity(entity);
        }

        // The empty area below the tree: drop to unparent, click to deselect, right-click to create.
        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, std::max(ImGui::GetContentRegionAvail().y, 40.0f)));
        if (ImGui::BeginDragDropTarget())
        {
            if (const auto* payload = ImGui::AcceptDragDropPayload(kPayload))
            {
                const auto id = *static_cast<const CoreUUID*>(payload->Data);
                scene->SetParent(scene->GetEntityWithCoreUUID(id), {}, true);
            }
            ImGui::EndDragDropTarget();
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            EditorContext::ClearSelection();
        }
        if (ImGui::BeginPopupContextItem("##hierarchy-background"))
        {
            DrawCreateMenu({});
            ImGui::EndPopup();
        }

        // Keyboard: F2 rename, Delete, Ctrl+D (the menu bar has the same shortcuts globally).
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_F2, false))
            {
                if (auto primary = EditorContext::GetPrimarySelection())
                {
                    m_Renaming = primary.GetCoreUUID();
                    std::snprintf(m_RenameBuffer, sizeof(m_RenameBuffer), "%s", primary.GetName().c_str());
                    m_FocusRename = true;
                }
            }
        }
        ImGui::EndChild();
        ImGui::End();
    }

    void HierarchyPanel::DrawEntity(Entity entity)
    {
        if (!MatchesFilter(entity))
        {
            return;
        }
        auto       scene    = EditorContext::GetActiveScene();
        const auto id       = entity.GetCoreUUID();
        const bool selected = EditorContext::IsSelected(entity);
        const bool leaf     = !entity.HasChildren();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                   ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
        if (selected)
            flags |= ImGuiTreeNodeFlags_Selected;
        if (leaf)
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (m_Filter[0] != '\0')
            ImGui::SetNextItemOpen(true); // a filtered tree shows its matches

        const bool renaming = m_Renaming == id;
        const bool enabled  = entity.IsEnabled();
        if (!enabled)
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        const std::string label = renaming ? std::string(IconOf(entity)) : std::string(IconOf(entity)) + "  " + entity.GetName();
        const bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(uint64_t(entt::to_integral(entt::entity(entity))) + 1), flags, "%s", label.c_str());
        if (!enabled)
            ImGui::PopStyleColor();

        // Select on release, so a drag does not change the selection.
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !ImGui::IsItemToggledOpen() &&
            ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).x == 0 && ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y == 0)
        {
            const bool additive = ImGui::GetIO().KeyCtrl || ImGui::GetIO().KeyShift;
            if (additive && selected)
                EditorContext::Deselect(entity);
            else
                EditorContext::Select(entity, additive);
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && leaf)
        {
            m_Renaming = id;
            std::snprintf(m_RenameBuffer, sizeof(m_RenameBuffer), "%s", entity.GetName().c_str());
            m_FocusRename = true;
        }

        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload(kPayload, &id, sizeof(id));
            ImGui::Text("%s %s", IconOf(entity), entity.GetName().c_str());
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const auto* payload = ImGui::AcceptDragDropPayload(kPayload))
            {
                auto dragged = scene->GetEntityWithCoreUUID(*static_cast<const CoreUUID*>(payload->Data));
                if (!scene->SetParent(dragged, entity, true))
                {
                    SNOW_LEOPARD_WARN("[Editor][Hierarchy] {0} cannot be a child of its own descendant", dragged.GetName());
                }
            }
            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginPopupContextItem())
        {
            if (!selected)
                EditorContext::Select(entity);
            if (ImGui::BeginMenu(ICON_MDI_PLUS " Create Child"))
            {
                DrawCreateMenu(entity);
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Rename", "F2"))
            {
                m_Renaming = id;
                std::snprintf(m_RenameBuffer, sizeof(m_RenameBuffer), "%s", entity.GetName().c_str());
                m_FocusRename = true;
            }
            if (ImGui::MenuItem(ICON_MDI_CONTENT_COPY " Duplicate", "Ctrl+D"))
                EditorContext::DuplicateSelection();
            if (entity.HasParent() && ImGui::MenuItem("Unparent"))
                scene->SetParent(entity, {}, true);
            ImGui::Separator();
            if (ImGui::MenuItem(ICON_MDI_DELETE " Delete", "Del"))
            {
                EditorContext::DeleteSelection();
                ImGui::EndPopup();
                if (open && !leaf)
                    ImGui::TreePop();
                return; // the entity is gone
            }
            ImGui::EndPopup();
        }

        // Inline rename over the node's label.
        if (renaming)
        {
            ImGui::SameLine();
            if (m_FocusRename)
            {
                ImGui::SetKeyboardFocusHere();
                m_FocusRename = false;
            }
            ImGui::SetNextItemWidth(-FLT_MIN);
            const bool done = ImGui::InputText("##rename", m_RenameBuffer, sizeof(m_RenameBuffer),
                                               ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll);
            if (done && m_RenameBuffer[0] != '\0')
                entity.SetName(m_RenameBuffer);
            if (done || ImGui::IsItemDeactivated() || ImGui::IsKeyPressed(ImGuiKey_Escape))
                m_Renaming = {};
        }

        if (open && !leaf)
        {
            // A copy: children may be reparented while the tree is drawn.
            const auto children = entity.GetChildrenUUIDs();
            for (const auto& childId : children)
            {
                if (auto child = scene->GetEntityWithCoreUUID(childId))
                {
                    DrawEntity(child);
                }
            }
            ImGui::TreePop();
        }
    }
} // namespace SnowLeopardEngine::Editor
