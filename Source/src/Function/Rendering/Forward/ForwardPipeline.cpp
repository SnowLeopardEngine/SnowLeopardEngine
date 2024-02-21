#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardPipeline.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"

namespace SnowLeopardEngine
{
    bool ForwardPipeline::Init(const PipelineInitInfo& initInfo)
    {
        Pipeline::Init(initInfo);

        // Create and register pipeline states
        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Registering pipeline states...");

        // Phong
        auto phong                = CreateRef<PipelineState>();
        phong->Name               = "Phong";
        phong->VertexShaderName   = "Phong.vert";
        phong->FragmentShaderName = "Phong.frag";
        m_StateManager->RegisterState(phong);

        // Terrain
        auto terrain                = CreateRef<PipelineState>();
        terrain->Name               = "Terrain";
        terrain->VertexShaderName   = "Terrain.vert";
        terrain->FragmentShaderName = "Terrain.frag";
        m_StateManager->RegisterState(terrain);

        // Skybox
        auto skybox                = CreateRef<PipelineState>();
        skybox->Name               = "Skybox";
        skybox->VertexShaderName   = "Skybox.vert";
        skybox->FragmentShaderName = "Skybox.frag";
        // TODO: Ziyu Min, draw skybox the last, so the pipeline state need to set.
        // https://learnopengl.com/Advanced-OpenGL/Cubemaps
        m_StateManager->RegisterState(skybox);

        // Add forward single pass
        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Adding ForwardSinglePass...");
        auto forwardSinglePass = CreateRef<ForwardSinglePass>();
        forwardSinglePass->BindPipeline(this);
        m_DrawPasses.emplace_back(forwardSinglePass);

        SNOW_LEOPARD_CORE_INFO("[Rendering][ForwardPipeline] Initialized");

        return true;
    }
} // namespace SnowLeopardEngine