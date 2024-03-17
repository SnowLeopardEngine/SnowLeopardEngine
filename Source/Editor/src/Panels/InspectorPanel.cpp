#include "SnowLeopardEditor/Panels/InspectorPanel.h"
#include "SnowLeopardEditor/Selector.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

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
                    if (ImGui::CollapsingHeader("MeshFilter", ImGuiTreeNodeFlags_DefaultOpen))
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

                    if (ImGui::CollapsingHeader("MeshRenderer", ImGuiTreeNodeFlags_DefaultOpen))
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

                // TODO: Draw more components
            }
        }
    }

    void InspectorPanel::Shutdown() {}
} // namespace SnowLeopardEngine::Editor