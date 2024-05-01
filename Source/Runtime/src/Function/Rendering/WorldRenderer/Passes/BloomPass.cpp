#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BloomPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    BloomPass::BloomPass(RenderContext& rc) : m_RenderContext(rc), m_Upsampler(rc), m_Downsampler(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/BloomPass.frag");
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

    BloomPass::~BloomPass() { m_RenderContext.Destroy(m_Pipeline); }

    FrameGraphResource BloomPass::Resample(FrameGraph& fg, FrameGraphResource input, float radius)
    {
        constexpr auto kNumSteps = 6;

        for (auto i = 0; i < kNumSteps; ++i)
            input = m_Downsampler.AddToGraph(fg, input, i);
        for (auto i = 0; i < kNumSteps - 1; ++i)
            input = m_Upsampler.AddToGraph(fg, input, radius);

        return input;
    }

    FrameGraphResource
    BloomPass::Compose(FrameGraph& fg, FrameGraphResource scene, FrameGraphResource bloom, float strength)
    {
        struct Data
        {
            FrameGraphResource Output;
        };
        const auto& data = fg.addCallbackPass<Data>(
            "Bloom Pass",
            [&](FrameGraph::Builder& builder, Data& data) {
                builder.read(scene);
                builder.read(bloom);

                const auto& desc = fg.getDescriptor<FrameGraphTexture>(scene);
                data.Output      = builder.create<FrameGraphTexture>("Bloomed",
                                                                {
                                                                         .Extent = desc.Extent,
                                                                         .Format = desc.Format,
                                                                });
                data.Output      = builder.write(data.Output);
            },
            [=, this](const Data& data, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("Bloom Pass");
                SNOW_LEOPARD_PROFILE_GL("Bloom Pass");

                auto& rc = *static_cast<RenderContext*>(ctx);

                auto& outputTexture = getTexture(resources, data.Output);

                const RenderingInfo renderingInfo {
                    .Area             = {.Extent = {outputTexture.GetExtent()}},
                    .ColorAttachments = {{
                        .Image      = outputTexture,
                        .ClearValue = glm::vec4 {0.0f},
                    }},
                };

                const auto framebuffer = rc.BeginRendering(renderingInfo);
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindTexture(0, getTexture(resources, scene))
                    .BindTexture(1, getTexture(resources, bloom))
                    .SetUniform1f("strength", strength)
                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });

        return data.Output;
    }
} // namespace SnowLeopardEngine