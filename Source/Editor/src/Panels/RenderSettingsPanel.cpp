#include "SnowLeopardEditor/Panels/RenderSettingsPanel.h"
#include "SnowLeopardEditor/EditorContext.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include "IconsMaterialDesignIcons.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include <algorithm>

namespace SnowLeopardEngine::Editor
{
    void RenderSettingsPanel::OnTick(float deltaTime)
    {
        auto& visible = EditorContext::GetPanels().RenderSettings;
        if (!visible)
        {
            return;
        }
        if (!ImGui::Begin(ICON_MDI_TUNE " Render Settings###RenderSettings", &visible))
        {
            ImGui::End();
            return;
        }

        // Widgets leave room for their labels on the right.
        ImGui::PushItemWidth(-ImGui::GetFontSize() * 7.5f);

        auto& renderSystem = g_EngineContext->RenderSys;
        auto* renderer     = renderSystem->GetSceneRenderer();
        auto  settings     = renderer->GetSettings();
        bool  changed      = false;

        if (ImGui::CollapsingHeader("Image", ImGuiTreeNodeFlags_DefaultOpen))
        {
            changed |= ImGui::SliderFloat("Exposure", &settings.Exposure, 0.05f, 4.0f, "%.2f", ImGuiSliderFlags_Logarithmic);
            changed |= ImGui::Checkbox("FXAA", &settings.FXAA);
        }
        if (ImGui::CollapsingHeader("Shadows", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static const uint32_t kResolutions[] {512, 1024, 2048, 4096};
            int                   current = 0;
            for (int i = 0; i < 4; ++i)
                current = kResolutions[i] == settings.ShadowResolution ? i : current;
            const char* names[] {"512", "1024", "2048", "4096"};
            if (ImGui::Combo("Resolution", &current, names, 4))
            {
                settings.ShadowResolution = kResolutions[current];
                changed                   = true;
            }
            changed |= ImGui::DragFloat("Distance", &settings.ShadowDistance, 1.0f, 10.0f, 2000.0f, "%.0f");
            changed |= ImGui::SliderFloat("Cascade Split", &settings.CascadeLambda, 0.0f, 1.0f, "%.2f");
            ImGui::SetItemTooltip("0: uniform cascades, 1: logarithmic");
        }
        if (ImGui::CollapsingHeader("Ambient Occlusion (GTAO)", ImGuiTreeNodeFlags_DefaultOpen))
        {
            changed |= ImGui::Checkbox("Enabled##ao", &settings.AmbientOcclusion);
            ImGui::BeginDisabled(!settings.AmbientOcclusion);
            changed |= ImGui::SliderFloat("Radius", &settings.AmbientOcclusionRadius, 0.1f, 5.0f, "%.2f");
            changed |= ImGui::SliderFloat("Intensity", &settings.AmbientOcclusionIntensity, 0.1f, 4.0f, "%.2f");
            ImGui::EndDisabled();
        }
        if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_DefaultOpen))
        {
            changed |= ImGui::Checkbox("Enabled##bloom", &settings.Bloom);
            ImGui::BeginDisabled(!settings.Bloom);
            changed |= ImGui::SliderFloat("Strength", &settings.BloomStrength, 0.0f, 0.3f, "%.3f");
            int levels = int(settings.BloomLevels);
            if (ImGui::SliderInt("Levels", &levels, 1, 10))
            {
                settings.BloomLevels = uint32_t(levels);
                changed              = true;
            }
            ImGui::EndDisabled();
        }
        if (ImGui::CollapsingHeader("Editor"))
        {
            changed |= ImGui::ColorEdit3("Selection", glm::value_ptr(settings.SelectionColor));
            int width = int(settings.SelectionOutline);
            if (ImGui::SliderInt("Outline Width", &width, 1, 6))
            {
                settings.SelectionOutline = uint32_t(width);
                changed                   = true;
            }
        }
        // Switches that add or remove passes rebuild the frame graph; values apply from the next frame.
        if (changed)
        {
            renderer->SetSettings(settings);
        }

        if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto& stats = renderer->GetStatistics();
            ImGui::TextWrapped("Objects %u (alpha-test %u, transparent %u)", stats.Instances, stats.AlphaTest, stats.Transparent);
            ImGui::TextWrapped("Unique meshes %u, materials %u, textures %u", stats.UniqueMeshes, stats.Materials, stats.Textures);
            ImGui::TextWrapped("Mesh arena %llu vertices, %llu indices", (unsigned long long)stats.ArenaVertices, (unsigned long long)stats.ArenaIndices);
            ImGui::TextWrapped("UI quads %u", stats.UIQuads);
        }

        if (ImGui::CollapsingHeader("GPU Passes", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const float frameGpu = float(renderSystem->GetFrameGpuMs());
            m_SmoothedGpu        = m_SmoothedGpu == 0 ? frameGpu : m_SmoothedGpu * 0.9f + frameGpu * 0.1f;
            ImGui::Text("Frame GPU %.3f ms", m_SmoothedGpu);
            ImGui::SameLine();
            ImGui::Checkbox("Sort by time", &m_SortByTime);

            auto timings = renderSystem->GetPassTimings();
            if (m_SortByTime)
            {
                std::sort(timings.begin(), timings.end(), [](const auto& a, const auto& b) { return a.GpuMs > b.GpuMs; });
            }
            if (ImGui::BeginTable("##passes", 3, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("Pass", ImGuiTableColumnFlags_WidthStretch, 3.0f);
                ImGui::TableSetupColumn("GPU ms", ImGuiTableColumnFlags_WidthStretch, 1.0f);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch, 2.0f);
                ImGui::TableHeadersRow();
                for (const auto& timing : timings)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(timing.Name.c_str());
                    ImGui::TableNextColumn();
                    ImGui::Text("%.3f", timing.GpuMs);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("barriers %.3f ms, CPU recording %.3f ms", timing.GpuBarrierMs, timing.CpuMs);
                    ImGui::TableNextColumn();
                    const float share = frameGpu > 0 ? float(timing.GpuMs) / frameGpu : 0.0f;
                    ImGui::ProgressBar(share, ImVec2(-FLT_MIN, 0), "");
                }
                ImGui::EndTable();
            }
        }

        ImGui::PopItemWidth();
        ImGui::End();
    }
} // namespace SnowLeopardEngine::Editor
