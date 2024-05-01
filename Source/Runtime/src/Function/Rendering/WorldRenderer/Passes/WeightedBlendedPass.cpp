#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/WeightedBlendedPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/BRDFData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GBufferData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GlobalLightProbeData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/ShadowData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/WeightedBlendedData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    WeightedBlendedPass::WeightedBlendedPass(RenderContext& renderContext) : BaseGeometryPass(renderContext) {}

    void WeightedBlendedPass::AddToGraph(FrameGraph&           fg,
                                         FrameGraphBlackboard& blackboard,
                                         std::span<Renderable> transparentRenderables)
    {
        const auto [frameUniform]    = blackboard.get<FrameData>();
        const auto& shadow           = blackboard.get<ShadowData>();
        const auto& gBuffer          = blackboard.get<GBufferData>();
        const auto& brdf             = blackboard.get<BRDFData>();
        const auto& globalLightProbe = blackboard.get<GlobalLightProbeData>();
        const auto  extent           = fg.getDescriptor<FrameGraphTexture>(gBuffer.Depth).Extent;

        blackboard.add<WeightedBlendedData>() = fg.addCallbackPass<WeightedBlendedData>(
            "Weighted Blended OIT Pass",
            [&](FrameGraph::Builder& builder, WeightedBlendedData& data) {
                builder.read(frameUniform);
                builder.read(gBuffer.Depth);

                builder.read(shadow.ShadowMap);

                builder.read(brdf.LUT);
                builder.read(globalLightProbe.Diffuse);
                builder.read(globalLightProbe.Specular);

                data.Accum =
                    builder.create<FrameGraphTexture>("Accum", {.Extent = extent, .Format = PixelFormat::RGBA16F});
                data.Accum = builder.write(data.Accum);

                data.Reveal =
                    builder.create<FrameGraphTexture>("Reveal", {.Extent = extent, .Format = PixelFormat::R8_UNorm});
                data.Reveal = builder.write(data.Reveal);
            },
            [=, this](const WeightedBlendedData& data, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("Weighted Blended OIT Pass");
                SNOW_LEOPARD_PROFILE_GL("Weighted Blended OIT Pass");

                auto& rc = *static_cast<RenderContext*>(ctx);

                RenderingInfo renderingInfo = {
                    .Area = {.Extent = extent},
                    .ColorAttachments =
                        {
                            {.Image = getTexture(resources, data.Accum), .ClearValue = glm::vec4(0.0f)},
                            {.Image = getTexture(resources, data.Reveal), .ClearValue = glm::vec4(1.0f)},
                        },
                    .DepthAttachment = AttachmentInfo {.Image = getTexture(resources, gBuffer.Depth)}};

                auto frameBuffer = rc.BeginRendering(renderingInfo);

                for (const auto& renderable : transparentRenderables)
                {
                    // Skinned Mesh
                    if (renderable.Mesh->Skinned())
                    {
                        rc.Upload(*renderable.Mesh->Data.VertBuffer,
                                  0,
                                  renderable.Mesh->Data.Vertices.size() * sizeof(MeshVertexData),
                                  renderable.Mesh->Data.Vertices.data());
                    }

                    rc.BindGraphicsPipeline(GetPipeline(*renderable.Mesh->Data.VertFormat, renderable.Mat))
                        .BindUniformBuffer(0, getBuffer(resources, frameUniform));

                    SetTransform(renderable.ModelMatrix);

                    rc.BindMaterial(renderable.Mat)
                        .BindTexture(5, getTexture(resources, shadow.ShadowMap))
                        .BindTexture(6, getTexture(resources, brdf.LUT))
                        .BindTexture(7, getTexture(resources, globalLightProbe.Diffuse))
                        .BindTexture(8, getTexture(resources, globalLightProbe.Specular))
                        .Draw(*renderable.Mesh->Data.VertBuffer,
                              *renderable.Mesh->Data.IdxBuffer,
                              renderable.Mesh->Data.Indices.size(),
                              renderable.Mesh->Data.Vertices.size());
                }

                rc.EndRendering(frameBuffer);
            });
    }

    GraphicsPipeline WeightedBlendedPass::CreateBasePassPipeline(const VertexFormat& vertexFormat,
                                                                 const Material*     material)
    {
        auto vertexArrayObject = m_RenderContext.GetVertexArray(vertexFormat.GetAttributes());

        std::vector<std::variant<std::string, std::tuple<std::string, std::string>>> defines;

        if (material->GetDefine().EnableInstancing)
        {
            defines.emplace_back("ENABLE_INSTANCING");
        }

        auto vertResult =
            ShaderCompiler::Compile(!material->GetDefine().UserVertPath.empty() ? material->GetDefine().UserVertPath :
                                                                                  "Assets/Shaders/Geometry.vert",
                                    defines);
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile(!material->GetDefine().UserFragPath.empty() ?
                                                      material->GetDefine().UserFragPath :
                                                      "Assets/Shaders/WeightedBlendedPass.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        return GraphicsPipeline::Builder {}
            .SetDepthStencil({
                .DepthTest      = true,
                .DepthWrite     = false,
                .DepthCompareOp = CompareOp::LessOrEqual,
            })
            .SetBlendState(0,
                           {
                               .Enabled   = true,
                               .SrcColor  = BlendFactor::One,
                               .DestColor = BlendFactor::One,
                               .SrcAlpha  = BlendFactor::One,
                               .DestAlpha = BlendFactor::One,
                           })
            .SetBlendState(1,
                           {
                               .Enabled   = true,
                               .SrcColor  = BlendFactor::Zero,
                               .DestColor = BlendFactor::OneMinusSrcColor,
                               .SrcAlpha  = BlendFactor::Zero,
                               .DestAlpha = BlendFactor::OneMinusSrcAlpha,
                           })
            .SetRasterizerState({
                .PolygonMode = PolygonMode::Fill,
                .CullMode    = CullMode::None,
                .ScissorTest = false,
            })
            .SetVAO(vertexArrayObject)
            .SetShaderProgram(program)
            .Build();
    }
} // namespace SnowLeopardEngine