#include "SnowLeopardEditor/ImGuiRenderer.h"

#include <imgui.h>

#include <cstring>
#include <numeric>
#include <stdexcept>

namespace SnowLeopardEngine::Editor
{
    ImGuiRenderer::ImGuiRenderer(RHI::Device& device, VriFormat targetFormat) : m_Device(device)
    {
        RHI::Check(vriGetInterface(device.Handle, VRI_INTERFACE_IMGUI, sizeof(m_Api), &m_Api), "Get ImGui interface");

        // No built-in font atlas: ImGui 1.92 creates and updates its textures through ImTextureData (UpdateTextures).
        VriImguiDesc desc {};
        desc.uploadQueue = device.Queue;
        desc.colorFormat = targetFormat;
        RHI::Check(m_Api.CreateImgui(device.Handle, &desc, &m_Renderer), "Create ImGui renderer");

        auto& io = ImGui::GetIO();
        io.BackendRendererName = "SnowLeopardEngine VRI";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures;
    }

    ImGuiRenderer::~ImGuiRenderer()
    {
        m_Device.WaitIdle();
        DestroyTextures();
        m_Api.DestroyImgui(m_Renderer);
        auto& io = ImGui::GetIO();
        io.BackendRendererName = nullptr;
        io.BackendFlags &= ~(ImGuiBackendFlags_RendererHasVtxOffset | ImGuiBackendFlags_RendererHasTextures);
    }

    void ImGuiRenderer::UpdateTextures()
    {
        for (auto* data : ImGui::GetPlatformIO().Textures)
        {
            if (data->Status == ImTextureStatus_OK || data->Status == ImTextureStatus_Destroyed)
            {
                continue;
            }
            auto* view = reinterpret_cast<VriDescriptor*>(data->GetTexID());
            if (data->Status == ImTextureStatus_WantDestroy)
            {
                m_Device.WaitIdle();
                m_Api.FreeImguiTexture(m_Renderer, view);
                m_Textures.erase(view);
                data->SetTexID(ImTextureID_Invalid);
                data->SetStatus(ImTextureStatus_Destroyed);
                continue;
            }
            if (data->Format != ImTextureFormat_RGBA32)
            {
                throw std::runtime_error("ImGui textures must be RGBA32");
            }

            const bool create = data->Status == ImTextureStatus_WantCreate;
            if (create)
            {
                auto desc    = RHI::ColorTextureDesc({uint32_t(data->Width), uint32_t(data->Height)});
                desc.usage   = VriTextureUsage_ShaderResource | VriTextureUsage_TransferDst;
                auto texture = CreateScope<RHI::Texture>(m_Device, desc);
                view         = texture->View();
                m_Textures.emplace(view, std::move(texture));
                data->SetTexID(reinterpret_cast<ImTextureID>(view));
            }
            auto& texture = *m_Textures.at(view);

            // Whole texture on create, the dirty rectangle on update; staging rows follow the device alignment.
            const uint32_t x      = create ? 0 : data->UpdateRect.x;
            const uint32_t y      = create ? 0 : data->UpdateRect.y;
            const uint32_t width  = create ? data->Width : data->UpdateRect.w;
            const uint32_t height = create ? data->Height : data->UpdateRect.h;
            const uint32_t alignment =
                std::lcm(256u, std::max(1u, m_Device.Core.GetDeviceDesc(m_Device.Handle)->uploadBufferTextureRowAlignment));
            const uint32_t pitch = (width * 4 + alignment - 1) / alignment * alignment;
            const uint64_t bytes = uint64_t(pitch) * height;

            RHI::Buffer staging(m_Device, {bytes, 0, VriBufferUsage_TransferSrc, VriMemoryLocation_HostUpload});
            auto*       mapped = static_cast<std::byte*>(m_Device.Core.MapBuffer(staging.Handle, 0, bytes));
            if (mapped == nullptr)
            {
                throw std::runtime_error("Map ImGui texture staging buffer failed");
            }
            for (uint32_t row = 0; row < height; ++row)
            {
                std::memcpy(mapped + uint64_t(row) * pitch, data->GetPixelsAt(int(x), int(y + row)), width * 4);
            }
            m_Device.Core.UnmapBuffer(staging.Handle);

            RHI::Frame frame(m_Device);
            auto*      cmd = frame.Begin();
            texture.Transition(cmd,
                               {VriAccess_CopyDestinationWrite, VriLayout_CopyDestination, VriPipelineStage_Transfer});
            VriBufferTextureCopyDesc copy {};
            copy.bufferRowLength   = pitch / 4;
            copy.bufferImageHeight = height;
            copy.texture           = {0, 0, 1, VriImageAspect_Color, int32_t(x), int32_t(y), 0, width, height, 1};
            m_Device.Core.CmdUploadBufferToTexture(cmd, texture.Handle, staging.Handle, &copy);
            texture.Transition(cmd,
                               {VriAccess_ShaderResourceRead, VriLayout_ShaderResource, VriPipelineStage_AllCommands});
            // Atlas changes are rare: finish the upload before the staging buffer goes away.
            frame.SubmitAndWait();
            data->SetStatus(ImTextureStatus_OK);
        }
    }

