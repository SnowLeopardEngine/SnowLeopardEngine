#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/InGameGUIPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/InGameGUIData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    InGameGUIPass::InGameGUIPass(RenderContext& renderContext) : BaseGeometryPass(renderContext) {}

    void InGameGUIPass::AddToGraph(FrameGraph&           fg,
                                   FrameGraphBlackboard& blackboard,
                                   const Extent2D&       resolution,
                                   std::span<Renderable> uiRenderables)
    {
        const auto [frameUniform] = blackboard.get<FrameData>();

        blackboard.add<InGameGUIData>() = fg.addCallbackPass<InGameGUIData>(
            "In-Game GUI Pass",
            [&, resolution](FrameGraph::Builder& builder, InGameGUIData& data) {
                builder.read(frameUniform);

                data.UITarget = builder.create<FrameGraphTexture>(
                    "UI Target", {.Extent = resolution, .Format = PixelFormat::RGBA16F});
                data.UITarget = builder.write(data.UITarget);

                data.UIDepth = builder.create<FrameGraphTexture>(
                    "UI Depth", {.Extent = resolution, .Format = PixelFormat::Depth32F});
                data.UIDepth = builder.write(data.UIDepth);
            },
            [=, this](const InGameGUIData& data, FrameGraphPassResources& resources, void* ctx) {
                auto& rc = *static_cast<RenderContext*>(ctx);

                constexpr glm::vec4 kTransparentBlack {0.0f};
                constexpr float     kFarPlane {1.0f};

                RenderingInfo renderingInfo = {
                    .Area             = {.Extent = resolution},
                    .ColorAttachments = {{.Image      = getTexture(resources, data.UITarget),
                                          .ClearValue = kTransparentBlack}},
                    .DepthAttachment =
                        AttachmentInfo {.Image = getTexture(resources, data.UIDepth), .ClearValue = kFarPlane}};

                auto frameBuffer = rc.BeginRendering(renderingInfo);

                for (const auto& renderable : uiRenderables)
                {
                    assert(renderable.Type == RenderableType::UI);

                    rc.BindGraphicsPipeline(GetPipeline(*renderable.Mesh->Data.VertFormat, renderable.Mat))
                        .BindUniformBuffer(0, getBuffer(resources, frameUniform));

                    SetTransform(renderable.ModelMatrix);

                    if (renderable.UISpecs.ButtonColorTintTexture)
                    {
                        rc.SetUniform1i("useImage", 1);
                        rc.SetUniformVec4("baseColor", renderable.UISpecs.ButtonColorTintCurrentColor);
                        rc.BindTexture(0, *renderable.UISpecs.ButtonColorTintTexture);
                    }
                    else if (renderable.UISpecs.ImageTexture)
                    {
                        rc.SetUniform1i("useImage", 1);
                        rc.SetUniformVec4("baseColor", renderable.UISpecs.ImageColor);
                        rc.BindTexture(0, *renderable.UISpecs.ImageTexture);
                    }
                    else
                    {
                        rc.SetUniform1i("useImage", 0);
                        rc.SetUniformVec4("baseColor", glm::vec4(1.0f));
                    }

                    rc.Draw(*renderable.Mesh->Data.VertBuffer,
                            *renderable.Mesh->Data.IdxBuffer,
                            renderable.Mesh->Data.Indices.size(),
                            renderable.Mesh->Data.Vertices.size());
                }

                rc.EndRendering(frameBuffer);
            });
    }

    GraphicsPipeline InGameGUIPass::CreateBasePassPipeline(const VertexFormat& vertexFormat, const Material* material)
    {
        auto vertexArrayObject = m_RenderContext.GetVertexArray(vertexFormat.GetAttributes());

        auto vertResult =
            ShaderCompiler::Compile(!material->GetDefine().UserVertPath.empty() ? material->GetDefine().UserVertPath :
                                                                                  "Assets/Shaders/UI.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult =
            ShaderCompiler::Compile(!material->GetDefine().UserFragPath.empty() ? material->GetDefine().UserFragPath :
                                                                                  "Assets/Shaders/UI.frag");
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