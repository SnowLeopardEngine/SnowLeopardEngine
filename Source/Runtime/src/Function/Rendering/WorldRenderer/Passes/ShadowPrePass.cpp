#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ShadowPrePass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/LightData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/ShadowData.h"

namespace SnowLeopardEngine
{
    ShadowPrePass::ShadowPrePass(RenderContext& renderContext) : BaseGeometryPass(renderContext) {}

    void ShadowPrePass::AddToGraph(FrameGraph&           fg,
                                   FrameGraphBlackboard& blackboard,
                                   const Extent2D&       resolution,
                                   std::span<Renderable> renderables)
    {
        const auto [frameUniform] = blackboard.get<FrameData>();
        const auto [lightUniform] = blackboard.get<LightData>();

        blackboard.add<ShadowData>() = fg.addCallbackPass<ShadowData>(
            "Shadow Pre-Pass",
            [&, resolution](FrameGraph::Builder& builder, ShadowData& data) {
                builder.read(frameUniform);
                builder.read(lightUniform);

                data.ShadowMap = builder.create<FrameGraphTexture>(
                    "ShadowMap",
                    {.Extent = resolution, .Format = PixelFormat::Depth32F, .Wrap = WrapMode::ClampToOpaqueWhite});
                data.ShadowMap = builder.write(data.ShadowMap);
            },
            [=, this](const ShadowData& data, FrameGraphPassResources& resources, void* ctx) {
                auto& rc = *static_cast<RenderContext*>(ctx);

                constexpr glm::vec4 kBlackColor {0.0f};
                constexpr float     kFarPlane {1.0f};

                RenderingInfo renderingInfo = {
                    .Area = {.Extent = resolution},
                    .DepthAttachment =
                        AttachmentInfo {.Image = getTexture(resources, data.ShadowMap), .ClearValue = kFarPlane}};

                auto frameBuffer = rc.BeginRendering(renderingInfo);

                for (auto& renderable : renderables)
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
                        .BindUniformBuffer(1, getBuffer(resources, lightUniform));

                    SetTransform(renderable.ModelMatrix);

                    rc.BindMaterial(renderable.Mat)
                        .Draw(*renderable.Mesh->Data.VertBuffer,
                              *renderable.Mesh->Data.IdxBuffer,
                              renderable.Mesh->Data.Indices.size(),
                              renderable.Mesh->Data.Vertices.size());
                }

                rc.EndRendering(frameBuffer);
            });
    }

    GraphicsPipeline ShadowPrePass::CreateBasePassPipeline(const VertexFormat& vertexFormat, const Material*)
    {
        auto vertexArrayObject = m_RenderContext.GetVertexArray(vertexFormat.GetAttributes());

        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/ShadowMapping.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/ShadowMapping.frag");
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
                .CullMode    = CullMode::Front, // Avoid peter panning
                .ScissorTest = false,
            })
            .SetVAO(vertexArrayObject)
            .SetShaderProgram(program)
            .Build();
    }
} // namespace SnowLeopardEngine