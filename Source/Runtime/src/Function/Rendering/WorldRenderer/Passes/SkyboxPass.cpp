#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/SkyboxPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GBufferData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    SkyboxPass::SkyboxPass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/SkyboxPass.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/SkyboxPass.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_Pipeline = GraphicsPipeline::Builder {}
                         .SetDepthStencil({
                             .DepthTest      = true,
                             .DepthWrite     = false,
                             .DepthCompareOp = CompareOp::Less,
                         })
                         .SetRasterizerState({
                             .PolygonMode = PolygonMode::Fill,
                             .CullMode    = CullMode::Back,
                             .ScissorTest = false,
                         })
                         .SetShaderProgram(program)
                         .Build();
    }

    SkyboxPass::~SkyboxPass() { m_RenderContext.Destroy(m_Pipeline); }

    FrameGraphResource SkyboxPass::AddToGraph(FrameGraph&           fg,
                                              FrameGraphBlackboard& blackboard,
                                              FrameGraphResource    target,
                                              Texture*              cubemap)
    {
        assert(cubemap && *cubemap);

        const auto [frameUniform] = blackboard.get<FrameData>();

        const auto& gBuffer = blackboard.get<GBufferData>();
        const auto  extent  = fg.getDescriptor<FrameGraphTexture>(target).Extent;

        const auto skybox = importTexture(fg, "Skybox", cubemap);

        fg.addCallbackPass(
            "Skybox Pass",
            [&](FrameGraph::Builder& builder, auto&) {
                builder.read(frameUniform);
                builder.read(gBuffer.Depth);
                builder.read(skybox);

                target = builder.write(target);
            },
            [=, this](const auto&, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("Skybox Pass");
                SNOW_LEOPARD_PROFILE_GL("Skybox Pass");

                const RenderingInfo renderingInfo {
                    .Area             = {.Extent = extent},
                    .ColorAttachments = {{
                        .Image = getTexture(resources, target),
                    }},
                    .DepthAttachment =
                        AttachmentInfo {
                            .Image = getTexture(resources, gBuffer.Depth),
                        },
                };
                auto&      rc          = *static_cast<RenderContext*>(ctx);
                const auto framebuffer = rc.BeginRendering(renderingInfo);
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindUniformBuffer(0, getBuffer(resources, frameUniform))
                    .BindTexture(0, getTexture(resources, skybox))
                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });

        return target;
    }
} // namespace SnowLeopardEngine