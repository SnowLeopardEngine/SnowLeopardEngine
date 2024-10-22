#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/InGameGUIPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/Character.h"
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
                NAMED_DEBUG_MARKER("In-Game GUI Pass");
                SNOW_LEOPARD_PROFILE_GL("In-Game GUI Pass");

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

                    // Text
                    if (renderable.UISpecs.Type == RenderableUISpecs::UISpecType::Text)
                    {
                        assert(renderable.Mesh->Data.Vertices.size() == 4);

                        const float     scale = renderable.UISpecs.FontSize * 1.0f / 12.0f;
                        const glm::vec3 pos   = renderable.UISpecs.TextPos;
                        float           x     = pos.x;
                        float           y     = pos.y;

                        for (const auto& c : renderable.UISpecs.Text)
                        {
                            const auto& ch = g_Characters[c];

                            GLfloat xpos = x + ch.Bearing.x * scale;
                            GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

                            GLfloat w = ch.Size.x * scale;
                            GLfloat h = ch.Size.y * scale;

                            renderable.Mesh->Data.Vertices[0].Position = {xpos, ypos + h, 0};
                            renderable.Mesh->Data.Vertices[1].Position = {xpos, ypos, 0};
                            renderable.Mesh->Data.Vertices[2].Position = {xpos + w, ypos, 0};
                            renderable.Mesh->Data.Vertices[3].Position = {xpos + w, ypos + h, 0};

                            rc.Upload(*renderable.Mesh->Data.VertBuffer,
                                      0,
                                      renderable.Mesh->Data.Vertices.size() * sizeof(MeshVertexData),
                                      renderable.Mesh->Data.Vertices.data());

                            glBindTextureUnit(0, ch.TextureId);
                            rc.SetUniformVec4("baseColor", renderable.UISpecs.TextColor);

                            rc.Draw(*renderable.Mesh->Data.VertBuffer,
                                    *renderable.Mesh->Data.IdxBuffer,
                                    renderable.Mesh->Data.Indices.size(),
                                    renderable.Mesh->Data.Vertices.size());

                            x += (ch.Advance >> 6) * scale; // Move to next glyph
                        }
                    }
                    // Non-Text
                    else
                    {
                        // Button
                        if (renderable.UISpecs.Type == RenderableUISpecs::UISpecType::Button)
                        {
                            rc.SetUniform1i("useImage", 1);
                            rc.SetUniformVec4("baseColor", renderable.UISpecs.ButtonColorTintCurrentColor);
                            rc.BindTexture(0, *renderable.UISpecs.ButtonColorTintTexture);
                        }
                        // Image
                        else if (renderable.UISpecs.Type == RenderableUISpecs::UISpecType::Image)
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
                                                                                  "Assets/Shaders/UIImage.frag");
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
            .SetBlendState(0,
                           {
                               .Enabled   = true,
                               .SrcColor  = BlendFactor::SrcColor,
                               .DestColor = BlendFactor::OneMinusSrcColor,
                               .SrcAlpha  = BlendFactor::SrcAlpha,
                               .DestAlpha = BlendFactor::OneMinusSrcAlpha,
                           })
            .SetVAO(vertexArrayObject)
            .SetShaderProgram(program)
            .Build();
    }
} // namespace SnowLeopardEngine