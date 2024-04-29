#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ToneMappingPass.h"
#include "FrameGraphResource.hpp"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"

namespace SnowLeopardEngine
{
    ToneMappingPass::ToneMappingPass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/ToneMappingPass.frag");
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

    ToneMappingPass::~ToneMappingPass() { m_RenderContext.Destroy(m_Pipeline); }

    FrameGraphResource ToneMappingPass::AddToGraph(FrameGraph& fg, FrameGraphResource input)
    {
        const auto extent = fg.getDescriptor<FrameGraphTexture>(input).Extent;

        struct Data
        {
            FrameGraphResource Output;
        };
        const auto& pass = fg.addCallbackPass<Data>(
            "Tone-Mapping Pass",
            [&](FrameGraph::Builder& builder, Data& data) {
                builder.read(input);

                data.Output = builder.create<FrameGraphTexture>("SceneColor",
                                                                {.Extent = extent, .Format = PixelFormat::RGB8_UNorm});
                data.Output = builder.write(data.Output);
            },
            [=, this](const Data& data, FrameGraphPassResources& resources, void* ctx) {
                const RenderingInfo renderingInfo {
                    .Area             = {.Extent = extent},
                    .ColorAttachments = {{
                        .Image = getTexture(resources, data.Output),
                    }},
                };

                auto&      rc          = *static_cast<RenderContext*>(ctx);
                const auto framebuffer = rc.BeginRendering(renderingInfo);
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindTexture(0, getTexture(resources, input))
                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });

        return pass.Output;
    }
} // namespace SnowLeopardEngine