#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderManager.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/FrameBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"

namespace SnowLeopardEngine
{
    DzShaderCompiler                             DzShaderManager::s_Compiler;
    std::unordered_map<std::string, DzShader>    DzShaderManager::s_DzShaders;
    std::unordered_map<std::string, Ref<Shader>> DzShaderManager::s_CompiledShaders;
    std::unordered_map<std::string, DzResource>  DzShaderManager::s_RenderResources;

    void DzShaderManager::AddShader(const std::string& dzShaderName, const std::filesystem::path& dzShaderFilePath)
    {
        if (s_DzShaders.count(dzShaderName) > 0)
        {
            return;
        }

        std::ifstream shaderFile(dzShaderFilePath);
        if (!shaderFile.is_open())
        {
            SNOW_LEOPARD_CORE_ERROR("[DzShaderManager] Failed to open shader file: {0}",
                                    dzShaderFilePath.generic_string());
            return;
        }
        cereal::JSONInputArchive archive(shaderFile);

        DzShader shaderFromFile;
        archive(shaderFromFile);

        AddShader(shaderFromFile);
    }

    void DzShaderManager::AddShader(const DzShader& dzShader) { s_DzShaders[dzShader.Name] = dzShader; }

    DzShader& DzShaderManager::GetShader(const std::string& dzShaderName)
    {
        SNOW_LEOPARD_CORE_ASSERT(s_DzShaders.count(dzShaderName) > 0,
                                 "[DzShaderManager] Failed to get DzShader {0}. Not found!",
                                 dzShaderName);

        return s_DzShaders[dzShaderName];
    }

    bool DzShaderManager::Compile()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        bool hasError = false;

        for (auto& [dzShaderName, dzShader] : s_DzShaders)
        {
            if (dzShader.Compiled)
            {
                continue;
            }

            auto compileResult = s_Compiler.Compile(dzShader);
            if (!compileResult.Success)
            {
                SNOW_LEOPARD_CORE_ERROR("[DzShaderManager] Failed to compile DzShader {0}, Message: {1}",
                                        dzShader.Name,
                                        compileResult.Message);
                hasError = true;
                continue;
            }

            for (const auto& passResult : compileResult.PassResults)
            {
                s_CompiledShaders[passResult.PassName] = Shader::Create(passResult.ShaderStageSources);
            }

            for (auto& resource : dzShader.Resources)
            {
                if (resource.Type == "DepthBuffer")
                {
                    int size = std::stoi(resource.Data);

                    // Depth frame buffer
                    FrameBufferDesc frameBufferDesc = {};
                    frameBufferDesc.Width           = size;
                    frameBufferDesc.Height          = size;

                    FrameBufferTextureDesc depthTexture = {};
                    depthTexture.TextureFormat          = FrameBufferTextureFormat::DEPTH24;
                    frameBufferDesc.AttachmentDesc.Attachments.emplace_back(depthTexture);
                    resource.ResourceHandle = FrameBuffer::Create(frameBufferDesc);

                    s_RenderResources[resource.Name] = resource;
                }
                else if (resource.Type == "GBuffer_PBR")
                {
                    auto viewport = g_EngineContext->RenderSys->GetAPI()->GetViewport();

                    // GBuffer (MR PBR)
                    FrameBufferDesc frameBufferDesc = {};
                    frameBufferDesc.Width           = viewport.Width;
                    frameBufferDesc.Height          = viewport.Height;

                    // Color attachment 0 : Position (RGB)
                    FrameBufferTextureDesc c0 = {};
                    c0.TextureFormat          = FrameBufferTextureFormat::RGBA32F;
                    frameBufferDesc.AttachmentDesc.Attachments.emplace_back(c0);

                    // Color attachment 1 : Normal (RGB)
                    FrameBufferTextureDesc c1 = {};
                    c1.TextureFormat          = FrameBufferTextureFormat::RGBA32F;
                    frameBufferDesc.AttachmentDesc.Attachments.emplace_back(c1);

                    // Color attachment 2 : Albedo (RGB) + Metallic (A)
                    FrameBufferTextureDesc c2 = {};
                    c2.TextureFormat          = FrameBufferTextureFormat::RGBA32F;
                    frameBufferDesc.AttachmentDesc.Attachments.emplace_back(c2);

                    // Color attachment 3 : Roughness (R) + AO (G) + EntityID(B)
                    FrameBufferTextureDesc c3 = {};
                    c3.TextureFormat          = FrameBufferTextureFormat::RGBA32F;
                    frameBufferDesc.AttachmentDesc.Attachments.emplace_back(c3);

                    // Depth Attachment
                    FrameBufferTextureDesc depth;
                    depth.TextureFormat = FrameBufferTextureFormat::DEPTH24_STENCIL8;
                    frameBufferDesc.AttachmentDesc.Attachments.emplace_back(depth);

                    resource.ResourceHandle = FrameBuffer::Create(frameBufferDesc);

                    s_RenderResources[resource.Name] = resource;
                }

                // TODO: Handle more types of resources
            }

            dzShader.Compiled = true;
        }

