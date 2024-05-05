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
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/LightData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/ShadowData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/WeightedBlendedData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    WeightedBlendedPass::WeightedBlendedPass(RenderContext& renderContext) : BaseGeometryPass(renderContext) {}

    void
    WeightedBlendedPass::AddToGraph(FrameGraph& fg, FrameGraphBlackboard& blackboard, const RenderableGroups& groups)
    {
        const auto [frameUniform]    = blackboard.get<FrameData>();
        const auto [lightUniform]    = blackboard.get<LightData>();
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

                builder.read(lightUniform);

                builder.read(shadow.CascadedUniformBuffer);
                builder.read(shadow.CascadedShadowMaps);

                builder.read(gBuffer.Position);
                builder.read(gBuffer.Normal);
                builder.read(gBuffer.Albedo);
                builder.read(gBuffer.Emissive);
                builder.read(gBuffer.MetallicRoughnessAO);

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

                for (const auto& [_, group] : groups)
                {
                    // Default group
                    if (group.GroupType == RenderableGroupType::Default)
                    {
                        for (const auto& renderable : group.Renderables)
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
                                .BindTexture(8, getTexture(resources, globalLightProbe.Specular));

                            SetTransform(renderable.ModelMatrix);

                            rc.BindMaterial(renderable.Mat)
                                .SetUniform1i("tileSize", renderable.Mat->GetDefine().TileSize)
                                .Draw(*renderable.Mesh->Data.VertBuffer,
                                      *renderable.Mesh->Data.IdxBuffer,
                                      renderable.Mesh->Data.Indices.size(),
                                      renderable.Mesh->Data.Vertices.size());
                        }
                    }
                    // Instancing group
                    else if (group.GroupType == RenderableGroupType::Instancing)
                    {
                        assert(!group.Renderables.empty());
                        const auto& renderableTemplate = group.Renderables[0];

                        // TODO: support skinned mesh instancing

                        rc.BindGraphicsPipeline(
                              GetPipeline(*renderableTemplate.Mesh->Data.VertFormat, renderableTemplate.Mat))
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
                            .BindTexture(8, getTexture(resources, globalLightProbe.Specular));

                        uint32_t instanceCount = group.Renderables.size();
                        for (uint32_t instanceId = 0; instanceId < instanceCount; ++instanceId)
                        {
                            SetTransform(group.Renderables[instanceId].ModelMatrix, instanceId);
                        }

                        rc.BindMaterial(renderableTemplate.Mat)
                            .SetUniform1i("tileSize", renderableTemplate.Mat->GetDefine().TileSize)
                            .Draw(*renderableTemplate.Mesh->Data.VertBuffer,
                                  *renderableTemplate.Mesh->Data.IdxBuffer,
                                  renderableTemplate.Mesh->Data.Indices.size(),
                                  renderableTemplate.Mesh->Data.Vertices.size(),
                                  instanceCount);
                    }
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