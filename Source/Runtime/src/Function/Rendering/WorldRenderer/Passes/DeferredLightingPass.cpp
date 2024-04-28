#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/DeferredLightingPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/DeferredLightingData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GBufferData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/ShadowData.h"

namespace SnowLeopardEngine
{
    DeferredLightingPass::DeferredLightingPass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/DeferredLighting.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_Pipeline = GraphicsPipeline::Builder {}
                         .SetShaderProgram(program)
                         .SetDepthStencil({
                             .DepthTest  = false,
                             .DepthWrite = false,
                         })
                         .SetRasterizerState({
                             .PolygonMode = PolygonMode::Fill,
                             .CullMode    = CullMode::Back,
                             .ScissorTest = false,
                         })
                         .Build();
    }
    DeferredLightingPass::~DeferredLightingPass() { m_RenderContext.Destroy(m_Pipeline); }

    void DeferredLightingPass::AddToGraph(FrameGraph& fg, FrameGraphBlackboard& blackboard)
    {
        const auto [frameUniform] = blackboard.get<FrameData>();

        const auto& shadow  = blackboard.get<ShadowData>();
        const auto& gBuffer = blackboard.get<GBufferData>();
        const auto  extent  = fg.getDescriptor<FrameGraphTexture>(gBuffer.Depth).Extent;

        blackboard.add<DeferredLightingData>() = fg.addCallbackPass<DeferredLightingData>(
            "Deferred Lighting Pass",
            [&](FrameGraph::Builder& builder, DeferredLightingData& data) {
                builder.read(frameUniform);

                builder.read(shadow.ShadowMap);

                builder.read(gBuffer.Position);
                builder.read(gBuffer.Normal);
                builder.read(gBuffer.Albedo);
                builder.read(gBuffer.MetallicRoughnessAO);

                data.SceneColor =
                    builder.create<FrameGraphTexture>("SceneColor", {.Extent = extent, .Format = PixelFormat::RGB16F});
                data.SceneColor = builder.write(data.SceneColor);
            },
            [=, this](const DeferredLightingData& data, FrameGraphPassResources& resources, void* ctx) {
                const RenderingInfo renderingInfo {
                    .Area             = {.Extent = extent},
                    .ColorAttachments = {{
                        .Image      = getTexture(resources, data.SceneColor),
                        .ClearValue = glm::vec4 {0.0f},
                    }},
                };

                auto& rc = *static_cast<RenderContext*>(ctx);

                const auto framebuffer = rc.BeginRendering(renderingInfo);

                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindUniformBuffer(0, getBuffer(resources, frameUniform))
                    .BindTexture(0, getTexture(resources, gBuffer.Position))
                    .BindTexture(1, getTexture(resources, gBuffer.Normal))
                    .BindTexture(2, getTexture(resources, gBuffer.Albedo))
                    .BindTexture(3, getTexture(resources, gBuffer.MetallicRoughnessAO))
                    .BindTexture(4, getTexture(resources, shadow.ShadowMap))
                    .DrawFullScreenTriangle();

                rc.EndRendering(framebuffer);
            });
    }
} // namespace SnowLeopardEngine