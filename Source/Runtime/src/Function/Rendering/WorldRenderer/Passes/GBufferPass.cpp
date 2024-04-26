#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GBufferPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Data/GBufferData.h"

namespace SnowLeopardEngine
{
    GBufferPass::GBufferPass(RenderContext& renderContext) : m_RenderContext(renderContext)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/GBuffer.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/GBuffer.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = renderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_Pipeline = GraphicsPipeline::Builder {}.SetShaderProgram(program).Build();
    }

    void GBufferPass::AddToGraph(FrameGraph& fg, FrameGraphBlackboard& blackboard)
    {
        auto viewPort = g_EngineContext->RenderSys->GetAPI()->GetViewport();
        uint32_t width = viewPort.Width;
        uint32_t height = viewPort.Height;

        blackboard.add<GBufferData>() = fg.addCallbackPass<GBufferData>(
            "GBuffer Pass",
            [&](FrameGraph::Builder& builder, GBufferData& data) {
                data.Position = builder.create<FrameGraphTexture2D>("Position", {width, height, PixelColorFormat::RGB32});
                data.Position = builder.write(data.Position);

                data.Normal = builder.create<FrameGraphTexture2D>("Normal", {width, height, PixelColorFormat::RGB32});
                data.Normal = builder.write(data.Normal);

                data.Albedo = builder.create<FrameGraphTexture2D>("Albedo", {width, height, PixelColorFormat::RGB32});
                data.Albedo = builder.write(data.Albedo);

                data.MetallicRoughnessAO = builder.create<FrameGraphTexture2D>("Metallic Roughness AO", {width, height, PixelColorFormat::RGB8_UNORM});
                data.MetallicRoughnessAO = builder.write(data.MetallicRoughnessAO);

                data.EntityID = builder.create<FrameGraphTexture2D>("EntityID", {width, height, PixelColorFormat::R32_SINT});
                data.EntityID = builder.write(data.EntityID);
            },
            [=](const GBufferData& data, FrameGraphPassResources& resources, void* ctx) {
                auto& rc = *static_cast<RenderContext*>(ctx);
            });
    }
} // namespace SnowLeopardEngine