#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GBufferPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GBufferData.h"

namespace SnowLeopardEngine
{
    GBufferPass::GBufferPass(RenderContext& renderContext) : BaseGeometryPass(renderContext) {}

    void GBufferPass::AddToGraph(FrameGraph&           fg,
                                 FrameGraphBlackboard& blackboard,
                                 const Extent2D&       resolution,
                                 std::span<Renderable> renderables)
    {
        const auto [frameUniform] = blackboard.get<FrameData>();

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

                data.MetallicRoughnessAO = builder.create<FrameGraphTexture>(
                    "Metallic Roughness AO", {.Extent = resolution, .Format = PixelFormat::RGBA8_UNorm});
                data.MetallicRoughnessAO = builder.write(data.MetallicRoughnessAO);

                data.EntityID =
                    builder.create<FrameGraphTexture>("EntityID", {.Extent = resolution, .Format = PixelFormat::R32I});
                data.EntityID = builder.write(data.EntityID);

                data.Depth =
                    builder.create<FrameGraphTexture>("Depth", {.Extent = resolution, .Format = PixelFormat::Depth32F});
                data.Depth = builder.write(data.Depth);
            },
            [=, this](const GBufferData& data, FrameGraphPassResources& resources, void* ctx) {
                auto& rc = *static_cast<RenderContext*>(ctx);

                constexpr glm::vec4 kBlackColor {0.0f};
                constexpr float     kFarPlane {1.0f};

                RenderingInfo renderingInfo = {
                    .Area             = {.Extent = resolution},
                    .ColorAttachments = {{.Image = getTexture(resources, data.Position), .ClearValue = kBlackColor},
                                         {.Image = getTexture(resources, data.Normal), .ClearValue = kBlackColor},
                                         {.Image = getTexture(resources, data.Albedo), .ClearValue = kBlackColor},
                                         {.Image      = getTexture(resources, data.MetallicRoughnessAO),
                                          .ClearValue = kBlackColor},
                                         {.Image = getTexture(resources, data.EntityID), .ClearValue = kBlackColor}},
                    .DepthAttachment =
                        AttachmentInfo {.Image = getTexture(resources, data.Depth), .ClearValue = kFarPlane}};

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
                        .BindUniformBuffer(0, getBuffer(resources, frameUniform));

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

    GraphicsPipeline GBufferPass::CreateBasePassPipeline(const VertexFormat& vertexFormat, const Material* material)
    {
        auto vertexArrayObject = m_RenderContext.GetVertexArray(vertexFormat.GetAttributes());

        auto vertResult =
            ShaderCompiler::Compile(!material->GetDefine().UserVertPath.empty() ? material->GetDefine().UserVertPath :
                                                                                  "Assets/Shaders/Geometry.vert");
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