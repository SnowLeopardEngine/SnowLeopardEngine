#include "SnowLeopardEditor/Panels/InspectorPanel.h"
#include "SnowLeopardEditor/EditorContext.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"
#include "SnowLeopardEngine/Function/Scene/LayerManager.h"
#include "SnowLeopardEngine/Function/Scene/TagManager.h"

#include "IconsMaterialDesignIcons.h"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "magic_enum.hpp"

#include <functional>

namespace SnowLeopardEngine::Editor
{
    namespace
    {
        // ---- property rows: label | widget, in a two-column table per component

        bool BeginProperties(const char* id)
        {
            if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PadOuterX))
                return false;
            ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 130.0f);
            ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
            return true;
        }

        void Label(const char* label, const char* tooltip = nullptr)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted(label);
            if (tooltip != nullptr && ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", tooltip);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(-FLT_MIN);
        }

        bool Float(const char* label, float& value, float speed = 0.1f, float min = 0, float max = 0, const char* format = "%.3f")
        {
            Label(label);
            return ImGui::DragFloat((std::string("##") + label).c_str(), &value, speed, min, max, format);
        }

        bool Vec3(const char* label, glm::vec3& value, float speed = 0.1f, float min = 0, float max = 0)
        {
            Label(label);
            return ImGui::DragFloat3((std::string("##") + label).c_str(), glm::value_ptr(value), speed, min, max, "%.3f");
        }

        bool Bool(const char* label, bool& value)
        {
            Label(label);
            return ImGui::Checkbox((std::string("##") + label).c_str(), &value);
        }

        bool Color3(const char* label, glm::vec3& value)
        {
            Label(label);
            return ImGui::ColorEdit3((std::string("##") + label).c_str(), glm::value_ptr(value), ImGuiColorEditFlags_Float);
        }

        template<typename E>
        bool Enum(const char* label, E& value)
        {
            Label(label);
            bool changed = false;
            if (ImGui::BeginCombo((std::string("##") + label).c_str(), magic_enum::enum_name(value).data()))
            {
                for (const auto option : magic_enum::enum_values<E>())
                {
                    if (ImGui::Selectable(magic_enum::enum_name(option).data(), option == value))
                    {
                        changed = option != value;
                        value   = option;
                    }
                }
                ImGui::EndCombo();
            }
            return changed;
        }

        // A file path: typed, or dropped from the Resources panel ("AssetFile" payload). Paths inside the working
        // directory are stored relative to it, like the engine's own asset paths.
        bool Path(const char* label, std::filesystem::path& value, const char* extension)
        {
            Label(label, extension);
            char buffer[512];
            std::snprintf(buffer, sizeof(buffer), "%s", value.generic_string().c_str());
            bool changed = ImGui::InputText((std::string("##") + label).c_str(), buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue);
            if (changed)
                value = buffer;
            if (ImGui::BeginDragDropTarget())
            {
                if (const auto* payload = ImGui::AcceptDragDropPayload("AssetFile"))
                {
                    std::filesystem::path dropped(std::string(static_cast<const char*>(payload->Data)));
                    if (extension == nullptr || dropped.extension() == extension)
                    {
                        std::error_code error;
                        auto            relative = std::filesystem::relative(dropped, std::filesystem::current_path(), error);
                        value                    = !error && !relative.empty() && *relative.begin() != ".." ? relative : dropped;
                        changed                  = true;
                    }
                }
                ImGui::EndDragDropTarget();
            }
            return changed;
        }

        // A component section with a context menu to remove it. Returns whether to draw its properties.
        template<typename T>
        bool Section(Entity entity, const char* title, bool removable, bool& removed)
        {
            ImGui::PushID(title);
            const bool open = ImGui::CollapsingHeader(title, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
            if (removable && ImGui::BeginPopupContextItem("##component"))
            {
                if (ImGui::MenuItem(ICON_MDI_DELETE " Remove Component"))
                    removed = true;
                ImGui::EndPopup();
            }
            ImGui::PopID();
            return open && !removed;
        }

        template<typename T, typename Draw>
        void Component(Entity entity, const char* title, Draw draw, bool removable = true)
        {
            if (!entity.HasComponent<T>())
                return;
            bool removed = false;
            if (Section<T>(entity, title, removable, removed))
            {
                if (BeginProperties(title))
                {
                    draw(entity.GetComponent<T>());
                    ImGui::EndTable();
                }
            }
            if (removed)
                entity.RemoveComponent<T>();
        }

        void DrawAddComponent(Entity entity)
        {
            const float width = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + width * 0.15f);
            if (ImGui::Button(ICON_MDI_PLUS " Add Component", ImVec2(width * 0.7f, 0)))
                ImGui::OpenPopup("##add-component");
            if (!ImGui::BeginPopup("##add-component"))
                return;

            auto item = [&](const char* name, auto add, bool present) {
                if (!present && ImGui::MenuItem(name))
                    add();
            };
            auto material = [] { return CreateRef<PhysicsMaterial>(0.4, 0.4, 0.4); };
            ImGui::TextDisabled("Rendering");
            item("Mesh Filter", [&] { entity.AddComponent<MeshFilterComponent>().PrimitiveType = MeshPrimitiveType::Cube; }, entity.HasComponent<MeshFilterComponent>());
            item("Mesh Renderer", [&] { entity.AddComponent<MeshRendererComponent>().MaterialFilePath = "Assets/Materials/Next/White.dzmaterial"; }, entity.HasComponent<MeshRendererComponent>());
            item("Camera", [&] { entity.AddComponent<CameraComponent>(); }, entity.HasComponent<CameraComponent>());
            item("Directional Light", [&] { entity.AddComponent<DirectionalLightComponent>(); }, entity.HasComponent<DirectionalLightComponent>());
            item("Point Light", [&] { entity.AddComponent<PointLightComponent>(); }, entity.HasComponent<PointLightComponent>());
            ImGui::TextDisabled("Physics");
            item("Rigid Body", [&] { entity.AddComponent<RigidBodyComponent>(); }, entity.HasComponent<RigidBodyComponent>());
            item("Sphere Collider", [&] { entity.AddComponent<SphereColliderComponent>(material()); }, entity.HasComponent<SphereColliderComponent>());
            item("Box Collider", [&] { entity.AddComponent<BoxColliderComponent>(material()); }, entity.HasComponent<BoxColliderComponent>());
            item("Capsule Collider", [&] { entity.AddComponent<CapsuleColliderComponent>(0.5f, 1.0f, glm::vec3(0), material()); }, entity.HasComponent<CapsuleColliderComponent>());
            ImGui::EndPopup();
        }
    } // namespace

    void InspectorPanel::Init(const PanelCommonInitInfo& initInfo) {}

    void InspectorPanel::OnTick(float deltaTime)
    {
        auto& visible = EditorContext::GetPanels().Inspector;
        if (!visible)
        {
            return;
        }
        if (!ImGui::Begin("Inspector", &visible))
        {
            ImGui::End();
            return;
        }

        auto   scene  = EditorContext::GetActiveScene();
        Entity entity = EditorContext::GetPrimarySelection();
        if (scene == nullptr || !entity)
        {
            ImGui::TextDisabled("Select an entity in the Viewport or the Hierarchy.");
            ImGui::End();
            return;
        }
        if (const auto count = EditorContext::GetSelection().size(); count > 1)
        {
            ImGui::TextDisabled("%zu selected; showing the last one", count);
        }

        // ---- header: enabled, name, static, tag, layer
        bool enabled = entity.IsEnabled();
        if (ImGui::Checkbox("##enabled", &enabled))
            entity.SetIsEnabled(enabled);
        ImGui::SameLine();
        char name[128];
        std::snprintf(name, sizeof(name), "%s", entity.GetName().c_str());
        const auto& style = ImGui::GetStyle();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - (ImGui::GetFrameHeight() + style.ItemInnerSpacing.x +
                                                                    ImGui::CalcTextSize("Static").x + style.ItemSpacing.x));
        if (ImGui::InputText("##name", name, sizeof(name), ImGuiInputTextFlags_EnterReturnsTrue) && name[0] != '\0')
            entity.SetName(name);
        ImGui::SameLine();
        bool isStatic = entity.IsStatic();
        if (ImGui::Checkbox("Static", &isStatic))
            entity.SetIsStatic(isStatic);

        if (BeginProperties("##header"))
        {
            Label("Tag");
            if (ImGui::BeginCombo("##Tag", entity.GetTag().c_str()))
            {
                for (const auto& tag : TagManager::s_Tags)
                {
                    if (ImGui::Selectable(tag.c_str(), tag == entity.GetTag()))
                        entity.SetTag(tag);
                }
                ImGui::EndCombo();
            }
            std::string layerName;
            LayerMaskManager::TryGetLayerNameByLayer(entity.GetLayer(), layerName);
            Label("Layer");
            if (ImGui::BeginCombo("##Layer", layerName.c_str()))
            {
                for (const auto& option : LayerMaskManager::GetNamedLayerNames())
                {
                    Layer layer;
                    if (LayerMaskManager::TryGetLayerByName(option, layer) && ImGui::Selectable(option.c_str(), layer == entity.GetLayer()))
                        entity.SetLayer(layer);
                }
                ImGui::EndCombo();
            }
            ImGui::EndTable();
        }
        ImGui::Spacing();

        // ---- components
        Component<TransformComponent>(
            entity,
            ICON_MDI_AXIS_ARROW " Transform",
            [&](TransformComponent& transform) {
                Vec3("Position", transform.Position);
                auto rotation = transform.GetRotationEuler();
                if (Vec3("Rotation", rotation, 0.5f))
                    transform.SetRotationEuler(rotation);
                Vec3("Scale", transform.Scale, 0.05f);
                if (entity.HasParent())
                {
                    Label("Space");
                    ImGui::TextDisabled("local to %s", scene->GetEntityWithCoreUUID(entity.GetParentUUID()).GetName().c_str());
                }
            },
            false);

        Component<MeshFilterComponent>(entity, ICON_MDI_CUBE_OUTLINE " Mesh Filter", [&](MeshFilterComponent& filter) {
            // Primitive or file; either change drops the meshes, which the scene reloads next tick.
            auto type = filter.PrimitiveType;
            if (Enum("Primitive", type))
            {
                filter.PrimitiveType = type;
                filter.UsePrimitive  = type != MeshPrimitiveType::Invalid;
                filter.Meshes        = nullptr;
            }
            if (filter.PrimitiveType == MeshPrimitiveType::Invalid)
            {
                if (Path("Model File", filter.FilePath, nullptr))
                    filter.Meshes = nullptr;
                if (filter.Meshes != nullptr && filter.FilePath.empty())
                {
                    Label("Source");
                    ImGui::TextDisabled("runtime model (%zu meshes, not saved)", filter.Meshes->Items.size());
                }
            }
        });

        Component<MeshRendererComponent>(entity, ICON_MDI_PALETTE " Mesh Renderer", [&](MeshRendererComponent& renderer) {
            if (Path("Material", renderer.MaterialFilePath, ".dzmaterial"))
                renderer.Mat = nullptr; // reloaded next tick
            Bool("Cast Shadow", renderer.CastShadow);
        });

        Component<CameraComponent>(entity, ICON_MDI_CAMERA " Camera", [&](CameraComponent& camera) {
            Enum("Clear Flags", camera.ClearFlags);
            if (camera.ClearFlags == CameraClearFlags::Color)
            {
                Label("Clear Color");
                ImGui::ColorEdit4("##ClearColor", glm::value_ptr(camera.ClearColor));
            }
            Enum("Projection", camera.Projection);
            Float("Field of View", camera.FOV, 0.2f, 1.0f, 179.0f, "%.1f");
            Float("Near", camera.Near, 0.01f, 0.001f, camera.Far, "%.3f");
            Float("Far", camera.Far, 1.0f, camera.Near, 100000.0f, "%.1f");
            Path("Environment", camera.EnvironmentMapFilePath, ".hdr");
        });

        Component<DirectionalLightComponent>(entity, ICON_MDI_WEATHER_SUNNY " Directional Light", [&](DirectionalLightComponent& light) {
            if (Vec3("Direction", light.Direction, 0.01f) && glm::length(light.Direction) > 1e-4f)
                light.Direction = glm::normalize(light.Direction);
            Color3("Color", light.Color);
            Float("Intensity", light.Intensity, 0.05f, 0.0f, 100.0f);
        });

        Component<PointLightComponent>(entity, ICON_MDI_LIGHTBULB " Point Light", [&](PointLightComponent& light) {
            Color3("Color", light.Color);
            Float("Intensity", light.Intensity, 0.1f, 0.0f, 1000.0f);
            Float("Constant", light.Constant, 0.01f, 0.0f, 10.0f);
            Float("Linear", light.Linear, 0.001f, 0.0f, 10.0f);
            Float("Quadratic", light.Quadratic, 0.001f, 0.0f, 10.0f);
        });

        Component<RigidBodyComponent>(entity, ICON_MDI_WEIGHT " Rigid Body", [&](RigidBodyComponent& body) {
            if (entity.IsStatic())
            {
                Label("Static");
                ImGui::TextDisabled("static body: no properties");
                return;
            }
            Float("Mass", body.Mass, 0.1f, 0.0f, 1e6f);
            Bool("Continuous (CCD)", body.EnableCCD);
            Float("Linear Damping", body.LinearDamping, 0.01f, 0.0f, 100.0f);
            Float("Angular Damping", body.AngularDamping, 0.01f, 0.0f, 100.0f);
        });

        Component<SphereColliderComponent>(entity, ICON_MDI_CIRCLE_OUTLINE " Sphere Collider", [&](SphereColliderComponent& collider) {
            Float("Radius", collider.Radius, 0.05f, 0.0f, 1e4f);
            Bool("Is Trigger", collider.IsTrigger);
        });

        Component<BoxColliderComponent>(entity, ICON_MDI_CUBE_OUTLINE " Box Collider", [&](BoxColliderComponent& collider) {
            Vec3("Offset", collider.Offset);
            Vec3("Size", collider.Size, 0.05f, 0.0f, 1e4f);
            Bool("Is Trigger", collider.IsTrigger);
        });

        Component<CapsuleColliderComponent>(entity, ICON_MDI_PILL " Capsule Collider", [&](CapsuleColliderComponent& collider) {
            Float("Radius", collider.Radius, 0.05f, 0.0f, 1e4f);
            Float("Height", collider.Height, 0.05f, 0.0f, 1e4f);
            Vec3("Offset", collider.Offset);
            Bool("Is Trigger", collider.IsTrigger);
        });

        Component<AnimatorComponent>(entity, ICON_MDI_RUN " Animator", [&](AnimatorComponent&) {
            Label("Controller");
            ImGui::TextDisabled("set up in code (state-machine editor: Phase 5)");
        });

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        DrawAddComponent(entity);

        ImGui::End();
    }

    void InspectorPanel::Shutdown() {}
} // namespace SnowLeopardEngine::Editor
