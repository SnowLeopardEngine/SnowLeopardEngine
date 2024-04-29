#include "SnowLeopardEngine/Function/Rendering/Converters/CubemapConverter.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/Converters/CubemapCapture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"

namespace SnowLeopardEngine
{
    static uint32_t calculateCubeSize(const Texture& equirectangular)
    {
        assert(equirectangular.GetType() == GL_TEXTURE_2D);
        const auto x = 8 * (glm::floor(2 * equirectangular.GetExtent().Height / glm::pi<float>() / 8));
        return Math::NextPowerOf2(x);
    }

    CubemapConverter::CubemapConverter(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/Cube.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/EquirectangularToCubemap.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_Pipeline = GraphicsPipeline::Builder {}
                         .SetDepthStencil({.DepthTest = false, .DepthWrite = false})
                         .SetRasterizerState({.CullMode = CullMode::Front})
                         .SetShaderProgram(program)
                         .Build();
    }

    CubemapConverter::~CubemapConverter() { m_RenderContext.Destroy(m_Pipeline); }

    Texture CubemapConverter::EquirectangularToCubemap(const Texture& equirectangular)
    {
        const auto size    = calculateCubeSize(equirectangular);
        auto       cubemap = m_RenderContext.CreateCubemap(size, PixelFormat::RGB16F, 0);
        m_RenderContext.SetupSampler(cubemap,
                                     {
                                         .MinFilter    = TexelFilter::Linear,
                                         .MipmapMode   = MipmapMode::Linear,
                                         .MagFilter    = TexelFilter::Linear,
                                         .AddressModeS = SamplerAddressMode::ClampToEdge,
                                         .AddressModeT = SamplerAddressMode::ClampToEdge,
                                         .AddressModeR = SamplerAddressMode::ClampToEdge,
                                     });

        for (uint8_t face = 0; face < 6; ++face)
        {
            const RenderingInfo renderingInfo {
                .Area             = {.Extent = {size, size}},
                .ColorAttachments = {AttachmentInfo {
                    .Image      = cubemap,
                    .Face       = face,
                    .ClearValue = glm::vec4 {0.0f},
                }},
            };
            const auto framebuffer = m_RenderContext.BeginRendering(renderingInfo);
            m_RenderContext.BindGraphicsPipeline(m_Pipeline)
                .BindTexture(0, equirectangular)
                .SetUniformMat4("VP", kCubeProjection * kCaptureViews[face])
                .DrawCube()
                .EndRendering(framebuffer);
        }
        m_RenderContext.GenerateMipmaps(cubemap);

        return cubemap;
    }
} // namespace SnowLeopardEngine