#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/DownsamplePass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    DownsamplePass::DownsamplePass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/DownsamplePass.frag");
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

    DownsamplePass::~DownsamplePass() { m_RenderContext.Destroy(m_Pipeline); }

    FrameGraphResource DownsamplePass::AddToGraph(FrameGraph& fg, FrameGraphResource input, uint32_t mipLevel)
    {
        const auto extent = fg.getDescriptor<FrameGraphTexture>(input).Extent;

        struct Data
        {
            FrameGraphResource Downsampled;
        };
        const auto& inputDesc = fg.getDescriptor<FrameGraphTexture>(input);
        const auto& pass      = fg.addCallbackPass<Data>(
            "Downsample Pass",
            [&](FrameGraph::Builder& builder, Data& data) {
                builder.read(input);

                data.Downsampled = builder.create<FrameGraphTexture>(
                    "Downsampled",
                    {.Extent =
                         {
                                  .Width =
                                 glm::max(static_cast<uint32_t>(static_cast<float>(inputDesc.Extent.Width) * 0.5f), 1u),
                                  .Height = glm::max(
                                 static_cast<uint32_t>(static_cast<float>(inputDesc.Extent.Height) * 0.5f), 1u),
                         },
                          .Format = inputDesc.Format});
                data.Downsampled = builder.write(data.Downsampled);
            },
            [=, this](const Data& data, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("Downsample Pass");
                SNOW_LEOPARD_PROFILE_GL("Downsample Pass");

                auto& targetTexture = getTexture(resources, data.Downsampled);

                const RenderingInfo renderingInfo {
                         .Area             = {.Extent = targetTexture.GetExtent()},
                         .ColorAttachments = {{.Image = targetTexture, .ClearValue = glm::vec4 {0.0f}}},
                };

                auto&      rc          = *static_cast<RenderContext*>(ctx);
                const auto framebuffer = rc.BeginRendering(renderingInfo);
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindTexture(0, getTexture(resources, input))
                    .SetUniform1i("mipLevel", mipLevel)
                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });

        return pass.Downsampled;
    }
} // namespace SnowLeopardEngine