#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ShadowPrePass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Math/Frustum.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/Tools/ShadowCascadesBuilder.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/LightData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/ShadowData.h"

#include "SnowLeopardEngine/Function/Scene/Components.h"
#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    const uint32_t kShadowMapSize = 2048;
    const uint32_t kNumCascades   = 4;

    // clang-format off
#if GLM_CONFIG_CLIP_CONTROL & GLM_CLIP_CONTROL_ZO_BIT
    const glm::mat4 kBiasMatrix{
      0.5, 0.0, 0.0, 0.0,
      0.0, 0.5, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.5, 0.5, 0.0, 1.0
    };
#else
    const glm::mat4 kBiasMatrix{
      0.5, 0.0, 0.0, 0.0,
      0.0, 0.5, 0.0, 0.0,
      0.0, 0.0, 0.5, 0.0,
      0.5, 0.5, 0.5, 1.0
    };
#endif
    // clang-format on

    struct CascadesUniform
    {
        glm::vec4 SplitDepth;
        glm::mat4 LightSpaceMatrices[kNumCascades]; // 4 x cascades
    };

    template<typename Func>
    requires std::is_invocable_v<Func, const AABB&> auto getVisibleShadowCasters(std::span<Renderable> renderables,
                                                                                 Func                  isVisible)
    {
        std::vector<Renderable> result;
        for (const auto& renderable : renderables)
        {
            if (isOpaque(renderable) && !isGrass(renderable) && isVisible(renderable.BoundingBox))
            {
                result.push_back(renderable);
            }
        }
        return result;
    }

    void uploadCascades(FrameGraph& fg, FrameGraphBlackboard& blackboard, std::vector<ShadowCascade>&& cascades)
    {
        auto& cascadedUniformBuffer = blackboard.get<ShadowData>().CascadedUniformBuffer;

        fg.addCallbackPass(
            "Upload Shadow Cascades",
            [&](FrameGraph::Builder& builder, auto&) { cascadedUniformBuffer = builder.write(cascadedUniformBuffer); },
            [=, cascades = std::move(cascades)](const auto&, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("UploadCascades");
                SNOW_LEOPARD_PROFILE_GL("UploadCascades");

                CascadesUniform cascadesUniform {};
                for (uint32_t i = 0; i < cascades.size(); ++i)
                {
                    cascadesUniform.SplitDepth[i]         = cascades[i].SplitDepth;
                    cascadesUniform.LightSpaceMatrices[i] = kBiasMatrix * cascades[i].LightSpaceMatrices;
                }
                static_cast<RenderContext*>(ctx)->Upload(
                    getBuffer(resources, cascadedUniformBuffer), 0, sizeof(CascadesUniform), &cascadesUniform);
            });
    }

    ShadowPrePass::ShadowPrePass(RenderContext& rc) : BaseGeometryPass(rc)
    {
        m_CascadedUniformBuffer = rc.CreateBuffer(sizeof(CascadesUniform));
    }

    ShadowPrePass::~ShadowPrePass() { m_RenderContext.Destroy(m_CascadedUniformBuffer); }

    void ShadowPrePass::BuildCascadedShadowMaps(FrameGraph&           fg,
                                                FrameGraphBlackboard& blackboard,
                                                Entity                cameraEntity,
                                                Entity                lightEntity,
                                                std::span<Renderable> renderables)
    {
        auto& shadowMapData                 = blackboard.add<ShadowData>();
        shadowMapData.CascadedUniformBuffer = importBuffer(fg, "Cascaded Uniform Buffer", &m_CascadedUniformBuffer);

        auto& light = lightEntity.GetComponent<DirectionalLightComponent>();

        auto cascades = buildCascades(cameraEntity, light.Direction, kNumCascades, 0.94f, kShadowMapSize);
        std::optional<FrameGraphResource> cascadedShadowMaps;
        for (uint32_t i = 0; i < cascades.size(); ++i)
        {
            const auto& lightViewProj        = cascades[i].LightSpaceMatrices;
            auto        visibleShadowCasters = getVisibleShadowCasters(
                renderables, [frustum = Frustum {lightViewProj}](const AABB& aabb) { return frustum.TestAABB(aabb); });

            cascadedShadowMaps =
                AddCascadePass(fg, blackboard, cascadedShadowMaps, lightViewProj, std::move(visibleShadowCasters), i);
        }
        assert(cascadedShadowMaps);
        shadowMapData.CascadedShadowMaps = *cascadedShadowMaps;
        uploadCascades(fg, blackboard, std::move(cascades));
    }

    GraphicsPipeline ShadowPrePass::CreateBasePassPipeline(const VertexFormat& vertexFormat, const Material*)
    {
        auto vertexArrayObject = m_RenderContext.GetVertexArray(vertexFormat.GetAttributes());

        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/ShadowPrePass.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/ShadowPrePass.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        return GraphicsPipeline::Builder {}
            .SetDepthStencil({
                .DepthTest      = true,
                .DepthWrite     = true,
                .DepthCompareOp = CompareOp::LessOrEqual,
            })
            .SetRasterizerState({
                .PolygonMode      = PolygonMode::Fill,
                .CullMode         = CullMode::Front, // Avoid peter panning
                .DepthClampEnable = true,
                .ScissorTest      = false,
            })
            .SetVAO(vertexArrayObject)
            .SetShaderProgram(program)
            .Build();
    }

    FrameGraphResource ShadowPrePass::AddCascadePass(FrameGraph&                       fg,
                                                     FrameGraphBlackboard&             blackboard,
                                                     std::optional<FrameGraphResource> cascadedShadowMaps,
                                                     const glm::mat4&                  lightViewProj,
                                                     std::vector<Renderable>&&         renderables,
                                                     uint32_t                          cascadeIdx)
    {
        assert(cascadeIdx < kNumCascades);
        const auto name            = "CSM #" + std::to_string(cascadeIdx);
        const auto& [frameUniform] = blackboard.get<FrameData>();

        struct Data
        {
            FrameGraphResource Output;
        };
        const auto& pass = fg.addCallbackPass<Data>(
            name,
            [&](FrameGraph::Builder& builder, Data& data) {
                if (cascadeIdx == 0)
                {
                    assert(!cascadedShadowMaps);
                    cascadedShadowMaps =
                        builder.create<FrameGraphTexture>("CascadedShadowMaps",
                                                          {
                                                              .Extent        = {kShadowMapSize, kShadowMapSize},
                                                              .Layers        = kNumCascades,
                                                              .Format        = PixelFormat::Depth24,
                                                              .ShadowSampler = true,
                                                          });
                }
                builder.read(frameUniform);

                data.Output = builder.write(*cascadedShadowMaps);
            },
            [=, this, renderables = std::move(renderables)](
                const Data& data, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER(name);
                TracyGpuZone("CSM");

                constexpr float     kFarPlane {1.0f};
                const RenderingInfo renderingInfo {
                    .Area = {.Extent = {kShadowMapSize, kShadowMapSize}},
                    .DepthAttachment =
                        AttachmentInfo {
                            .Image      = getTexture(resources, data.Output),
                            .Layer      = cascadeIdx,
                            .ClearValue = kFarPlane,
                        },
                };
                auto&      rc          = *static_cast<RenderContext*>(ctx);
                const auto framebuffer = rc.BeginRendering(renderingInfo);
                for (const auto& renderable : renderables)
                {
                    if (renderable.Mesh->Skinned())
                    {
                        rc.Upload(*renderable.Mesh->Data.VertBuffer,
                                  0,
                                  renderable.Mesh->Data.Vertices.size() * sizeof(MeshVertexData),
                                  renderable.Mesh->Data.Vertices.data());
                    }

                    rc.BindGraphicsPipeline(GetPipeline(*renderable.Mesh->Data.VertFormat, renderable.Mat))
                        .BindUniformBuffer(0, getBuffer(resources, frameUniform))
                        .SetUniformMat4("shadowLightViewProjection", lightViewProj);

                    SetTransform(renderable.ModelMatrix);

                    rc.BindMaterial(renderable.Mat)
                        .Draw(*renderable.Mesh->Data.VertBuffer,
                              *renderable.Mesh->Data.IdxBuffer,
                              renderable.Mesh->Data.Indices.size(),
                              renderable.Mesh->Data.Vertices.size());
                }
                rc.EndRendering(framebuffer);
            });

        return pass.Output;
    }
} // namespace SnowLeopardEngine