    void ImGuiRenderer::DestroyTextures()
    {
        if (ImGui::GetCurrentContext() != nullptr)
        {
            for (auto* data : ImGui::GetPlatformIO().Textures)
            {
                auto* view = reinterpret_cast<VriDescriptor*>(data->GetTexID());
                if (m_Textures.contains(view))
                {
                    m_Api.FreeImguiTexture(m_Renderer, view);
                    data->SetTexID(ImTextureID_Invalid);
                    data->SetStatus(ImTextureStatus_Destroyed);
                }
            }
        }
        m_Textures.clear();
    }

    void ImGuiRenderer::Upload(RHI::Extent target)
    {
        m_DrawData = {};
        UpdateTextures();

        const ImDrawData* draw = ImGui::GetDrawData();
        m_Vertices.clear();
        m_Indices16.clear();
        m_Indices32.clear();
        m_Commands.clear();
        if (draw == nullptr || draw->TotalVtxCount == 0 || draw->DisplaySize.x <= 0 || draw->DisplaySize.y <= 0 ||
            target.Empty())
        {
            return;
        }

        // VRI scissors are in framebuffer pixels; ImGui works in display coordinates.
        const ImVec2 scale {float(target.Width) / draw->DisplaySize.x, float(target.Height) / draw->DisplaySize.y};
        const auto   position = [&](float px, float py) {
            return ImVec2((px - draw->DisplayPos.x) * scale.x, (py - draw->DisplayPos.y) * scale.y);
        };
        for (const auto* list : draw->CmdLists)
        {
            const auto vertexBase = uint32_t(m_Vertices.size());
            const auto indexBase  = uint32_t(sizeof(ImDrawIdx) == 2 ? m_Indices16.size() : m_Indices32.size());
            for (const auto& vertex : list->VtxBuffer)
            {
                const auto pixel = position(vertex.pos.x, vertex.pos.y);
                m_Vertices.push_back({{pixel.x, pixel.y}, {vertex.uv.x, vertex.uv.y}, vertex.col});
            }
            if constexpr (sizeof(ImDrawIdx) == 2)
            {
                m_Indices16.insert(m_Indices16.end(), list->IdxBuffer.begin(), list->IdxBuffer.end());
            }
            else
            {
                m_Indices32.insert(m_Indices32.end(), list->IdxBuffer.begin(), list->IdxBuffer.end());
            }
            for (const auto& command : list->CmdBuffer)
            {
                if (command.UserCallback != nullptr)
                {
                    // VRI binds its whole render state per draw; only the reset sentinel is accepted.
                    if (command.UserCallback != ImDrawCallback_ResetRenderState)
                    {
                        throw std::runtime_error("Custom ImDrawCallback is not supported by the VRI ImGui renderer");
                    }
                    continue;
                }
                const auto clipMin = position(command.ClipRect.x, command.ClipRect.y);
                const auto clipMax = position(command.ClipRect.z, command.ClipRect.w);
                m_Commands.push_back({{clipMin.x, clipMin.y, clipMax.x, clipMax.y},
                                      command.ElemCount,
                                      indexBase + command.IdxOffset,
                                      int32_t(vertexBase + command.VtxOffset),
                                      reinterpret_cast<VriDescriptor*>(command.GetTexID())});
            }
        }

        m_DrawData.vertices    = m_Vertices.data();
        m_DrawData.vertexCount = uint32_t(m_Vertices.size());
        if constexpr (sizeof(ImDrawIdx) == 2)
        {
            m_DrawData.indices    = m_Indices16.data();
            m_DrawData.indexCount = uint32_t(m_Indices16.size());
        }
        else
        {
            m_DrawData.indices    = m_Indices32.data();
            m_DrawData.indexCount = uint32_t(m_Indices32.size());
        }
        m_DrawData.indexSize         = sizeof(ImDrawIdx);
        m_DrawData.commands          = m_Commands.data();
        m_DrawData.commandCount      = uint32_t(m_Commands.size());
        m_DrawData.displaySize[0]    = float(target.Width);
        m_DrawData.displaySize[1]    = float(target.Height);
        m_DrawData.framebufferWidth  = target.Width;
        m_DrawData.framebufferHeight = target.Height;
        m_Api.UploadImguiData(m_Renderer, &m_DrawData);
    }

    void ImGuiRenderer::Copy(VriCommandBuffer* cmd)
    {
        if (m_DrawData.vertexCount != 0)
        {
            m_Api.CmdCopyImguiData(cmd, m_Renderer);
        }
    }

    void ImGuiRenderer::Draw(VriCommandBuffer* cmd)
    {
        if (m_DrawData.vertexCount != 0)
        {
            m_Api.CmdDrawImgui(cmd, m_Renderer, &m_DrawData);
        }
    }
} // namespace SnowLeopardEngine::Editor
