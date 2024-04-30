#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/SSAOPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GBufferData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/SSAOData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    SSAOPass::SSAOPass(RenderContext& rc, uint32_t kernelSize) : m_RenderContext(rc)
    {
        GenerateNoiseTexture();
        GenerateSampleKernel(kernelSize);
        CreatePipeline(kernelSize);
    }

    SSAOPass::~SSAOPass() { m_RenderContext.Destroy(m_Pipeline).Destroy(m_Noise).Destroy(m_KernelBuffer); }

    void SSAOPass::AddToGraph(FrameGraph& fg, FrameGraphBlackboard& blackboard)
    {
        const auto [frameUniform] = blackboard.get<FrameData>();

        const auto& gBuffer = blackboard.get<GBufferData>();
        const auto  extent  = fg.getDescriptor<FrameGraphTexture>(gBuffer.Depth).Extent;

        blackboard.add<SSAOData>() = fg.addCallbackPass<SSAOData>(
            "SSAO Pass",
            [&](FrameGraph::Builder& builder, SSAOData& data) {
                builder.read(frameUniform);
                builder.read(gBuffer.Depth);
                builder.read(gBuffer.Normal);

                data.SSAO =
                    builder.create<FrameGraphTexture>("SSAO Pass", {.Extent = extent, .Format = PixelFormat::R8_UNorm});
                data.SSAO = builder.write(data.SSAO);
            },
            [=, this](const SSAOData& data, FrameGraphPassResources& resources, void* ctx) {
                const RenderingInfo renderingInfo {
                    .Area             = {.Extent = extent},
                    .ColorAttachments = {{
                        .Image      = getTexture(resources, data.SSAO),
                        .ClearValue = glm::vec4 {1.0f},
                    }},
                };

                auto&      rc          = *static_cast<RenderContext*>(ctx);
                const auto framebuffer = rc.BeginRendering(renderingInfo);
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindUniformBuffer(0, getBuffer(resources, frameUniform))

                    .BindTexture(0, getTexture(resources, gBuffer.Depth))
                    .BindTexture(1, getTexture(resources, gBuffer.Normal))
                    .BindTexture(2, m_Noise)

                    .BindUniformBuffer(1, m_KernelBuffer)

                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });
    }

    void SSAOPass::GenerateNoiseTexture()
    {
        constexpr auto kSize = 4u;

        std::uniform_real_distribution<float> dist {0.0, 1.0};
        std::random_device                    rd {};
        std::default_random_engine            g {rd()};
        std::vector<glm::vec3>                ssaoNoise;
        std::generate_n(std::back_inserter(ssaoNoise), kSize * kSize, [&] {
            // Rotate around z-axis (in tangent space)
            return glm::vec3 {dist(g) * 2.0 - 1.0, dist(g) * 2.0 - 1.0, 0.0f};
        });

        m_Noise = m_RenderContext.CreateTexture2D({kSize, kSize}, PixelFormat::RGB16F);
        m_RenderContext
            .SetupSampler(m_Noise,
                          {
                              .MinFilter    = TexelFilter::Linear,
                              .MipmapMode   = MipmapMode::None,
                              .MagFilter    = TexelFilter::Linear,
                              .AddressModeS = SamplerAddressMode::Repeat,
                              .AddressModeT = SamplerAddressMode::Repeat,
                          })
            .Upload(m_Noise,
                    0,
                    {kSize, kSize},
                    {
                        .Format   = GL_RGB,
                        .DataType = GL_FLOAT,
                        .Pixels   = ssaoNoise.data(),
                    });
    }

    void SSAOPass::GenerateSampleKernel(uint32_t kernelSize)
    {
        std::uniform_real_distribution<float> dist {0.0f, 1.0f};
        std::random_device                    rd {};
        std::default_random_engine            g {rd()};

        std::vector<glm::vec4> ssaoKernel;
        std::generate_n(std::back_inserter(ssaoKernel), kernelSize, [&, i = 0]() mutable {
            glm::vec3 sample {
                dist(g) * 2.0f - 1.0f,
                dist(g) * 2.0f - 1.0f,
                dist(g),
            };
            sample = glm::normalize(sample);
            sample *= dist(g);

            auto scale = static_cast<float>(i++) / kernelSize;
            scale      = glm::mix(0.1f, 1.0f, scale * scale);
            return glm::vec4 {sample * scale, 0.0f};
        });

        m_KernelBuffer = m_RenderContext.CreateBuffer(sizeof(glm::vec4) * kernelSize, ssaoKernel.data());
    }

    void SSAOPass::CreatePipeline(uint32_t kernelSize)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/SSAOPass.frag",
                                                  {std::make_tuple("KERNEL_SIZE", std::to_string(kernelSize))});
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_Pipeline = GraphicsPipeline::Builder {}
                         .SetDepthStencil({
                             .DepthTest  = false,
                             .DepthWrite = false,
                         })
                         .SetRasterizerState({
                             .PolygonMode = PolygonMode::Fill,
                             .CullMode    = CullMode::Back,
                             .ScissorTest = false,
                         })
                         .SetShaderProgram(program)
                         .Build();
    }
} // namespace SnowLeopardEngine