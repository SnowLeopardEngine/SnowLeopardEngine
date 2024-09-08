#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GBufferPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GBufferData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    GBufferPass::GBufferPass(RenderContext& renderContext) : BaseGeometryPass(renderContext) {}

    void GBufferPass::AddToGraph(FrameGraph&             fg,
                                 FrameGraphBlackboard&   blackboard,
                                 const Extent2D&         resolution,
                                 const RenderableGroups& groups)
    {
        const auto frameUniform = blackboard.get<FrameData>().FrameUniform;

        blackboard.add<GBufferData>() = fg.addCallbackPass<GBufferData>(
            "GBuffer Pass",
            [&, resolution](FrameGraph::Builder& builder, GBufferData& data) {
                builder.read(frameUniform);

                data.Position = builder.create<FrameGraphTexture>(
                    "Position", {.Extent = resolution, .Format = PixelFormat::RGB16F});
                data.Position = builder.write(data.Position);

                data.Normal =
                    builder.create<FrameGraphTexture>("Normal", {.Extent = resolution, .Format = PixelFormat::RGB16F});
                data.Normal = builder.write(data.Normal);

                data.Albedo =
                    builder.create<FrameGraphTexture>("Albedo", {.Extent = resolution, .Format = PixelFormat::RGB16F});
                data.Albedo = builder.write(data.Albedo);

                data.Emissive = builder.create<FrameGraphTexture>(
                    "Emissive", {.Extent = resolution, .Format = PixelFormat::RGB16F});
                data.Emissive = builder.write(data.Emissive);

                data.MetallicRoughnessAO = builder.create<FrameGraphTexture>(
                    "Metallic Roughness AO", {.Extent = resolution, .Format = PixelFormat::RGB8_UNorm});
                data.MetallicRoughnessAO = builder.write(data.MetallicRoughnessAO);

                data.EntityID =
                    builder.create<FrameGraphTexture>("EntityID", {.Extent = resolution, .Format = PixelFormat::R32I});
                data.EntityID = builder.write(data.EntityID);

                data.Depth =
                    builder.create<FrameGraphTexture>("Depth", {.Extent = resolution, .Format = PixelFormat::Depth32F});
                data.Depth = builder.write(data.Depth);
            },
            [=, this](const GBufferData& data, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("GBuffer Pass");
                SNOW_LEOPARD_PROFILE_GL("GBuffer Pass");

                auto& rc = *static_cast<RenderContext*>(ctx);

                constexpr glm::vec4 kBlackColor {0.0f};
                constexpr float     kFarPlane {1.0f};

                RenderingInfo renderingInfo = {
                    .Area             = {.Extent = resolution},
                    .ColorAttachments = {{.Image = getTexture(resources, data.Position), .ClearValue = kBlackColor},
                                         {.Image = getTexture(resources, data.Normal), .ClearValue = kBlackColor},
                                         {.Image = getTexture(resources, data.Albedo), .ClearValue = kBlackColor},
                                         {.Image = getTexture(resources, data.Emissive), .ClearValue = kBlackColor},
                                         {.Image      = getTexture(resources, data.MetallicRoughnessAO),
                                          .ClearValue = kBlackColor},
                                         {.Image = getTexture(resources, data.EntityID), .ClearValue = kBlackColor}},
                    .DepthAttachment =
                        AttachmentInfo {.Image = getTexture(resources, data.Depth), .ClearValue = kFarPlane}};

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
                                .BindUniformBuffer(0, getBuffer(resources, frameUniform));

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
                            .BindUniformBuffer(0, getBuffer(resources, frameUniform));

                        const uint32_t totalRenderables = group.Renderables.size();
                        const uint32_t batchSize        = 250;

                        for (uint32_t startIdx = 0; startIdx < totalRenderables; startIdx += batchSize)
                        {
                            const uint32_t endIdx        = std::min(startIdx + batchSize, totalRenderables);
                            const uint32_t instanceCount = endIdx - startIdx;

                            for (uint32_t instanceId = startIdx; instanceId < endIdx; ++instanceId)
                            {
                                SetTransform(group.Renderables[instanceId].ModelMatrix, instanceId - startIdx);
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
                }

                rc.EndRendering(frameBuffer);
            });
    }

    GraphicsPipeline GBufferPass::CreateBasePassPipeline(const VertexFormat& vertexFormat, const Material* material)
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

        auto fragResult =
            ShaderCompiler::Compile(!material->GetDefine().UserFragPath.empty() ? material->GetDefine().UserFragPath :
                                                                                  "Assets/Shaders/GBufferPass.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        return GraphicsPipeline::Builder {}
            .SetDepthStencil({
                .DepthTest      = true,
                .DepthWrite     = true,
                .DepthCompareOp = CompareOp::LessOrEqual,
            })
            .SetRasterizerState({
                .PolygonMode = PolygonMode::Fill,
                .CullMode    = CullMode::Back,
                .ScissorTest = false,
            })
            .SetVAO(vertexArrayObject)
            .SetShaderProgram(program)
            .Build();
    }
} // namespace SnowLeopardEngine