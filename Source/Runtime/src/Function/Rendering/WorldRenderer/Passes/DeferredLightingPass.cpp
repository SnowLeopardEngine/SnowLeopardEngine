#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/DeferredLightingPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/BRDFData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GBufferData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GlobalLightProbeData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GrassData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/LightData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/SSAOData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/ShadowData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    DeferredLightingPass::DeferredLightingPass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/DeferredLightingPass.frag");
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

    FrameGraphResource DeferredLightingPass::AddToGraph(FrameGraph& fg, FrameGraphBlackboard& blackboard)
    {
        const auto [frameUniform] = blackboard.get<FrameData>();
        const auto [lightUniform] = blackboard.get<LightData>();

        const auto& shadow  = blackboard.get<ShadowData>();
        const auto& gBuffer = blackboard.get<GBufferData>();
        // test
        const auto& grass            = blackboard.get<GrassData>();
        const auto& brdf             = blackboard.get<BRDFData>();
        const auto& globalLightProbe = blackboard.get<GlobalLightProbeData>();
        const auto& ssao             = blackboard.get<SSAOData>();
        const auto  extent           = fg.getDescriptor<FrameGraphTexture>(gBuffer.Depth).Extent;

        struct Data
        {
            FrameGraphResource SceneColor;
        };
        const auto& deferredLighting = fg.addCallbackPass<Data>(
            "Deferred Lighting Pass",
            [&](FrameGraph::Builder& builder, Data& data) {
                builder.read(frameUniform);
                builder.read(lightUniform);

                builder.read(shadow.CascadedUniformBuffer);
                builder.read(shadow.CascadedShadowMaps);

                builder.read(gBuffer.Position);
                builder.read(gBuffer.Normal);
                builder.read(gBuffer.Albedo);
                builder.read(gBuffer.Emissive);
                builder.read(gBuffer.MetallicRoughnessAO);

                builder.read(grass.GrassTarget);

                builder.read(brdf.LUT);
                builder.read(globalLightProbe.Diffuse);
                builder.read(globalLightProbe.Specular);

                builder.read(ssao.SSAO);

                data.SceneColor =
                    builder.create<FrameGraphTexture>("SceneColor", {.Extent = extent, .Format = PixelFormat::RGB16F});
                data.SceneColor = builder.write(data.SceneColor);
            },
            [=, this](const Data& data, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("Deferred Lighting Pass");
                SNOW_LEOPARD_PROFILE_GL("Deferred Lighting Pass");

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
                    .BindUniformBuffer(1, getBuffer(resources, lightUniform))
                    .BindUniformBuffer(2, getBuffer(resources, shadow.CascadedUniformBuffer))
                    .BindTexture(0, getTexture(resources, gBuffer.Position))
                    .BindTexture(1, getTexture(resources, gBuffer.Normal))
                    .BindTexture(2, getTexture(resources, gBuffer.Albedo))
                    .BindTexture(3, getTexture(resources, gBuffer.Emissive))
                    .BindTexture(4, getTexture(resources, gBuffer.MetallicRoughnessAO))
                    .BindTexture(5, getTexture(resources, shadow.CascadedShadowMaps))
                    .BindTexture(6, getTexture(resources, brdf.LUT))
                    .BindTexture(7, getTexture(resources, globalLightProbe.Diffuse))
                    .BindTexture(8, getTexture(resources, globalLightProbe.Specular))
                    .BindTexture(9, getTexture(resources, ssao.SSAO))
                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });

        return deferredLighting.SceneColor;
    }
} // namespace SnowLeopardEngine