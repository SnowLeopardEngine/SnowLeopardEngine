#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/UpsamplePass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    UpsamplePass::UpsamplePass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/UpsamplePass.frag");
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

    UpsamplePass::~UpsamplePass() { m_RenderContext.Destroy(m_Pipeline); }

    FrameGraphResource UpsamplePass::AddToGraph(FrameGraph& fg, FrameGraphResource input, float radius)
    {
        const auto extent = fg.getDescriptor<FrameGraphTexture>(input).Extent;

        struct Data
        {
            FrameGraphResource Upsampled;
        };
        const auto& inputDesc = fg.getDescriptor<FrameGraphTexture>(input);
        const auto& pass      = fg.addCallbackPass<Data>(
            "Upsample Pass",
            [&](FrameGraph::Builder& builder, Data& data) {
                builder.read(input);

                data.Upsampled = builder.create<FrameGraphTexture>("Upsampled",
                                                                   {.Extent =
                                                                        {
                                                                                 .Width  = inputDesc.Extent.Width * 2u,
                                                                                 .Height = inputDesc.Extent.Height * 2u,
                                                                        },
                                                                         .Format = inputDesc.Format});
                data.Upsampled = builder.write(data.Upsampled);
            },
            [=, this](const Data& data, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("Upsample Pass");
                SNOW_LEOPARD_PROFILE_GL("Upsample Pass");

                auto& targetTexture = getTexture(resources, data.Upsampled);

                const RenderingInfo renderingInfo {
                         .Area             = {.Extent = targetTexture.GetExtent()},
                         .ColorAttachments = {{.Image = targetTexture, .ClearValue = glm::vec4 {0.0f}}},
                };

                auto&      rc          = *static_cast<RenderContext*>(ctx);
                const auto framebuffer = rc.BeginRendering(renderingInfo);
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindTexture(0, getTexture(resources, input))
                    .SetUniform1f("radius", radius)
                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });

        return pass.Upsampled;
    }
} // namespace SnowLeopardEngine