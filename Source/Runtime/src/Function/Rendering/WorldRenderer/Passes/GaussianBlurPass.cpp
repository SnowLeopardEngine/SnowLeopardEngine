#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GaussianBlurPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    GaussianBlurPass::GaussianBlurPass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/GaussianBlur.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_Pipeline = GraphicsPipeline::Builder {}
                         .SetShaderProgram(program)
                         .SetDepthStencil({
                             .DepthTest  = true,
                             .DepthWrite = true,
                         })
                         .SetRasterizerState({
                             .PolygonMode = PolygonMode::Fill,
                             .CullMode    = CullMode::Back,
                             .ScissorTest = false,
                         })
                         .Build();
    }

    GaussianBlurPass::~GaussianBlurPass() { m_RenderContext.Destroy(m_Pipeline); }

    FrameGraphResource GaussianBlurPass::AddToGraph(FrameGraph& fg, FrameGraphResource input, float scale)
    {
        input = AddToGraph(fg, input, scale, false);
        return AddToGraph(fg, input, scale, true);
    }

    FrameGraphResource
    GaussianBlurPass::AddToGraph(FrameGraph& fg, FrameGraphResource input, float scale, bool horizontal)
    {
        const auto  name = (horizontal ? "Horizontal" : "Vertical") + std::string {" Gaussian Blur Pass"};
        const auto& desc = fg.getDescriptor<FrameGraphTexture>(input);

        struct Data
        {
            FrameGraphResource Output;
        };
        const auto& pass = fg.addCallbackPass<Data>(
            name,
            [&](FrameGraph::Builder& builder, Data& data) {
                builder.read(input);

                data.Output = builder.create<FrameGraphTexture>("Blurred SceneColor (Gaussian)",
                                                                {.Extent = desc.Extent, .Format = desc.Format});
                data.Output = builder.write(data.Output);
            },
            [=, this](const Data& data, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER(name);
                SNOW_LEOPARD_PROFILE_GL("Gaussian Blur Pass");

                const RenderingInfo renderingInfo {
                    .Area             = {.Extent = desc.Extent},
                    .ColorAttachments = {{
                        .Image = getTexture(resources, data.Output),
                    }},
                };

                auto&      rc          = *static_cast<RenderContext*>(ctx);
                const auto framebuffer = rc.BeginRendering(renderingInfo);
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindTexture(0, getTexture(resources, input))
                    .SetUniform1f("scale", scale)
                    .SetUniform1i("horizontal", horizontal)
                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });

        return pass.Output;
    }
} // namespace SnowLeopardEngine