        return !hasError;
    }

    void DzShaderManager::BindPassResources(const DzPass& dzPass)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        // Bind resources if there are some resources
        for (const auto& resourceName : dzPass.ResourcesToBind)
        {
            if (s_RenderResources.count(resourceName) == 0)
            {
                SNOW_LEOPARD_CORE_ERROR("[DzShaderManager] Resource {0} not found!", resourceName);
            }

            auto resource = s_RenderResources[resourceName];

            // Handle Depth Buffer
            if (resource.Type == "DepthBuffer")
            {
                auto depthBuffer = std::dynamic_pointer_cast<FrameBuffer>(resource.ResourceHandle);
                depthBuffer->Bind();

                g_EngineContext->RenderSys->GetAPI()->ClearColor({0, 0, 0, 0}, ClearBit::Depth);
            }
            // Handle GBuffer (MR PBR)
            else if (resource.Type == "GBuffer_PBR")
            {
                auto gbuffer = std::dynamic_pointer_cast<FrameBuffer>(resource.ResourceHandle);
                gbuffer->Bind();

                g_EngineContext->RenderSys->GetAPI()->ClearColor({0, 0, 0, 0}, ClearBit::Default);
            }
        }
    }

    void DzShaderManager::UnbindPassResources(const DzPass& dzPass)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        // Unbind resources if there are some resources
        for (const auto& resourceName : dzPass.ResourcesToBind)
        {
            if (s_RenderResources.count(resourceName) == 0)
            {
                SNOW_LEOPARD_CORE_ERROR("[DzShaderManager] Resource {0} not found!", resourceName);
            }

            auto resource = s_RenderResources[resourceName];

            // Handle Depth Buffer
            if (resource.Type == "DepthBuffer")
            {
                auto depthBuffer = std::dynamic_pointer_cast<FrameBuffer>(resource.ResourceHandle);
                depthBuffer->Unbind();
            }
            // Handle GBuffer (MR PBR)
            else if (resource.Type == "GBuffer_PBR")
            {
                auto gbuffer = std::dynamic_pointer_cast<FrameBuffer>(resource.ResourceHandle);
                gbuffer->Unbind();
            }
        }
    }

    void DzShaderManager::BlitResources()
    {
        for (const auto& [resourceName, resource] : s_RenderResources)
        {
            if (resource.Type == "GBuffer_PBR")
            {
                auto gbuffer = std::dynamic_pointer_cast<FrameBuffer>(resource.ResourceHandle);
                gbuffer->Blit();
            }
        }
    }

    void DzShaderManager::UsePassResources(const DzPass& dzPass, const Ref<Shader>& shader, int& resourceBinding)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        // Use resources if there are some resources
        for (const auto& resourceName : dzPass.ResourcesToUse)
        {
            if (s_RenderResources.count(resourceName) == 0)
            {
                SNOW_LEOPARD_CORE_ERROR("[DzShaderManager] Resource {0} not found!", resourceName);
            }

            auto resource = s_RenderResources[resourceName];

            // Handle Depth Buffer
            if (resource.Type == "DepthBuffer")
            {
                auto depthBuffer = std::dynamic_pointer_cast<FrameBuffer>(resource.ResourceHandle);
                shader->SetInt(resourceName, resourceBinding);
                depthBuffer->BindDepthAttachmentTexture(resourceBinding);
                resourceBinding++;
            }
            // Handle GBuffer (MR PBR)
            else if (resource.Type == "GBuffer_PBR")
            {
                auto gbuffer = std::dynamic_pointer_cast<FrameBuffer>(resource.ResourceHandle);

                auto resourceNames = split(resource.Data, ';');

                uint32_t index = 0;
                for (const auto& gBufferTextureName : resourceNames)
                {
                    shader->SetInt(gBufferTextureName, resourceBinding);
                    gbuffer->BindColorAttachmentTexture(index++, resourceBinding);
                    resourceBinding++;
                }
            }
        }
    }

    Ref<Shader> DzShaderManager::GetCompiledPassShader(const std::string& dzPassName)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        if (s_CompiledShaders.count(dzPassName) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[DzShaderManager] DzPass {0} not found or not compiled!", dzPassName);
            return nullptr;
        }

        return s_CompiledShaders[dzPassName];
    }
} // namespace SnowLeopardEngine