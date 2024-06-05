#include "SnowLeopardEngine/Function/Rendering/Tools/IBLDataGenerator.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/Tools/CubemapCapture.h"

namespace SnowLeopardEngine
{
    IBLDataGenerator::IBLDataGenerator(RenderContext& rc) : m_RenderContext(rc)
    {
        GraphicsPipeline::Builder builder {};
        builder.SetDepthStencil({.DepthTest = false, .DepthWrite = false});

        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/IBL/GenerateBRDF.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_BrdfPipeline = builder.SetShaderProgram(program).Build();

        vertResult = ShaderCompiler::Compile("Assets/Shaders/Cube.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        fragResult = ShaderCompiler::Compile("Assets/Shaders/IBL/GenerateIrradianceMap.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_IrradiancePipeline =
            builder.SetRasterizerState({.CullMode = CullMode::Front}).SetShaderProgram(program).Build();

        fragResult = ShaderCompiler::Compile("Assets/Shaders/IBL/PrefilterEnvMap.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_PrefilterEnvMapPipeline = builder.SetShaderProgram(program).Build();
    }

    IBLDataGenerator::~IBLDataGenerator()
    {
        m_RenderContext.Destroy(m_BrdfPipeline).Destroy(m_IrradiancePipeline).Destroy(m_PrefilterEnvMapPipeline);
    }

    Texture IBLDataGenerator::GenerateBrdfLUT()
    {
        constexpr auto kSize = 512u;
        auto           brdf  = m_RenderContext.CreateTexture2D({kSize, kSize}, PixelFormat::RG16F);
        m_RenderContext.SetupSampler(brdf,
                                     {
                                         .MinFilter    = TexelFilter::Linear,
                                         .MipmapMode   = MipmapMode::None,
                                         .MagFilter    = TexelFilter::Linear,
                                         .AddressModeS = SamplerAddressMode::ClampToEdge,
                                         .AddressModeT = SamplerAddressMode::ClampToEdge,
                                     });

        const RenderingInfo renderingInfo {
            .Area             = {.Extent = {kSize, kSize}},
            .ColorAttachments = {{
                .Image      = brdf,
                .ClearValue = glm::vec4 {0.0f},
            }},
        };
        const auto framebuffer = m_RenderContext.BeginRendering(renderingInfo);
        m_RenderContext.BindGraphicsPipeline(m_BrdfPipeline).DrawFullScreenTriangle().EndRendering(framebuffer);

        return brdf;
    }

    Texture IBLDataGenerator::GenerateIrradiance(const Texture& cubemap)
    {
        assert(cubemap.GetType() == GL_TEXTURE_CUBE_MAP);

        constexpr auto kSize      = 64u;
        auto           irradiance = m_RenderContext.CreateCubemap(kSize, PixelFormat::RGB16F, 1);
        m_RenderContext.SetupSampler(irradiance,
                                     {
                                         .MinFilter    = TexelFilter::Linear,
                                         .MipmapMode   = MipmapMode::None,
                                         .MagFilter    = TexelFilter::Linear,
                                         .AddressModeS = SamplerAddressMode::ClampToEdge,
                                         .AddressModeT = SamplerAddressMode::ClampToEdge,
                                         .AddressModeR = SamplerAddressMode::ClampToEdge,
                                     });

        for (uint8_t face {0}; face < 6; ++face)
        {
            const RenderingInfo renderingInfo {
                .Area             = {.Extent = {kSize, kSize}},
                .ColorAttachments = {AttachmentInfo {
                    .Image      = irradiance,
                    .Face       = face,
                    .ClearValue = glm::vec4 {0.0f},
                }},
            };
            const auto framebuffer = m_RenderContext.BeginRendering(renderingInfo);
            m_RenderContext.BindGraphicsPipeline(m_IrradiancePipeline)
                .BindTexture(0, cubemap)
                .SetUniformMat4("VP", kCubeProjection * kCaptureViews[face])
                .DrawCube()
                .EndRendering(framebuffer);
        }

        return irradiance;
    }

    Texture IBLDataGenerator::PrefilterEnvMap(const Texture& cubemap)
    {
        assert(cubemap.GetType() == GL_TEXTURE_CUBE_MAP);

        constexpr auto kSize             = 512u;
        auto           prefilteredEnvMap = m_RenderContext.CreateCubemap(kSize, PixelFormat::RGB16F, 0);
        const auto     numMipLevels      = prefilteredEnvMap.GetNumMipLevels();
        m_RenderContext.SetupSampler(prefilteredEnvMap,
                                     {
                                         .MinFilter    = TexelFilter::Linear,
                                         .MipmapMode   = MipmapMode::Linear,
                                         .MagFilter    = TexelFilter::Linear,
                                         .AddressModeS = SamplerAddressMode::ClampToEdge,
                                         .AddressModeT = SamplerAddressMode::ClampToEdge,
                                         .AddressModeR = SamplerAddressMode::ClampToEdge,
                                     });

        for (uint8_t level {0}; level < numMipLevels; ++level)
        {
            const auto mipSize = calcMipSize(glm::uvec3 {kSize, kSize, 1u}, level).x;
            for (uint8_t face {0}; face < 6; ++face)
            {
                const RenderingInfo renderingInfo {
                    .Area             = {.Extent = {mipSize, mipSize}},
                    .ColorAttachments = {AttachmentInfo {
                        .Image      = prefilteredEnvMap,
                        .MipLevel   = level,
                        .Face       = face,
                        .ClearValue = glm::vec4 {0.0f},
                    }},
                };
                const float roughness = static_cast<float>(level) / (numMipLevels - 1u);

                const auto framebuffer = m_RenderContext.BeginRendering(renderingInfo);
                m_RenderContext.BindGraphicsPipeline(m_PrefilterEnvMapPipeline)
                    .BindTexture(0, cubemap)
                    .SetUniform1f("roughness", roughness)
                    .SetUniformMat4("VP", kCubeProjection * kCaptureViews[face])
                    .DrawCube()
                    .EndRendering(framebuffer);
            }
        }

        return prefilteredEnvMap;
    }
} // namespace SnowLeopardEngine