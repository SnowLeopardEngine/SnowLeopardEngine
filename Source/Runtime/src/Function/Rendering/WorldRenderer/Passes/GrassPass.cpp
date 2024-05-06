#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GrassPass.h"
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
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GrassData.h"

#include <cstdint>
#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    GrassPass::GrassPass(RenderContext& renderContext) : BaseGeometryPass(renderContext) {}

    void GrassPass::AddToGraph(FrameGraph& fg, FrameGraphBlackboard& blackboard, const RenderableGroups& groups)
    {
        const auto [frameUniform] = blackboard.get<FrameData>();
        const auto& gBuffer       = blackboard.get<GBufferData>();
        const auto& extent        = fg.getDescriptor<FrameGraphTexture>(gBuffer.Depth).Extent;

        blackboard.add<GrassData>() = fg.addCallbackPass<GrassData>(
            "Grass Pass",
            [&](FrameGraph::Builder& builder, GrassData& data) {
                builder.read(frameUniform);
                builder.read(gBuffer.Depth);

                data.GrassTarget = builder.create<FrameGraphTexture>(
                    "Grass Target", {.Extent = extent, .Format = PixelFormat::RGBA16F});
                data.GrassTarget = builder.write(data.GrassTarget);
            },
            [=, this](const GrassData& data, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("Grass Pass");
                SNOW_LEOPARD_PROFILE_GL("Grass Pass");

                auto& rc = *static_cast<RenderContext*>(ctx);

                constexpr glm::vec4 kBlackColor {0.0f};

                RenderingInfo renderingInfo = {
                    .Area             = {.Extent = extent},
                    .ColorAttachments = {{.Image = getTexture(resources, data.GrassTarget), .ClearValue = kBlackColor}},
                    .DepthAttachment  = AttachmentInfo {.Image = getTexture(resources, gBuffer.Depth)}};

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
                                .SetUniformVec3("windDirection", glm::vec3(1, 0, 1))
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

    GraphicsPipeline GrassPass::CreateBasePassPipeline(const VertexFormat& vertexFormat, const Material* material)
    {
        auto vertexArrayObject = m_RenderContext.GetVertexArray(vertexFormat.GetAttributes());

        std::vector<std::variant<std::string, std::tuple<std::string, std::string>>> defines;

        if (material->GetDefine().EnableInstancing)
        {
            defines.emplace_back("ENABLE_INSTANCING");
        }

        auto vertResult =
            ShaderCompiler::Compile(!material->GetDefine().UserVertPath.empty() ? material->GetDefine().UserVertPath :
                                                                                  "Assets/Shaders/Grass.vert",
                                    defines);
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult =
            ShaderCompiler::Compile(!material->GetDefine().UserFragPath.empty() ? material->GetDefine().UserFragPath :
                                                                                  "Assets/Shaders/Grass.frag");
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
                .CullMode    = CullMode::None,
                .ScissorTest = false,
            })
            .SetVAO(vertexArrayObject)
            .SetShaderProgram(program)
            .Build();
    }
} // namespace SnowLeopardEngine