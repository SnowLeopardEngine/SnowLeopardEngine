#include "SnowLeopardEditor/Panels/InspectorPanel.h"
#include "SnowLeopardEditor/Selector.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"
#include "SnowLeopardEngine/Function/Scene/LayerManager.h"
#include "SnowLeopardEngine/Function/Scene/TagManager.h"

#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "magic_enum.hpp"

namespace SnowLeopardEngine::Editor
{
    const uint32_t LabelWidth = 140;

    void InspectorPanel::Init() {}

    void InspectorPanel::OnTick(float deltaTime)
    {
        ImGui::Begin("Inspector");

        // Draw entity inspector
        auto scene = g_EngineContext->SceneMngr->GetActiveScene();
        if (scene != nullptr)
        {
            auto uuid = Selector::GetLastSelection(SelectionCategory::Viewport);
            if (uuid.has_value())
            {
                // Manually draw components
                auto entity = scene->GetEntityWithCoreUUID(uuid.value());

                bool isEntityEnabled = entity.IsEnabled();
                ImGui::Checkbox("##entity-is-enabled", &isEntityEnabled);
                entity.SetIsEnabled(isEntityEnabled);
                ImGui::SameLine();

                ImGui::Text("%s", entity.GetName().c_str());
                ImGui::SameLine();

                bool isEntityStatic = entity.IsStatic();
                ImGui::Checkbox("Static", &isEntityStatic);
                entity.SetIsStatic(isEntityStatic);

                // Draw tag
                const auto& tag        = entity.GetTag();
                std::string seletedTag = tag;

                ImGui::Text("Tag");
                ImGui::SameLine();
                if (ImGui::BeginCombo("##Tag", seletedTag.c_str()))
                {
                    for (const auto& tagName : TagManager::s_Tags)
                    {
                        bool isSelected = seletedTag == tagName;
                        if (ImGui::Selectable(tagName.c_str(), isSelected))
                            seletedTag = tagName;
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                entity.SetTag(seletedTag);

                // Draw layer
                const auto& layer         = entity.GetLayer();
                Layer       selectedLayer = layer;
                std::string selectedLayerName;
                if (!LayerMaskManager::TryGetLayerNameByLayer(selectedLayer, selectedLayerName))
                {
                    // ERROR
                }

                ImGui::Text("Layer");
                ImGui::SameLine();
                if (ImGui::BeginCombo("##Layer", selectedLayerName.c_str()))
                {
                    for (const auto& layerName : LayerMaskManager::GetNamedLayerNames())
                    {
                        Layer namedLayer;
                        if (!LayerMaskManager::TryGetLayerByName(layerName, namedLayer))
                        {
                            // ERROR
                            continue;
                        }
                        bool isSelected = selectedLayer == namedLayer;
                        if (ImGui::Selectable(layerName.c_str(), isSelected))
                            selectedLayer = namedLayer;
                        if (isSelected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                entity.SetLayer(selectedLayer);

                // Draw transform
                auto& transformComponent = entity.GetComponent<TransformComponent>();
                if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, LabelWidth);

                    ImGui::Text("Position");
                    ImGui::NextColumn();
                    ImGui::DragFloat3(
                        "##Position", glm::value_ptr(transformComponent.Position), 1.0f, 0.0f, 0.0f, "%.6f");

                    auto rotationEuler = transformComponent.GetRotationEuler();
                    ImGui::NextColumn();
                    ImGui::Text("Rotation");
                    ImGui::NextColumn();
                    ImGui::DragFloat3("##Rotation", glm::value_ptr(rotationEuler), 1.0f, 0.0f, 0.0f, "%.6f");
                    transformComponent.SetRotationEuler(rotationEuler);

                    ImGui::NextColumn();
                    ImGui::Text("Scale");
                    ImGui::NextColumn();
                    ImGui::DragFloat3("##Scale", glm::value_ptr(transformComponent.Scale), 1.0f, 0.0f, 0.0f, "%.6f");
                    ImGui::Columns(1);
                }

                // Draw meshFilter
                if (entity.HasComponent<MeshFilterComponent>())
                {
                    auto& meshFilterComponent = entity.GetComponent<MeshFilterComponent>();
                    if (ImGui::CollapsingHeader("Mesh Filter", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Columns(2);
                        ImGui::SetColumnWidth(0, LabelWidth);

                        ImGui::Text("Use Primitive");
                        ImGui::NextColumn();
                        ImGui::Checkbox("##UsePrimitive", &meshFilterComponent.UsePrimitive);

                        if (meshFilterComponent.UsePrimitive)
                        {
                            ImGui::NextColumn();
                            ImGui::Text("Primitive Type");
                            ImGui::NextColumn();

                            // Get all values of the enum
                            auto values = magic_enum::enum_values<MeshPrimitiveType>();

                            // Create a combo box for enum selection
                            MeshPrimitiveType selectedEnumValue = meshFilterComponent.PrimitiveType;
                            if (ImGui::BeginCombo("##MeshPrimitiveType",
                                                  magic_enum::enum_name(selectedEnumValue).data()))
                            {
                                for (const auto& value : values)
                                {
                                    bool isSelected = selectedEnumValue == value;
                                    if (ImGui::Selectable(magic_enum::enum_name(value).data(), isSelected))
                                        selectedEnumValue = value;
                                    if (isSelected)
                                        ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                            if (meshFilterComponent.PrimitiveType != selectedEnumValue)
                            {
                                meshFilterComponent.Meshes.Items.clear();
                                // TODO: Load meshes
                                meshFilterComponent.PrimitiveType = selectedEnumValue;
                            }
                        }
                        else
                        {
                            // TODO: Switch to AssetManager: AssetHandle (UUID) with filter (*.obj, *.fbx, *.gltf, ...)
                            // Read only for now
                            ImGui::NextColumn();
                            ImGui::Text("Mesh File Path");
                            ImGui::NextColumn();
                            ImGui::Text("%s", meshFilterComponent.FilePath.generic_string().c_str());
                        }

                        ImGui::Columns(1);
                    }
                }

                // Draw meshRenderer
                if (entity.HasComponent<MeshRendererComponent>())
                {
                    auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

                    if (ImGui::CollapsingHeader("Mesh Renderer", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Columns(2);
                        ImGui::SetColumnWidth(0, LabelWidth);

                        // TODO: Switch to AssetManager: AssetHandle (UUID) with filter (*.dzmaterial)
                        ImGui::Text("Material File Path");
                        ImGui::NextColumn();
                        ImGui::Text("%s", meshRendererComponent.MaterialFilePath.generic_string().c_str());

                        ImGui::NextColumn();
                        ImGui::Text("Cast Shadow");
                        ImGui::NextColumn();
                        ImGui::Checkbox("##CastShadow", &meshRendererComponent.CastShadow);

                        ImGui::Columns(1);
                    }
                }

                // Draw rigidBody
                if (entity.HasComponent<RigidBodyComponent>())
                {
                    auto& rigidBodyComponent = entity.GetComponent<RigidBodyComponent>();
                    if (ImGui::CollapsingHeader("RigidBody", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        if (!isEntityStatic)
                        {
                            ImGui::Columns(2);
                            ImGui::SetColumnWidth(0, LabelWidth);

                            ImGui::Text("Mass");
                            ImGui::NextColumn();
                            ImGui::DragFloat("##Mass",
                                             &rigidBodyComponent.Mass,
                                             1.0f,
                                             0.0f,
                                             std::numeric_limits<float>::max(),
                                             "%.6f");

                            ImGui::NextColumn();
                            ImGui::Text("Enable CCD");
                            ImGui::NextColumn();
                            ImGui::Checkbox("##EnableCCD", &rigidBodyComponent.EnableCCD);

                            ImGui::NextColumn();
                            ImGui::Text("Linear Damping");
                            ImGui::NextColumn();
                            ImGui::DragFloat("##LinearDamping",
                                             &rigidBodyComponent.LinearDamping,
                                             1.0f,
                                             0.0f,
                                             std::numeric_limits<float>::max(),
                                             "%.6f");

                            ImGui::NextColumn();
                            ImGui::Text("Angular Damping");
                            ImGui::NextColumn();
                            ImGui::DragFloat("##AngularDamping",
                                             &rigidBodyComponent.AngularDamping,
                                             1.0f,
                                             0.0f,
                                             std::numeric_limits<float>::max(),
                                             "%.6f");

                            ImGui::Columns(1);
                        }
                        else
                        {
                            ImGui::Text("Static RigidBody: No properties");
                        }
                    }
                }

                // Draw SphereCollider
                if (entity.HasComponent<SphereColliderComponent>())
                {
                    auto& sphereColliderComponent = entity.GetComponent<SphereColliderComponent>();
                    if (ImGui::CollapsingHeader("Sphere Collider", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Columns(2);
                        ImGui::SetColumnWidth(0, LabelWidth);

                        ImGui::Text("Radius");
                        ImGui::NextColumn();
                        ImGui::DragFloat("##Radius", &sphereColliderComponent.Radius, 0.1f, 0.0f, 0.0f, "%.6f");

                        ImGui::NextColumn();
                        ImGui::Text("Is Trigger");
                        ImGui::NextColumn();
                        ImGui::Checkbox("##IsTrigger", &sphereColliderComponent.IsTrigger);

                        ImGui::Columns(1);
                    }
                }

                // Draw BoxCollider
                if (entity.HasComponent<BoxColliderComponent>())
                {
                    auto& boxColliderComponent = entity.GetComponent<BoxColliderComponent>();
                    if (ImGui::CollapsingHeader("Box Collider", ImGuiTreeNodeFlags_DefaultOpen))
                    {
                        ImGui::Columns(2);
                        ImGui::SetColumnWidth(0, LabelWidth);

                        ImGui::Text("Offset");
                        ImGui::NextColumn();
                        ImGui::DragFloat3(
                            "##Offset", glm::value_ptr(boxColliderComponent.Offset), 0.1f, 0.0f, 0.0f, "%.6f");

                        ImGui::NextColumn();
                        ImGui::Text("Size");
                        ImGui::NextColumn();
                        ImGui::DragFloat3("##Size",
                                          glm::value_ptr(boxColliderComponent.Size),
                                          0.1f,
                                          0.0f,
                                          std::numeric_limits<float>::max(),
                                          "%.6f");

                        ImGui::NextColumn();
                        ImGui::Text("Is Trigger");
                        ImGui::NextColumn();
                        ImGui::Checkbox("##IsTrigger", &boxColliderComponent.IsTrigger);

                        ImGui::Columns(1);
                    }
                }

                // TODO: Draw more components
            }
        }
    }

    void InspectorPanel::Shutdown() {}
} // namespace SnowLeopardEngine::Editor