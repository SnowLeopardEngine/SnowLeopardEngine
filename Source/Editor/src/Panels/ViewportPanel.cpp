#include "SnowLeopardEditor/Panels/ViewportPanel.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include <SnowLeopardEngine/Function/Scene/Entity.h>

#include <imgui.h>

namespace SnowLeopardEngine::Editor
{
    void ViewportPanel::Init()
    {
        // Create RT
        FrameBufferTextureDesc rtColorAttachmentDesc = {};
        rtColorAttachmentDesc.TextureFormat          = FrameBufferTextureFormat::RGBA8;

        FrameBufferAttachmentDesc rtAttachmentDesc = {};
        rtAttachmentDesc.Attachments.emplace_back(rtColorAttachmentDesc);

        FrameBufferDesc rtDesc = {};
        rtDesc.Width           = m_ViewportSize.x;
        rtDesc.Height          = m_ViewportSize.y;
        rtDesc.AttachmentDesc  = rtAttachmentDesc;

        m_RenderTarget = FrameBuffer::Create(rtDesc);

        // Set RT
        g_EngineContext->RenderSys->GetPipeline()->SetRenderTarget(m_RenderTarget);

        // TODO: remove , test only code
        // Create a scene and set active
        auto scene = g_EngineContext->SceneMngr->CreateScene("RenderingSystem", true);

        // Create a camera
        Entity camera                                      = scene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox
        // clang-format off
        cameraComponent.CubemapFilePaths = {
            "Assets/Textures/Skybox001/right.jpg",
            "Assets/Textures/Skybox001/left.jpg",
            "Assets/Textures/Skybox001/top.jpg",
            "Assets/Textures/Skybox001/bottom.jpg",
            "Assets/Textures/Skybox001/front.jpg",
            "Assets/Textures/Skybox001/back.jpg"
        };
        // clang-format on
    }

    void ViewportPanel::OnTick(float deltaTime)
    {
        // Tick rendering system
        g_EngineContext->RenderSys->OnTick(deltaTime);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImGui::PopStyleVar();

        // TODO: Double buffering
        uint32_t colorAttachment0 = m_RenderTarget->GetColorAttachmentID(0);
        if (colorAttachment0)
        {
            // update viewport size & frame buffer size
            auto      size         = ImGui::GetContentRegionAvail();
            glm::vec2 viewportSize = {size.x, size.y};

            if (m_ViewportSize != viewportSize)
            {
                m_RenderTarget->Resize(viewportSize.x, viewportSize.y);
                m_ViewportSize = viewportSize;
            }

            // Render Framebuffer to an image.
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(colorAttachment0)), size, {0, 1}, {1, 0});
        }

        ImGui::End();
    }
} // namespace SnowLeopardEngine::Editor