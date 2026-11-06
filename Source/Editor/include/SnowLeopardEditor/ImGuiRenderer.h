#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderSystem.h"

#include <vri/ext/vri_ext_imgui.h>

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace SnowLeopardEngine::Editor
{
    // Draws ImGui through VRI's built-in ImGui renderer (vri_ext_imgui), as a render overlay of the swapchain pass.
    // Handles ImGui 1.92's dynamic textures (font atlas pages); ported from libvultra's EditorGui.
    class ImGuiRenderer final : public RenderOverlay
    {
    public:
        ImGuiRenderer(RHI::Device& device, VriFormat targetFormat);
        ~ImGuiRenderer() override;

        ImGuiRenderer(const ImGuiRenderer&)            = delete;
        ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;

        void Upload(RHI::Extent target) override;
        void Copy(VriCommandBuffer* cmd) override;
        void Draw(VriCommandBuffer* cmd) override;

    private:
        void UpdateTextures();
        void DestroyTextures();

    private:
        RHI::Device&      m_Device;
        VriImguiInterface m_Api {};
        VriImgui*         m_Renderer = nullptr;

        // Textures ImGui asked for (ImTextureData), keyed by the view handed back as ImTextureID.
        std::unordered_map<VriDescriptor*, Scope<RHI::Texture>> m_Textures;

        std::vector<VriImguiVertex>      m_Vertices;
        std::vector<uint16_t>            m_Indices16;
        std::vector<uint32_t>            m_Indices32;
        std::vector<VriImguiDrawCommand> m_Commands;
        VriImguiDrawData                 m_DrawData {};
    };
} // namespace SnowLeopardEngine::Editor
