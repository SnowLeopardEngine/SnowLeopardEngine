#include "SnowLeopardEngine/Function/Rendering/Renderer/SceneRenderer.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/IO/TextureLoader.h"
#include "SnowLeopardEngine/Function/Rendering/Material.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "SnowLeopardEngine/Core/Math/AABB.h"

#include <algorithm>
#include <bit>
#include <limits>
#include <map>
#include <cmath>
#include <cstring>
#include <set>
#include <sstream>

// <windows.h> (through a dependency) maps FindResource to FindResourceA.
#ifdef FindResource
#undef FindResource
#endif

namespace SnowLeopardEngine
{
    namespace
    {
        static_assert(sizeof(SceneRenderer::GPUFrame) == 1440, "GPUFrame must match FrameData in Scene.slangh");
        static_assert(sizeof(SceneRenderer::GPUInstance) == 176, "GPUInstance must match InstanceData");
        static_assert(sizeof(SceneRenderer::GPUMaterial) == 96, "GPUMaterial must match MaterialData");

        // FNV-1a over a mesh's vertices and indices: identical meshes share an arena range.
        uint64_t MeshHash(const MeshData& data)
        {
            uint64_t hash = 1469598103934665603ull;
            auto     mix  = [&](const void* bytes, size_t size) {
                const auto* p = static_cast<const uint8_t*>(bytes);
                for (size_t i = 0; i < size; ++i)
                {
                    hash = (hash ^ p[i]) * 1099511628211ull;
                }
            };
            for (const auto& vertex : data.Vertices)
            {
                mix(&vertex.Position, sizeof(vertex.Position) + sizeof(vertex.Normal) + sizeof(vertex.TexCoord));
            }
            mix(data.Indices.data(), data.Indices.size() * sizeof(uint32_t));
            return hash ^ (uint64_t(data.Vertices.size()) << 32) ^ data.Indices.size();
        }

        constexpr uint32_t  kCommandStride = sizeof(VriDrawIndexedDesc); // 20 bytes, matches DrawCommand
        constexpr VriFormat kHDRFormat     = VriFormat_RGBA16_SFLOAT;
        constexpr uint32_t  kSpecularMips  = 8;

        struct CullParameters
        {
            glm::vec4  Planes[6];
            glm::uvec4 Range;   // first, count, view, capacity
            glm::uvec4 Options; // compact, shadow casters only
        };
        static_assert(sizeof(CullParameters) == 128);

        // The engine's cameras use OpenGL clip depth [-1, 1]; VRI's is [0, 1].
        glm::mat4 DepthZeroToOne(const glm::mat4& projection)
        {
            glm::mat4 remap(1.0f);
            remap[2][2] = 0.5f;
            remap[3][2] = 0.5f;
            return remap * projection;
        }

        // Frustum planes (normals inside, normalized) of a [0, 1]-depth view-projection (Gribb / Hartmann).
        std::array<glm::vec4, 6> FrustumPlanes(const glm::mat4& m)
        {
            const auto row = [&](int i) { return glm::vec4(m[0][i], m[1][i], m[2][i], m[3][i]); };
            std::array<glm::vec4, 6> planes {row(3) + row(0), row(3) - row(0), row(3) + row(1),
                                             row(3) - row(1), row(2),          row(3) - row(2)};
            for (auto& plane : planes)
            {
                plane /= std::max(glm::length(glm::vec3(plane)), 1e-6f);
            }
            return planes;
        }

        // Cascaded shadow maps over [near, shadowFar] of the camera (ported from libvultra's shadow_cascades.cpp):
        // bounding spheres of the frustum slices (stable under rotation), snapped to shadow texels (stable under
        // translation), light depth extended to cover every shadow caster.
        struct Cascades
        {
            glm::mat4 ViewProjection[SceneRenderer::kCascades];
            glm::vec4 Splits {0};
            glm::vec4 Texel {0};
        };

        Cascades CalculateCascades(const glm::mat4& view,
                                   float            fovY,
                                   float            aspect,
                                   float            nearPlane,
                                   float            farPlane,
                                   glm::vec3        lightDirection, // direction the light travels
                                   glm::vec4        casters,        // world sphere of all shadow casters
                                   uint32_t         resolution,
                                   float            lambda)
        {
            Cascades   result;
            const auto inverseView = glm::inverse(view);
            const auto toLight     = glm::normalize(-lightDirection);
            const auto up          = std::abs(toLight.y) < 0.99f ? glm::vec3(0, 1, 0) : glm::vec3(0, 0, 1);
            const auto lightView   = glm::lookAtRH(glm::vec3(0), -toLight, up);
            const auto caster      = glm::vec3(lightView * glm::vec4(glm::vec3(casters), 1));
            const float tanY       = std::tan(fovY * 0.5f);
            const float tanX       = tanY * aspect;

            float previous = nearPlane;
            for (uint32_t cascade = 0; cascade < SceneRenderer::kCascades; ++cascade)
            {
                const float fraction    = float(cascade + 1) / SceneRenderer::kCascades;
                const float logarithmic = nearPlane * std::pow(farPlane / nearPlane, fraction);
                const float uniform     = nearPlane + (farPlane - nearPlane) * fraction;
                const float split       = glm::mix(uniform, logarithmic, lambda);
                result.Splits[cascade]  = split;

                // Slice corners in view space, then world space.
                std::array<glm::vec3, 8> corners;
                glm::vec3                center(0);
                const float              depths[2] {previous, split};
                for (uint32_t i = 0; i < 8; ++i)
                {
                    const float depth = depths[i / 4];
                    const glm::vec3 corner((i & 1 ? 1 : -1) * tanX * depth, (i & 2 ? 1 : -1) * tanY * depth, -depth);
                    corners[i] = glm::vec3(inverseView * glm::vec4(corner, 1));
                    center += corners[i];
                }
                center /= 8.0f;
                float radius = 0;
                for (const auto& corner : corners)
                {
                    radius = std::max(radius, glm::length(corner - center));
                }
                radius = std::ceil(radius * 16) / 16;
                radius *= float(resolution) / float(resolution - 2); // guard band for the texel snapping

                auto        lightCenter = glm::vec3(lightView * glm::vec4(center, 1));
                const float texel       = 2 * radius / float(resolution);
                lightCenter.x           = std::floor(lightCenter.x / texel) * texel;
                lightCenter.y           = std::floor(lightCenter.y / texel) * texel;
                const float zMin = std::min(lightCenter.z - radius, caster.z - casters.w) - 1;
                const float zMax = std::max(lightCenter.z + radius, caster.z + casters.w) + 1;
                const auto  projection = glm::orthoRH_ZO(lightCenter.x - radius,
                                                        lightCenter.x + radius,
                                                        lightCenter.y - radius,
                                                        lightCenter.y + radius,
                                                        -zMax,
                                                        -zMin);
                result.ViewProjection[cascade] = projection * lightView;
                result.Texel[cascade]          = texel;
                previous                       = split;
            }
            return result;
        }

        glm::vec4 ParseVector(const std::string& text)
        {
            glm::vec4   value(0, 0, 0, 1);
            std::string cleaned;
            for (char c : text)
            {
                cleaned += (c == '(' || c == ')' || c == ',') ? ' ' : c;
            }
            std::istringstream stream(cleaned);
            for (int i = 0; i < 4 && (stream >> value[i]); ++i)
            {
            }
            return value;
        }

        float ParseFloat(const std::string& text)
        {
            try
            {
                return std::stof(text);
            }
            catch (...)
            {
                return 0;
            }
        }

        // .dzmaterial property types -> DzShader property types.
        bool TypeMatches(const std::string& dzType, ShaderPropertyType type)
        {
            if (dzType == "Color")
                return type == ShaderPropertyType::Color;
            if (dzType == "Vector")
                return type == ShaderPropertyType::Vector || type == ShaderPropertyType::Color;
            if (dzType == "Float")
                return type == ShaderPropertyType::Float || type == ShaderPropertyType::Range;
            if (dzType == "Int")
                return type == ShaderPropertyType::Integer || type == ShaderPropertyType::Boolean;
            if (dzType == "Texture2D")
                return type == ShaderPropertyType::Texture2D;
            return false;
        }

        void BeginRendering(RHI::Device&                    device,
                            VriCommandBuffer*               cmd,
                            std::span<VriDescriptor* const> colors,
                            const float*                    clearColor,
                            VriDescriptor*                  depth,
                            bool                            clearDepth,
                            RHI::Extent                     extent)
        {
            std::vector<VriAttachmentDesc> attachments(colors.size());
            for (size_t i = 0; i < colors.size(); ++i)
            {
                attachments[i].view    = colors[i];
                attachments[i].loadOp  = clearColor != nullptr ? VriAttachmentLoadOp_Clear : VriAttachmentLoadOp_Load;
                attachments[i].storeOp = VriAttachmentStoreOp_Store;
                if (clearColor != nullptr)
                {
                    std::copy_n(clearColor, 4, attachments[i].clearValue.color.f32);
                }
            }
            VriAttachmentDesc depthAttachment {};
            depthAttachment.view                          = depth;
            depthAttachment.loadOp                        = clearDepth ? VriAttachmentLoadOp_Clear : VriAttachmentLoadOp_Load;
            depthAttachment.storeOp                       = VriAttachmentStoreOp_Store;
            depthAttachment.clearValue.depthStencil.depth = 1.0f;

            VriAttachmentsDesc desc {};
            desc.colors     = attachments.data();
            desc.colorNum   = uint32_t(attachments.size());
            desc.depth      = depth != nullptr ? &depthAttachment : nullptr;
            desc.renderArea = {0, 0, extent.Width, extent.Height};
            desc.layerNum   = 1;
            device.Core.CmdBeginRendering(cmd, &desc);
            VriViewport viewport {0, 0, float(extent.Width), float(extent.Height), 0, 1};
            VriRect     scissor {0, 0, extent.Width, extent.Height};
            device.Core.CmdSetViewports(cmd, &viewport, 1);
            device.Core.CmdSetScissors(cmd, &scissor, 1);
        }

        VriDescriptor* CreateSampler(RHI::Device& device, const VriSamplerDesc& desc)
        {
            VriDescriptor* sampler = nullptr;
            RHI::Check(device.Core.CreateSampler(device.Handle, &desc, &sampler), "Create sampler");
            return sampler;
        }

        // Mesh vertex stream (MeshVertexData) + the identity instance stream.
        const VriVertexAttributeDesc kAttributes[] {{VriFormat_RGB32_SFLOAT, 0, 0, "POSITION", 0},
                                                    {VriFormat_RGB32_SFLOAT, 12, 0, "NORMAL", 0},
                                                    {VriFormat_RG32_SFLOAT, 24, 0, "TEXCOORD", 0},
                                                    {VriFormat_R32_UINT, 0, 1, "INSTANCE", 0}};
        const VriVertexStreamDesc    kStreams[] {{uint32_t(sizeof(MeshVertexData)), 0, VriVertexStepRate_PerVertex},
                                                 {4, 1, VriVertexStepRate_PerInstance}};
        VriVertexInputDesc           MeshInput() { return {kAttributes, 4, kStreams, 2}; }
        // Depth-only passes read the position and the instance only.
        const VriVertexAttributeDesc kDepthAttributes[] {{VriFormat_RGB32_SFLOAT, 0, 0, "POSITION", 0},
                                                         {VriFormat_R32_UINT, 0, 1, "INSTANCE", 0}};
        VriVertexInputDesc           DepthInput() { return {kDepthAttributes, 2, kStreams, 2}; }
        // Cut-out depth passes also read the uv.
        const VriVertexAttributeDesc kCutoutAttributes[] {{VriFormat_RGB32_SFLOAT, 0, 0, "POSITION", 0},
                                                          {VriFormat_RG32_SFLOAT, 24, 0, "TEXCOORD", 0},
                                                          {VriFormat_R32_UINT, 0, 1, "INSTANCE", 0}};
        VriVertexInputDesc           CutoutInput() { return {kCutoutAttributes, 3, kStreams, 2}; }
    } // namespace

    // ------------------------------------------------------------------------------------------------ setup

    SceneRenderer::SceneRenderer(RHI::Device& device, SceneRendererSettings settings) :
        m_Device(device), m_Settings(settings)
    {
        m_IndirectCount = device.Core.GetDeviceDesc(device.Handle)->hasDrawIndirectCount;
        CreateSamplersAndDefaults();
        LoadShaders();
        CreatePipelines();

        m_FrameBuffer = CreateScope<RHI::Buffer>(
            device, VriBufferDesc {(sizeof(GPUFrame) + 255) / 256 * 256, 0, VriBufferUsage_ConstantBuffer, VriMemoryLocation_HostUpload});
        m_DrawCounts = CreateScope<RHI::Buffer>(
            device,
            VriBufferDesc {kViews * 4, 0,
                           VriBufferUsage_StorageBuffer | VriBufferUsage_IndirectBuffer | VriBufferUsage_TransferDst |
                               VriBufferUsage_TransferSrc,
                           VriMemoryLocation_Device});
        EnsureInstanceCapacity(256);
        GrowArena(1 << 16, 1 << 17);
    }

    SceneRenderer::~SceneRenderer()
    {
        m_Device.WaitIdle();
        for (auto* sampler : {m_MaterialSampler, m_LinearClamp, m_EnvironmentSampler, m_ShadowSampler})
        {
            if (sampler != nullptr)
            {
                m_Device.Core.DestroyDescriptor(sampler);
            }
        }
    }

    void SceneRenderer::CreateSamplersAndDefaults()
    {
        VriSamplerDesc material {};
        material.magFilter        = VriFilter_Linear;
        material.minFilter        = VriFilter_Linear;
        material.mipmapMode       = VriMipmapMode_Linear;
        material.addressModeU     = VriAddressMode_Repeat;
        material.addressModeV     = VriAddressMode_Repeat;
        material.addressModeW     = VriAddressMode_Repeat;
        material.anisotropyEnable = VRI_TRUE;
        material.maxAnisotropy    = 8;
        material.maxLod           = 1000;
        m_MaterialSampler         = CreateSampler(m_Device, material);

        VriSamplerDesc clamp = material;
        clamp.anisotropyEnable = VRI_FALSE;
        clamp.addressModeU = clamp.addressModeV = clamp.addressModeW = VriAddressMode_ClampToEdge;
        m_LinearClamp = CreateSampler(m_Device, clamp);

        VriSamplerDesc environment = clamp;
        environment.addressModeU   = VriAddressMode_Repeat; // longitude wraps
        m_EnvironmentSampler       = CreateSampler(m_Device, environment);

        VriSamplerDesc shadow = clamp;
        shadow.mipmapMode     = VriMipmapMode_Nearest;
        shadow.maxLod         = 0;
        shadow.compareEnable  = VRI_TRUE;
        shadow.compareOp      = VriCompareOp_LessOrEqual;
        m_ShadowSampler       = CreateSampler(m_Device, shadow);

        auto makeTexel = [&](std::array<uint8_t, 4> rgba) {
            auto desc    = RHI::ColorTextureDesc({1, 1}, VriFormat_RGBA8_UNORM);
            desc.usage   = VriTextureUsage_ShaderResource | VriTextureUsage_TransferDst;
            auto texture = CreateScope<RHI::Texture>(m_Device, desc);
            RHI::UploadTexture2D(m_Device, *texture, std::as_bytes(std::span(rgba)));
            return texture;
        };
        m_White = makeTexel({255, 255, 255, 255});
        m_Black = makeTexel({0, 0, 0, 255});
        m_Textures.push_back(m_White.get());
        m_TextureIndices[m_White.get()] = 0;
    }

    void SceneRenderer::LoadShaders()
    {
        ShaderCompileOptions options;
        options.IncludeDirectories = {"Assets/Shaders"};
        m_Standard          = ShaderAsset::Load("Assets/Shaders/Standard/Standard.dzshader", options);
        m_Culling           = ShaderAsset::Load("Assets/Shaders/Renderer/Culling.dzshader", options);
        m_Lighting          = ShaderAsset::Load("Assets/Shaders/Renderer/DeferredLighting.dzshader", options);
        m_ToneMapping       = ShaderAsset::Load("Assets/Shaders/Renderer/ToneMapping.dzshader", options);
        m_EnvironmentShader = ShaderAsset::Load("Assets/Shaders/Renderer/Environment.dzshader", options);
        m_UI                = CreateScope<UIRenderer>(m_Device, options);
        m_PostShader        = ShaderAsset::Load("Assets/Shaders/Renderer/PostProcess.dzshader", options);
        m_EditorShader      = ShaderAsset::Load("Assets/Shaders/Renderer/EditorOverlay.dzshader", options);
        auto post = [&](const char* name, VriFormat format) {
            ShaderPipelineDesc desc;
            desc.ColorFormats = {format};
            return CreateScope<ShaderPipeline>(m_Device, m_PostShader, *m_PostShader.Subshaders.at(0).FindPass(name), desc);
        };
        m_GTAOPipeline      = post("GTAO", VriFormat_R16_SFLOAT);
        m_AODenoisePipeline = post("AODenoise", VriFormat_R16_SFLOAT);
        m_BloomDownPipeline = post("BloomDown", VriFormat_RGBA16_SFLOAT);
        m_BloomUpPipeline   = post("BloomUp", VriFormat_RGBA16_SFLOAT);
    }

    void SceneRenderer::SetSettings(const SceneRendererSettings& settings)
    {
        auto next        = settings;
        next.BloomLevels = std::clamp(next.BloomLevels, 1u, 10u);
        // Only switches that add, remove or resize passes rebuild the graph; values are read when passes execute.
        const bool shape = next.AmbientOcclusion != m_Settings.AmbientOcclusion || next.Bloom != m_Settings.Bloom ||
                           next.BloomLevels != m_Settings.BloomLevels || next.FXAA != m_Settings.FXAA ||
                           next.EditorOverlay != m_Settings.EditorOverlay || next.ShadowResolution != m_Settings.ShadowResolution;
        m_Settings = next;
        if (shape)
        {
            ++m_GraphVersion;
        }
    }

    void SceneRenderer::CreatePipelines()
    {
        const auto& standard = m_Standard.Subshaders.at(0);

        ShaderPipelineDesc gbuffer;
        gbuffer.ColorFormats = {VriFormat_RGBA8_SRGB, VriFormat_RGBA16_SFLOAT, VriFormat_RGBA16_SFLOAT};
        gbuffer.DepthFormat  = VriFormat_D32_SFLOAT;
        gbuffer.VertexInput  = MeshInput();
        m_GBufferPipeline    = CreateScope<ShaderPipeline>(m_Device, m_Standard, *standard.FindPass("GBuffer"), gbuffer);
        m_GBufferAlphaPipeline = CreateScope<ShaderPipeline>(m_Device, m_Standard, *standard.FindPass("GBufferAlphaTest"), gbuffer);

        ShaderPipelineDesc forward;
        forward.ColorFormats = {kHDRFormat};
        forward.DepthFormat  = VriFormat_D32_SFLOAT;
        forward.VertexInput  = MeshInput();
        m_ForwardPipeline    = CreateScope<ShaderPipeline>(m_Device, m_Standard, *standard.FindPass("Forward"), forward);

        ShaderPipelineDesc shadow;
        shadow.ColorFormats = {};
        shadow.DepthFormat  = VriFormat_D32_SFLOAT;
        shadow.VertexInput  = DepthInput();
        m_ShadowPipeline    = CreateScope<ShaderPipeline>(m_Device, m_Standard, *standard.FindPass("ShadowCaster"), shadow);
        shadow.VertexInput  = CutoutInput();
        m_ShadowAlphaPipeline = CreateScope<ShaderPipeline>(m_Device, m_Standard, *standard.FindPass("ShadowCasterAlphaTest"), shadow);

        ShaderPipelineDesc entityId;
        entityId.ColorFormats  = {VriFormat_R32_UINT};
        entityId.DepthFormat   = VriFormat_D32_SFLOAT;
        entityId.VertexInput   = DepthInput();
        m_EntityIdPipeline     = CreateScope<ShaderPipeline>(m_Device, m_Standard, *standard.FindPass("EntityID"), entityId);
        entityId.VertexInput   = CutoutInput();
        m_EntityIdAlphaPipeline = CreateScope<ShaderPipeline>(m_Device, m_Standard, *standard.FindPass("EntityIDAlphaTest"), entityId);
        m_EntityIdSet          = CreateScope<ShaderBindings>(m_Device, *m_EntityIdPipeline);
        m_EntityIdAlphaSet     = CreateScope<ShaderBindings>(m_Device, *m_EntityIdAlphaPipeline);
        m_PickReadback         = CreateScope<RHI::Buffer>(
            m_Device, VriBufferDesc {256, 0, VriBufferUsage_TransferDst, VriMemoryLocation_HostReadback});

        m_CullPipeline = CreateScope<ShaderPipeline>(m_Device, m_Culling, m_Culling.Subshaders.at(0).Passes.at(0), ShaderPipelineDesc {});

        ShaderPipelineDesc lighting;
        lighting.ColorFormats = {kHDRFormat};
        m_LightingPipeline = CreateScope<ShaderPipeline>(m_Device, m_Lighting, m_Lighting.Subshaders.at(0).Passes.at(0), lighting);

        for (const auto* pipeline : {m_GBufferPipeline.get(), m_GBufferAlphaPipeline.get(), m_ForwardPipeline.get(),
                                     m_ShadowPipeline.get(), m_ShadowAlphaPipeline.get(),
                                     m_CullPipeline.get(), m_LightingPipeline.get()})
        {
            // Renderer resources live in set 0; a generated `material` block must not take it.
            if (pipeline->MaterialSet() && *pipeline->MaterialSet() == 0)
            {
                throw std::logic_error("DzShader material block was assigned descriptor set 0");
            }
        }
        m_GBufferSet  = CreateScope<ShaderBindings>(m_Device, *m_GBufferPipeline);
        m_ForwardSet  = CreateScope<ShaderBindings>(m_Device, *m_ForwardPipeline);
        m_ShadowSet   = CreateScope<ShaderBindings>(m_Device, *m_ShadowPipeline);
        m_GBufferAlphaSet = CreateScope<ShaderBindings>(m_Device, *m_GBufferAlphaPipeline);
        m_ShadowAlphaSet  = CreateScope<ShaderBindings>(m_Device, *m_ShadowAlphaPipeline);
        m_CullSet     = CreateScope<ShaderBindings>(m_Device, *m_CullPipeline);
        m_LightingSet = CreateScope<ShaderBindings>(m_Device, *m_LightingPipeline);

        // Split-sum BRDF table: independent of the environment, made once.
        const auto& passes = m_EnvironmentShader.Subshaders.at(0);
        ShaderPipelineDesc table;
        table.ColorFormats = {VriFormat_RGBA16_SFLOAT};
        ShaderPipeline brdf(m_Device, m_EnvironmentShader, *passes.FindPass("BRDF"), table);
        m_BrdfLut = CreateScope<RHI::Texture>(m_Device, RHI::ColorTextureDesc({128, 128}, VriFormat_RGBA16_SFLOAT));
        RHI::Frame frame(m_Device);
        auto*      cmd = frame.Begin();
        m_BrdfLut->Transition(cmd, {VriAccess_ColorAttachmentWrite, VriLayout_ColorAttachment, VriPipelineStage_ColorAttachmentOutput});
        RHI::BeginColorPass(m_Device, cmd, m_BrdfLut->View(), m_BrdfLut->GetExtent());
        brdf.Bind(cmd);
        const struct
        {
            float    Roughness;
            uint32_t Samples;
            float    Padding[2];
        } parameters {0, 512, {0, 0}};
        brdf.SetConstants(cmd, &parameters, sizeof(parameters));
        const VriDrawDesc draw {3, 1, 0, 0};
        m_Device.Core.CmdDraw(cmd, &draw);
        m_Device.Core.CmdEndRendering(cmd);
        m_BrdfLut->Transition(cmd, {VriAccess_ShaderResourceRead, VriLayout_ShaderResource, VriPipelineStage_FragmentShader});
        frame.SubmitAndWait();

        // Until a scene brings an environment: black irradiance / specular (no IBL; frame.environment.y = 0).
        m_Irradiance.reset();
        m_Specular.reset();
    }

    void SceneRenderer::SetEnvironment(const std::filesystem::path& path)
    {
        if (path == m_EnvironmentPath)
        {
            return;
        }
        m_Device.WaitIdle();
        m_EnvironmentPath = path;
        m_Radiance        = path.empty() ? nullptr : IO::LoadHDR(path);
        m_Irradiance.reset();
        m_Specular.reset();
        if (m_Radiance == nullptr)
        {
            return;
        }

        const auto&        passes = m_EnvironmentShader.Subshaders.at(0);
        ShaderPipelineDesc desc;
        desc.ColorFormats = {VriFormat_RGBA16_SFLOAT};
        ShaderPipeline irradiance(m_Device, m_EnvironmentShader, *passes.FindPass("Irradiance"), desc);
        ShaderPipeline specular(m_Device, m_EnvironmentShader, *passes.FindPass("Specular"), desc);
        ShaderBindings irradianceSet(m_Device, irradiance), specularSet(m_Device, specular);
        for (auto* set : {&irradianceSet, &specularSet})
        {
            set->Write("radianceMap", m_Radiance->View());
            set->Write("environmentSampler", m_EnvironmentSampler);
        }

        m_Irradiance = CreateScope<RHI::Texture>(m_Device, RHI::ColorTextureDesc({64, 32}, VriFormat_RGBA16_SFLOAT));
        auto specularDesc   = RHI::ColorTextureDesc({256, 128}, VriFormat_RGBA16_SFLOAT);
        specularDesc.mipNum = kSpecularMips;
        m_Specular          = CreateScope<RHI::Texture>(m_Device, specularDesc);

        RHI::Frame frame(m_Device);
        auto*      cmd     = frame.Begin();
        auto       generate = [&](RHI::Texture& target, const ShaderPipeline& pipeline, const ShaderBindings& set, uint32_t samples) {
            target.Transition(cmd, {VriAccess_ColorAttachmentWrite, VriLayout_ColorAttachment, VriPipelineStage_ColorAttachmentOutput});
            for (uint32_t mip = 0; mip < target.Desc.mipNum; ++mip)
            {
                const RHI::Extent extent {std::max(target.Desc.width >> mip, 1u), std::max(target.Desc.height >> mip, 1u)};
                RHI::BeginColorPass(m_Device, cmd, target.Desc.mipNum > 1 ? target.MipView(mip) : target.View(), extent);
                pipeline.Bind(cmd);
                set.Bind(cmd);
                const struct
                {
                    float    Roughness;
                    uint32_t Samples;
                    float    Padding[2];
                } parameters {float(mip) / float(std::max(target.Desc.mipNum - 1, 1u)), samples, {0, 0}};
                pipeline.SetConstants(cmd, &parameters, sizeof(parameters));
                const VriDrawDesc draw {3, 1, 0, 0};
                m_Device.Core.CmdDraw(cmd, &draw);
                m_Device.Core.CmdEndRendering(cmd);
            }
            target.Transition(cmd, {VriAccess_ShaderResourceRead, VriLayout_ShaderResource, VriPipelineStage_FragmentShader});
        };
        generate(*m_Irradiance, irradiance, irradianceSet, 1024);
        generate(*m_Specular, specular, specularSet, 512);
        frame.SubmitAndWait();
        SNOW_LEOPARD_CORE_INFO("[SceneRenderer] Environment {0} ({1} x {2})",
                               path.generic_string(),
                               m_Radiance->Desc.width,
                               m_Radiance->Desc.height);
    }

    void SceneRenderer::OnSceneChanged()
    {
        m_Device.WaitIdle();
        m_Meshes.clear();
        m_SharedMeshes.clear();
        m_PendingVertices.clear();
        m_PendingIndices.clear();
        m_VertexCount = m_IndexCount = 0;
        m_UploadedVertices = m_UploadedIndices = 0;
        m_MaterialIndices.clear();
        m_Materials.clear();
        m_Textures.resize(1);
        m_TextureIndices.clear();
        m_TextureIndices[m_White.get()] = 0;
    }

    // ------------------------------------------------------------------------------------------------ resources

    void SceneRenderer::GrowArena(uint64_t vertices, uint64_t indices)
    {
        auto grow = [&](Scope<RHI::Buffer>& buffer, uint64_t& capacity, uint64_t needed, uint64_t stride, VriBufferUsageFlags usage) {
            if (needed <= capacity)
            {
                return;
            }
            uint64_t next = std::max<uint64_t>(capacity * 2, 1024);
            while (next < needed)
            {
                next *= 2;
            }
            auto replacement = CreateScope<RHI::Buffer>(
                m_Device,
                VriBufferDesc {next * stride, 0, usage | VriBufferUsage_TransferDst | VriBufferUsage_TransferSrc, VriMemoryLocation_Device});
            RHI::Frame frame(m_Device);
            auto*      cmd = frame.Begin();
            if (buffer && m_UploadedVertices + m_UploadedIndices > 0)
            {
                buffer->Transition(cmd, {VriAccess_CopySourceRead, VriPipelineStage_Transfer});
                replacement->Transition(cmd, {VriAccess_CopyDestinationWrite, VriPipelineStage_Transfer});
                VriBufferCopyDesc copy {};
                copy.size = buffer->Desc.size;
                m_Device.Core.CmdCopyBuffer(cmd, replacement->Handle, buffer->Handle, &copy);
            }
            replacement->Transition(cmd, {VriAccess_VertexBufferRead | VriAccess_IndexBufferRead, VriPipelineStage_AllCommands});
            frame.SubmitAndWait();
            buffer   = std::move(replacement);
            capacity = next;
        };
        grow(m_Vertices, m_VertexCapacity, vertices, sizeof(MeshVertexData), VriBufferUsage_VertexBuffer);
        grow(m_Indices, m_IndexCapacity, indices, sizeof(uint32_t), VriBufferUsage_IndexBuffer);
    }

    const SceneRenderer::MeshRecord* SceneRenderer::UploadMesh(const MeshItem& mesh)
    {
        const auto& data = mesh.Data;
        if (data.Vertices.empty() || data.Indices.empty())
        {
            return nullptr;
        }
        if (auto found = m_Meshes.find(&mesh); found != m_Meshes.end())
        {
            const auto& record = found->second;
            if (record.Source == data.Vertices.data() && record.VertexCount == data.Vertices.size() &&
                record.IndexCount == data.Indices.size())
            {
                return &record;
            }
            // The address now belongs to another mesh (or it was rebuilt): upload again. The old range stays unused
            // until the next scene change resets the arena.
        }

        // Static meshes with identical content share one range (skinned ones are rewritten per entity).
        uint64_t hash = 0;
        if (!mesh.Skinned())
        {
            hash = MeshHash(data);
            if (auto shared = m_SharedMeshes.find(hash); shared != m_SharedMeshes.end())
            {
                auto record   = shared->second;
                record.Source = data.Vertices.data();
                return &(m_Meshes[&mesh] = record);
            }
        }

        // Appended to this frame's pending block; FlushMeshUploads copies all of them in one transfer.
        MeshRecord record;
        record.FirstIndex   = uint32_t(m_IndexCount);
        record.IndexCount   = uint32_t(data.Indices.size());
        record.VertexOffset = uint32_t(m_VertexCount);
        record.VertexCount  = uint32_t(data.Vertices.size());
        record.Source       = data.Vertices.data();
        const auto box      = AABB::Build(data.Vertices); // once per mesh, not per frame
        record.LocalSphere  = glm::vec4(box.GetCenter(), glm::length(box.GetExtent()) * 0.5f);
        m_PendingVertices.insert(m_PendingVertices.end(), data.Vertices.begin(), data.Vertices.end());
        m_PendingIndices.insert(m_PendingIndices.end(), data.Indices.begin(), data.Indices.end());
        m_VertexCount += data.Vertices.size();
        m_IndexCount += data.Indices.size();
        if (!mesh.Skinned())
        {
            m_SharedMeshes[hash] = record;
        }
        return &(m_Meshes[&mesh] = record);
    }

    void SceneRenderer::FlushMeshUploads()
    {
        if (m_PendingVertices.empty())
        {
            return;
        }
        GrowArena(m_VertexCount, m_IndexCount);
        RHI::UploadBufferRegion(m_Device, *m_Vertices, m_UploadedVertices * sizeof(MeshVertexData), std::as_bytes(std::span(m_PendingVertices)));
        RHI::UploadBufferRegion(m_Device, *m_Indices, m_UploadedIndices * sizeof(uint32_t), std::as_bytes(std::span(m_PendingIndices)));
        m_UploadedVertices = m_VertexCount;
        m_UploadedIndices  = m_IndexCount;
        m_PendingVertices.clear();
        m_PendingIndices.clear();
    }

    uint32_t SceneRenderer::TextureIndex(RHI::Texture* texture)
    {
        if (texture == nullptr)
        {
            return 0;
        }
        if (auto found = m_TextureIndices.find(texture); found != m_TextureIndices.end())
        {
            return found->second;
        }
        if (m_Textures.size() >= kMaxTextures)
        {
            SNOW_LEOPARD_CORE_WARN("[SceneRenderer] Bindless texture table is full ({0})", kMaxTextures);
            return 0;
        }
        const auto index           = uint32_t(m_Textures.size());
        m_TextureIndices[texture]  = index;
        m_Textures.push_back(texture);
        return index;
    }

    uint32_t SceneRenderer::MaterialIndex(const Material& material)
    {
        if (auto found = m_MaterialIndices.find(&material); found != m_MaterialIndices.end())
        {
            return found->second;
        }
        const auto& define = material.GetDefine();

        // Start from the Standard shader's defaults, then apply the .dzmaterial values it declares.
        std::map<std::string, const DzMaterialProperty*, std::less<>> values;
        for (const auto& property : define.Properties)
        {
            const auto shaderProperty = std::find_if(m_Standard.Properties.begin(), m_Standard.Properties.end(), [&](const auto& p) {
                return p.Name == property.Name;
            });
            if (shaderProperty == m_Standard.Properties.end() || !TypeMatches(property.Type, shaderProperty->Type))
            {
                SNOW_LEOPARD_CORE_WARN("[SceneRenderer] Material {0}: {1} ({2}) is not a Standard property; ignored",
                                       define.Name,
                                       property.Name,
                                       property.Type);
                continue;
            }
            values[property.Name] = &property;
        }
        auto vector = [&](const char* name, glm::vec4 fallback) {
            auto found = values.find(name);
            return found != values.end() ? ParseVector(found->second->Value) : fallback;
        };
        auto scalar = [&](const char* name, float fallback) {
            auto found = values.find(name);
            return found != values.end() ? ParseFloat(found->second->Value) : fallback;
        };
        auto map = [&](const char* name, const char* use) -> uint32_t {
            auto found = values.find(name);
            if (scalar(use, 0) < 0.5f || found == values.end() || found->second->Value.empty())
            {
                return 0;
            }
            return TextureIndex(IO::Load(found->second->Value, define.TextureFlipY));
        };
        auto always = [&](const char* name) -> uint32_t { // maps without a use flag
            auto found = values.find(name);
            return found == values.end() || found->second->Value.empty() ? 0 : TextureIndex(IO::Load(found->second->Value, define.TextureFlipY));
        };

        GPUMaterial gpu {};
        gpu.Albedo   = glm::vec4(glm::vec3(vector("albedo", glm::vec4(1))), scalar("opacity", 1));
        gpu.Emissive = vector("emissive", glm::vec4(0, 0, 0, 1));
        gpu.Surface  = {scalar("metallic", 0), scalar("roughness", 0.5f), scalar("ao", 1), float(std::max(define.TileSize, 1))};
        gpu.Maps     = {map("albedoMap", "useAlbedoMap"),
                        map("metallicMap", "useMetallicMap"),
                        map("roughnessMap", "useRoughnessMap"),
                        map("aoMap", "useAoMap")};
        gpu.Maps2    = {map("emissiveMap", "useEmissiveMap"), always("noiseMap"), 0, 0};
        const auto wind = vector("windDirection", glm::vec4(1, 0, 1, 0));
        gpu.Alpha       = {scalar("alphaCutoff", 0.5f), scalar("windStrength", 0), wind.x, wind.z};

        const auto index              = uint32_t(m_Materials.size());
        m_MaterialIndices[&material] = index;
        m_Materials.push_back(gpu);
        return index;
    }

    void SceneRenderer::EnsureInstanceCapacity(uint32_t count)
    {
        if (count <= m_InstanceCapacity && m_InstanceBuffer)
        {
            return;
        }
        m_Device.WaitIdle();
        uint32_t capacity = std::max(m_InstanceCapacity, 256u);
        while (capacity < count)
        {
            capacity *= 2;
        }
        m_InstanceCapacity = capacity;
        m_InstanceBuffer   = CreateScope<RHI::Buffer>(
            m_Device, VriBufferDesc {capacity * sizeof(GPUInstance), 0, VriBufferUsage_StorageBuffer, VriMemoryLocation_HostUpload});
        m_DrawCommands = CreateScope<RHI::Buffer>(
            m_Device,
            VriBufferDesc {uint64_t(capacity) * kViews * kCommandStride, 0, VriBufferUsage_StorageBuffer | VriBufferUsage_IndirectBuffer,
                           VriMemoryLocation_Device});
        std::vector<uint32_t> identity(capacity);
        for (uint32_t i = 0; i < capacity; ++i)
        {
            identity[i] = i;
        }
        m_InstanceStream = RHI::UploadBuffer(m_Device, std::as_bytes(std::span(identity)), VriBufferUsage_VertexBuffer);
        ++m_GraphVersion; // the indirect command buffer is imported into the frame graph
    }

    // ------------------------------------------------------------------------------------------------ frame

    bool SceneRenderer::Prepare(LogicScene* scene, RHI::Extent extent, const CameraOverride* cameraOverride)
    {
        ++m_FrameIndex;
        m_OpaqueCount = m_AlphaCount = m_TransparentCount = 0;
        m_DynamicCopies.clear();
        m_Extent = extent;
        if (scene == nullptr || extent.Empty())
        {
            return false;
        }
        auto& registry = scene->GetRegistry();

        // ---- camera: the override (editor), else the scene's first one
        TransformComponent* cameraTransformPtr = nullptr;
        CameraComponent*    cameraPtr          = nullptr;
        if (cameraOverride != nullptr && cameraOverride->Transform != nullptr && cameraOverride->Camera != nullptr)
        {
            cameraTransformPtr = cameraOverride->Transform;
            cameraPtr          = cameraOverride->Camera;
        }
        else
        {
            auto cameras = registry.view<TransformComponent, CameraComponent>();
            if (cameras.begin() == cameras.end())
            {
                return false;
            }
            cameraTransformPtr = &cameras.get<TransformComponent>(cameras.front());
            cameraPtr          = &cameras.get<CameraComponent>(cameras.front());
        }
        auto&      cameraTransform = *cameraTransformPtr;
        auto&      camera          = *cameraPtr;
        camera.ViewportWidth       = float(extent.Width); // the aspect follows the render target
        camera.ViewportHeight      = float(extent.Height);
        const auto view            = g_EngineContext->CameraSys->GetViewMatrix(cameraTransform);
        const auto projection      = DepthZeroToOne(g_EngineContext->CameraSys->GetProjectionMatrix(camera));
        SetEnvironment(camera.EnvironmentMapFilePath);

        // ---- renderables: opaque, then alpha-tested, then transparent (sorted far to near)
        enum class Queue
        {
            Opaque,
            AlphaTest,
            Transparent
        };
        struct Item
        {
            const MeshRecord* Mesh;
            glm::mat4         Model;
            uint32_t          Material;
            Queue             Bucket;
            bool              CastShadow;
            float             Distance;
            uint32_t          Id; // entity + 1
        };
        std::vector<Item> items;
        auto add = [&](entt::entity entity, const MeshItem& mesh, const glm::mat4& model, const Material& material, bool castShadow) {
            const auto* record = UploadMesh(mesh);
            if (record == nullptr)
            {
                return;
            }
            if (mesh.Skinned())
            {
                // CPU skinning (Animator) rewrites the vertices every frame: copied into the mesh's arena range by
                // the frame's upload pass.
                m_DynamicCopies.push_back({mesh.Data.Vertices.data(), 0, uint64_t(record->VertexOffset) * sizeof(MeshVertexData),
                                           uint64_t(record->VertexCount) * sizeof(MeshVertexData)});
            }
            const auto& queue  = material.GetDefine().RenderQueue;
            const auto  bucket = queue == RenderQueue_Transparent ? Queue::Transparent :
                                 queue == RenderQueue_AlphaTest   ? Queue::AlphaTest :
                                                                    Queue::Opaque;
            items.push_back({record, model, MaterialIndex(material), bucket, castShadow && bucket != Queue::Transparent,
                             glm::distance(cameraTransform.Position, glm::vec3(model[3])), uint32_t(entt::to_integral(entity)) + 1});
        };
        registry.view<TransformComponent, EntityStatusComponent, MeshFilterComponent, MeshRendererComponent>().each(
            [&](entt::entity entity, TransformComponent&, EntityStatusComponent& status, MeshFilterComponent& filter, MeshRendererComponent& renderer) {
                if (!status.IsEnabled || filter.Meshes == nullptr || renderer.Mat == nullptr)
                {
                    return;
                }
                const auto model = scene->GetWorldTransform(entity);
                for (const auto& mesh : filter.Meshes->Items)
                {
                    add(entity, mesh, model, *renderer.Mat, renderer.CastShadow);
                }
            });
        registry.view<TransformComponent, EntityStatusComponent, TerrainComponent, TerrainRendererComponent>().each(
            [&](entt::entity entity, TransformComponent&, EntityStatusComponent& status, TerrainComponent& terrain, TerrainRendererComponent& renderer) {
                if (status.IsEnabled && renderer.Mat != nullptr)
                {
                    add(entity, terrain.Mesh, scene->GetWorldTransform(entity), *renderer.Mat, renderer.CastShadow);
                }
            });
        FlushMeshUploads(); // every mesh new this frame, in one transfer
        std::stable_sort(items.begin(), items.end(), [](const Item& a, const Item& b) { return a.Bucket < b.Bucket; });
        const auto firstAlpha       = std::find_if(items.begin(), items.end(), [](const Item& item) { return item.Bucket != Queue::Opaque; });
        const auto firstTransparent = std::find_if(items.begin(), items.end(), [](const Item& item) { return item.Bucket == Queue::Transparent; });
        std::sort(firstTransparent, items.end(), [](const Item& a, const Item& b) { return a.Distance > b.Distance; });
        m_OpaqueCount      = uint32_t(firstAlpha - items.begin());
        m_AlphaCount       = uint32_t(firstTransparent - firstAlpha);
        m_TransparentCount = uint32_t(items.end() - firstTransparent);

        // ---- instances
        EnsureInstanceCapacity(uint32_t(items.size()));
        std::vector<GPUInstance> instances(items.size());
        glm::vec3                casterMin(std::numeric_limits<float>::max()), casterMax(-std::numeric_limits<float>::max());
        for (size_t i = 0; i < items.size(); ++i)
        {
            const auto& item  = items[i];
            auto&       gpu   = instances[i];
            gpu.Model         = item.Model;
            gpu.NormalMatrix  = glm::mat4(glm::inverseTranspose(glm::mat3(item.Model)));
            const float scale = std::max({glm::length(glm::vec3(item.Model[0])), glm::length(glm::vec3(item.Model[1])),
                                          glm::length(glm::vec3(item.Model[2]))});
            // Wind bends alpha-tested meshes by up to windStrength: the culling sphere covers the swing.
            const float sway   = item.Bucket == Queue::AlphaTest ? m_Materials[item.Material].Alpha.y : 0.0f;
            gpu.BoundingSphere = glm::vec4(glm::vec3(item.Model * glm::vec4(glm::vec3(item.Mesh->LocalSphere), 1)),
                                           item.Mesh->LocalSphere.w * scale + sway);
            gpu.Draw  = {item.Mesh->IndexCount, item.Mesh->FirstIndex, item.Mesh->VertexOffset, item.Material};
            gpu.Flags = {item.CastShadow ? 1u : 0u, item.Id, 0, 0};
            if (item.CastShadow)
            {
                casterMin = glm::min(casterMin, glm::vec3(gpu.BoundingSphere) - gpu.BoundingSphere.w);
                casterMax = glm::max(casterMax, glm::vec3(gpu.BoundingSphere) + gpu.BoundingSphere.w);
            }
        }
        m_InstanceBuffer->Write(instances.data(), instances.size() * sizeof(GPUInstance));

        // ---- materials
        if (!m_MaterialBuffer || m_MaterialBuffer->Desc.size < std::max<size_t>(m_Materials.size(), 1) * sizeof(GPUMaterial))
        {
            m_Device.WaitIdle();
            const uint64_t capacity = std::max<uint64_t>(64, std::bit_ceil(m_Materials.size()));
            m_MaterialBuffer        = CreateScope<RHI::Buffer>(
                m_Device, VriBufferDesc {capacity * sizeof(GPUMaterial), 0, VriBufferUsage_StorageBuffer, VriMemoryLocation_HostUpload});
        }
        m_MaterialBuffer->Write(m_Materials.data(), m_Materials.size() * sizeof(GPUMaterial));

        // ---- skinned vertices into a staging buffer
        uint64_t dynamicBytes = 0;
        for (const auto& copy : m_DynamicCopies)
        {
            dynamicBytes += copy.Size;
        }
        if (dynamicBytes > 0)
        {
            if (!m_Staging || m_Staging->Desc.size < dynamicBytes)
            {
                m_Device.WaitIdle();
                m_Staging = CreateScope<RHI::Buffer>(
                    m_Device, VriBufferDesc {std::bit_ceil(dynamicBytes), 0, VriBufferUsage_TransferSrc, VriMemoryLocation_HostUpload});
            }
            uint64_t offset = 0;
            for (auto& copy : m_DynamicCopies)
            {
                m_Staging->Write(copy.Data, copy.Size, offset);
                copy.Source = offset;
                offset += copy.Size;
            }
        }

        // ---- frame constants
        GPUFrame& frame             = m_Frame;
        frame                       = {};
        frame.View                  = view;
        frame.Projection            = projection;
        frame.ViewProjection        = projection * view;
        frame.InverseViewProjection = glm::inverse(frame.ViewProjection);
        frame.CameraPosition        = glm::vec4(cameraTransform.Position, Time::ElapsedTime);
        frame.Resolution            = {extent.Width, extent.Height, 1.0f / extent.Width, 1.0f / extent.Height};
        frame.LightColor            = glm::vec4(1, 1, 1, 1);
        frame.Environment           = {float(kSpecularMips - 1), m_Specular ? 1.0f : 0.0f, 0, 0};
        // ClearFlags::Color replaces the sky with the clear colour (sRGB -> linear); IBL still uses the environment.
        frame.SkyColor = glm::vec4(glm::pow(glm::vec3(camera.ClearColor), glm::vec3(2.2f)),
                                   camera.ClearFlags == CameraClearFlags::Color ? 1.0f : 0.0f);

        auto suns = registry.view<TransformComponent, DirectionalLightComponent>();
        const bool hasCasters = casterMin.x <= casterMax.x;
        if (suns.begin() != suns.end())
        {
            const auto& sun      = suns.get<DirectionalLightComponent>(suns.front());
            frame.LightDirection = glm::vec4(glm::normalize(sun.Direction), sun.Intensity);
            frame.LightColor     = glm::vec4(sun.Color, 1);
            if (hasCasters && camera.Projection == CameraProjection::Perspective)
            {
                const glm::vec3 center = (casterMin + casterMax) * 0.5f;
                const auto      cascades =
                    CalculateCascades(view,
                                      glm::radians(camera.FOV),
                                      float(extent.Width) / float(extent.Height),
                                      camera.Near,
                                      std::min(camera.Far, m_Settings.ShadowDistance),
                                      sun.Direction,
                                      glm::vec4(center, glm::length(casterMax - center)),
                                      m_Settings.ShadowResolution,
                                      m_Settings.CascadeLambda);
                std::copy(std::begin(cascades.ViewProjection), std::end(cascades.ViewProjection), frame.LightViewProjection);
                frame.CascadeSplits = cascades.Splits;
                frame.CascadeTexel  = cascades.Texel;
                frame.Shadow        = {0.0005f, 1.5f, float(m_Settings.ShadowResolution), 1.0f};
            }
        }

        uint32_t lights = 0;
        registry.view<TransformComponent, EntityStatusComponent, PointLightComponent>().each(
            [&](entt::entity entity, TransformComponent&, EntityStatusComponent& status, PointLightComponent& light) {
                if (!status.IsEnabled || lights >= kMaxPointLights)
                {
                    return;
                }
                frame.PointLights[lights++] = {glm::vec4(glm::vec3(scene->GetWorldTransform(entity)[3]), 1),
                                               glm::vec4(light.Color, light.Intensity),
                                               glm::vec4(light.Constant, light.Linear, light.Quadratic, 0)};
            });
        frame.Counts = {lights, 0, 0, 0};
        m_FrameBuffer->Write(&frame, sizeof(frame));

        // ---- culling views
        m_ViewPlanes[0] = FrustumPlanes(frame.ViewProjection);
        for (uint32_t cascade = 0; cascade < kCascades; ++cascade)
        {
            m_ViewPlanes[1 + cascade] = FrustumPlanes(frame.LightViewProjection[cascade]);
            // Casters behind the light's near plane still throw shadows into the cascade: no near plane.
            m_ViewPlanes[1 + cascade][4] = glm::vec4(0, 0, 0, 1);
        }
        m_ViewPlanes[kViewTransparent] = m_ViewPlanes[0];
        for (uint32_t view = 0; view <= kCascades; ++view)
        {
            m_ViewPlanes[kViewAlphaTest + view] = m_ViewPlanes[view];
        }

        // ---- in-game UI
        m_UI->Prepare(*scene, extent, [this](RHI::Texture* texture) { return TextureIndex(texture); });

        m_Stats = {uint32_t(items.size()), m_TransparentCount, m_AlphaCount, m_UI->GetQuadCount(), uint32_t(m_SharedMeshes.size()),
                   uint32_t(m_Materials.size()), uint32_t(m_Textures.size()), m_VertexCount, m_IndexCount};
        return true;
    }

    void SceneRenderer::SetSelection(std::span<const entt::entity> entities)
    {
        m_Selection.clear();
        for (const auto entity : entities)
        {
            if (entity != entt::null && m_Selection.size() < kMaxSelection)
            {
                m_Selection.push_back(uint32_t(entt::to_integral(entity)) + 1);
            }
        }
    }

    void SceneRenderer::RequestPick(glm::uvec2 pixel) { m_PickRequest = pixel; }

    std::optional<entt::entity> SceneRenderer::TakePickResult()
    {
        auto result = m_PickResult;
        m_PickResult.reset();
        return result;
    }

    void SceneRenderer::OnFrameComplete()
    {
        if (!m_PickInFlight)
        {
            return;
        }
        m_PickInFlight.reset();
        uint32_t    id     = 0;
        const auto* mapped = m_Device.Core.MapBuffer(m_PickReadback->Handle, 0, sizeof(id));
        if (mapped != nullptr)
        {
            std::memcpy(&id, mapped, sizeof(id));
            m_Device.Core.UnmapBuffer(m_PickReadback->Handle);
        }
        m_PickResult = id == 0 ? entt::entity(entt::null) : entt::entity(id - 1);
    }

    std::array<uint32_t, SceneRenderer::kViews> SceneRenderer::ReadDrawCounts()
    {
        RHI::Buffer readback(m_Device, VriBufferDesc {kViews * 4, 0, VriBufferUsage_TransferDst, VriMemoryLocation_HostReadback});
        RHI::Frame  frame(m_Device);
        auto*       cmd      = frame.Begin();
        const auto  previous = m_DrawCounts->State;
        m_DrawCounts->Transition(cmd, {VriAccess_CopySourceRead, VriPipelineStage_Transfer});
        readback.Transition(cmd, {VriAccess_CopyDestinationWrite, VriPipelineStage_Transfer});
        VriBufferCopyDesc copy {};
        copy.size = kViews * 4;
        m_Device.Core.CmdCopyBuffer(cmd, readback.Handle, m_DrawCounts->Handle, &copy);
        m_DrawCounts->Transition(cmd, previous);
        frame.SubmitAndWait();
        std::array<uint32_t, kViews> counts {};
        const auto* mapped = m_Device.Core.MapBuffer(readback.Handle, 0, sizeof(counts));
        std::memcpy(counts.data(), mapped, sizeof(counts));
        m_Device.Core.UnmapBuffer(readback.Handle);
        return counts;
    }

    void SceneRenderer::WriteBindings(ShaderBindings& set, RenderGraph* graph)
    {
        // Once per frame: a set must not be updated after it was bound in the frame's command buffer (the four
        // shadow passes share one).
        auto& written = m_WrittenFrame[&set];
        if (written == m_FrameIndex)
        {
            return;
        }
        written = m_FrameIndex;
        set.Write("frame", m_FrameBuffer->View(VriDescriptorType_ConstantBuffer));
        set.Write("instances", m_InstanceBuffer->View(VriDescriptorType_StructuredBuffer));
        set.Write("materials", m_MaterialBuffer->View(VriDescriptorType_StructuredBuffer));
        if (set.Has("textures"))
        {
            std::vector<VriDescriptor*> views;
            views.reserve(m_Textures.size());
            for (auto* texture : m_Textures)
            {
                views.push_back(texture->View());
            }
            set.Write("textures", views);
        }
        set.Write("materialSampler", m_MaterialSampler);
        set.Write("linearClamp", m_LinearClamp);
        set.Write("environmentSampler", m_EnvironmentSampler);
        set.Write("shadowSampler", m_ShadowSampler);
        set.Write("radianceMap", (m_Radiance != nullptr ? m_Radiance : m_Black.get())->View());
        set.Write("irradianceMap", (m_Irradiance ? m_Irradiance.get() : m_Black.get())->View());
        set.Write("specularMap", (m_Specular ? m_Specular.get() : m_Black.get())->View());
        set.Write("brdfLut", m_BrdfLut->View());
        set.Write("drawCommands", m_DrawCommands->View(VriDescriptorType_StorageBuffer));
        set.Write("drawCounts", m_DrawCounts->View(VriDescriptorType_StorageBuffer));
        if (graph != nullptr)
        {
            auto texture = [&](const char* name) -> VriDescriptor* {
                const auto resource = graph->FindResource(name);
                return graph->GetTexture(resource).View();
            };
            if (set.Has("shadowMaps"))
            {
                VriDescriptor* shadows[kCascades] {texture("Shadow0"), texture("Shadow1"), texture("Shadow2"), texture("Shadow3")};
                set.Write("shadowMaps", shadows);
            }
            if (set.Has("gbufferAlbedoMetallic"))
            {
                set.Write("gbufferAlbedoMetallic", texture("GBufferAlbedoMetallic"));
                set.Write("gbufferNormalRoughness", texture("GBufferNormalRoughness"));
                set.Write("gbufferEmissiveOcclusion", texture("GBufferEmissiveOcclusion"));
                set.Write("sceneDepth", texture("SceneDepth"));
            }
            if (set.Has("hdrColor"))
            {
                set.Write("hdrColor", texture("SceneHDR"));
            }
        }
        set.Write("ssaoMap", graph != nullptr && m_AOResource ? graph->GetTexture(*m_AOResource).View() : m_White->View());
        set.Write("bloomMap", graph != nullptr && m_BloomResource ? graph->GetTexture(*m_BloomResource).View() : m_Black->View());
    }

    void SceneRenderer::BindGeometry(VriCommandBuffer* cmd)
    {
        const VriVertexBufferBinding streams[] {{m_Vertices->Handle, 0}, {m_InstanceStream->Handle, 0}};
        m_Device.Core.CmdSetVertexBuffers(cmd, 0, streams, 2);
        m_Device.Core.CmdSetIndexBuffer(cmd, m_Indices->Handle, 0, VriIndexType_UInt32);
    }

    void SceneRenderer::DrawIndirect(VriCommandBuffer* cmd, uint32_t view, uint32_t first, uint32_t count)
    {
        (void)first;
        if (count == 0)
        {
            return;
        }
        const uint64_t offset = uint64_t(view) * m_InstanceCapacity * kCommandStride;
        // The count comes from the culling pass, so no draw needs Vulkan's multiDrawIndirect (VRI does not enable it).
        // Without the count extension, commands are written in place and drawn one by one.
        if (m_IndirectCount)
        {
            m_Device.Core.CmdDrawIndexedIndirectCount(cmd, m_DrawCommands->Handle, offset, m_DrawCounts->Handle, view * 4u, count, kCommandStride);
        }
        else
        {
            for (uint32_t i = 0; i < count; ++i)
            {
                m_Device.Core.CmdDrawIndexedIndirect(cmd, m_DrawCommands->Handle, offset + uint64_t(i) * kCommandStride, 1, kCommandStride);
            }
        }
    }

    // ------------------------------------------------------------------------------------------------ graph

    void SceneRenderer::AddPasses(RenderGraph& graph, RenderGraph::Resource target, VriFormat targetFormat)
    {
        // Descriptor sets of the previous graph go away: forget which were written (their addresses can be reused).
        m_Device.WaitIdle();
        m_PostSets.clear();
        m_WrittenFrame.clear();
        m_AOResource.reset();
        m_BloomResource.reset();
        m_TargetFormat = targetFormat;

        // With FXAA, tone mapping writes an RGBA8 intermediate (luma in alpha) and FXAA writes the target.
        const VriFormat toneFormat = m_Settings.FXAA ? VriFormat_RGBA8_UNORM : targetFormat;
        if (toneFormat != m_ToneMappingFormat)
        {
            ShaderPipelineDesc desc;
            desc.ColorFormats     = {toneFormat};
            m_ToneMappingSet.reset();
            m_ToneMappingPipeline = CreateScope<ShaderPipeline>(m_Device, m_ToneMapping, m_ToneMapping.Subshaders.at(0).Passes.at(0), desc);
            m_ToneMappingSet      = CreateScope<ShaderBindings>(m_Device, *m_ToneMappingPipeline);
            m_ToneMappingFormat   = toneFormat;
        }
        if (m_Settings.EditorOverlay && targetFormat != m_OutlineFormat)
        {
            ShaderPipelineDesc desc;
            desc.ColorFormats = {targetFormat};
            m_OutlineSet.reset();
            m_OutlinePipeline = CreateScope<ShaderPipeline>(m_Device, m_EditorShader, *m_EditorShader.Subshaders.at(0).FindPass("SelectionOutline"), desc);
            m_OutlineSet      = CreateScope<ShaderBindings>(m_Device, *m_OutlinePipeline);
            m_OutlineFormat   = targetFormat;
        }
        if (m_Settings.FXAA && targetFormat != m_FXAAFormat)
        {
            ShaderPipelineDesc desc;
            desc.ColorFormats = {targetFormat};
            m_FXAAPipeline    = CreateScope<ShaderPipeline>(m_Device, m_PostShader, *m_PostShader.Subshaders.at(0).FindPass("FXAA"), desc);
            m_FXAAFormat      = targetFormat;
        }

        // A fullscreen post-processing pass: `inputs` become postSource / postSecondary.
        struct PostParameters
        {
            glm::vec4 Values;
            glm::vec4 Texel;
        };
        auto addPost = [this, &graph](std::string                             name,
                                      const ShaderPipeline&                   pipeline,
                                      RenderGraph::Resource                   output,
                                      std::vector<RenderGraph::Resource>      inputs,
                                      std::vector<RenderGraph::Use>           extra,
                                      std::function<PostParameters(RenderGraph&)> parameters) {
            m_PostSets.push_back(CreateScope<ShaderBindings>(m_Device, pipeline));
            ShaderBindings*               set = m_PostSets.back().get();
            std::vector<RenderGraph::Use> uses {{output, GraphUsage::ColorWrite, true}};
            for (const auto input : inputs)
            {
                uses.push_back({input, GraphUsage::Sampled});
            }
            uses.insert(uses.end(), extra.begin(), extra.end());
            graph.AddPass(std::move(name), uses, [this, set, &pipeline, output, inputs, parameters](VriCommandBuffer* cmd, RenderGraph& g) {
                WriteBindings(*set, &g);
                const char* names[] {"postSource", "postSecondary"};
                for (size_t i = 0; i < inputs.size() && i < 2; ++i)
                {
                    set->Write(names[i], g.GetTexture(inputs[i]).View());
                }
                auto& target = g.GetTexture(output);
                RHI::BeginColorPass(m_Device, cmd, target.View(), target.GetExtent());
                pipeline.Bind(cmd);
                set->Bind(cmd);
                const auto values = parameters(g);
                pipeline.SetConstants(cmd, &values, sizeof(values));
                const VriDrawDesc draw {3, 1, 0, 0};
                m_Device.Core.CmdDraw(cmd, &draw);
                m_Device.Core.CmdEndRendering(cmd);
            });
        };
        auto texelOf = [](RenderGraph& g, RenderGraph::Resource resource) {
            const auto size = g.GetTexture(resource).GetExtent();
            return glm::vec2(1.0f / float(size.Width), 1.0f / float(size.Height));
        };

        const auto extent   = m_Extent; // the target may be a transient, unallocated until Compile
        const auto commands = graph.ImportResource("DrawCommands", *m_DrawCommands, false);
        const auto counts   = graph.ImportResource("DrawCounts", *m_DrawCounts, false);

        RenderGraph::Resource shadows[kCascades];
        for (uint32_t i = 0; i < kCascades; ++i)
        {
            shadows[i] = graph.CreateTexture("Shadow" + std::to_string(i),
                                             RHI::DepthTextureDesc({m_Settings.ShadowResolution, m_Settings.ShadowResolution}));
        }
        const auto albedo   = graph.CreateTexture("GBufferAlbedoMetallic", RHI::ColorTextureDesc(extent, VriFormat_RGBA8_SRGB));
        const auto normal   = graph.CreateTexture("GBufferNormalRoughness", RHI::ColorTextureDesc(extent, VriFormat_RGBA16_SFLOAT));
        const auto emissive = graph.CreateTexture("GBufferEmissiveOcclusion", RHI::ColorTextureDesc(extent, VriFormat_RGBA16_SFLOAT));
        const auto depth    = graph.CreateTexture("SceneDepth", RHI::DepthTextureDesc(extent));
        const auto hdr      = graph.CreateTexture("SceneHDR", RHI::ColorTextureDesc(extent, kHDRFormat));

        // Skinned vertices -> their arena ranges (the arena is not a graph resource: it is read-only otherwise).
        graph.AddPass(
            "UploadSkinned",
            std::span<const RenderGraph::Use> {},
            [this](VriCommandBuffer* cmd, RenderGraph&) {
                if (m_DynamicCopies.empty())
                {
                    return;
                }
                m_Vertices->Transition(cmd, {VriAccess_CopyDestinationWrite, VriPipelineStage_Transfer});
                for (const auto& copy : m_DynamicCopies)
                {
                    VriBufferCopyDesc region {copy.Source, copy.Destination, copy.Size};
                    m_Device.Core.CmdCopyBuffer(cmd, m_Vertices->Handle, m_Staging->Handle, &region);
                }
                m_Vertices->Transition(cmd, {VriAccess_VertexBufferRead | VriAccess_IndexBufferRead, VriPipelineStage_AllCommands});
            },
            true);

        graph.AddPass("ClearDrawCounts", {{counts, GraphUsage::CopyDestination, true}}, [this](VriCommandBuffer* cmd, RenderGraph&) {
            m_Device.Core.CmdClearStorageBuffer(cmd, m_DrawCounts->Handle, 0, 0, 0);
        });

        graph.AddPass("Cull",
                      {{counts, GraphUsage::StorageReadWrite}, {commands, GraphUsage::StorageWrite, true}},
                      [this](VriCommandBuffer* cmd, RenderGraph&) {
                          WriteBindings(*m_CullSet, nullptr);
                          m_CullPipeline->Bind(cmd);
                          m_CullSet->Bind(cmd);
                          auto dispatch = [&](uint32_t view, uint32_t first, uint32_t count, bool compact, bool shadow) {
                              if (count == 0)
                              {
                                  return;
                              }
                              CullParameters parameters {};
                              std::copy(m_ViewPlanes[view].begin(), m_ViewPlanes[view].end(), parameters.Planes);
                              parameters.Range   = {first, count, view, m_InstanceCapacity};
                              parameters.Options = {compact && m_IndirectCount ? 1u : 0u, shadow ? 1u : 0u, 0, 0};
                              m_CullPipeline->SetConstants(cmd, &parameters, sizeof(parameters));
                              const VriDispatchDesc groups {(count + 63) / 64, 1, 1};
                              m_Device.Core.CmdDispatch(cmd, &groups);
                          };
                          dispatch(kViewCamera, 0, m_OpaqueCount, true, false);
                          dispatch(kViewAlphaTest, m_OpaqueCount, m_AlphaCount, true, false);
                          if (m_Frame.Shadow.w > 0)
                          {
                              for (uint32_t cascade = 0; cascade < kCascades; ++cascade)
                              {
                                  dispatch(1 + cascade, 0, m_OpaqueCount, true, true);
                                  dispatch(kViewAlphaTest + 1 + cascade, m_OpaqueCount, m_AlphaCount, true, true);
                              }
                          }
                          dispatch(kViewTransparent, m_OpaqueCount + m_AlphaCount, m_TransparentCount, false, false);
                      });

        for (uint32_t cascade = 0; cascade < kCascades; ++cascade)
        {
            graph.AddPass("Shadow" + std::to_string(cascade),
                          {{shadows[cascade], GraphUsage::DepthWrite, true},
                           {commands, GraphUsage::IndirectRead},
                           {counts, GraphUsage::IndirectRead}},
                          [this, cascade, target = shadows[cascade]](VriCommandBuffer* cmd, RenderGraph& g) {
                              auto& map = g.GetTexture(target);
                              BeginRendering(m_Device, cmd, {}, nullptr, map.View(), true, map.GetExtent());
                              if (m_Frame.Shadow.w > 0)
                              {
                                  WriteBindings(*m_ShadowSet, nullptr);
                                  m_ShadowPipeline->Bind(cmd);
                                  m_ShadowSet->Bind(cmd);
                                  BindGeometry(cmd);
                                  const glm::uvec4 parameters {cascade, 0, 0, 0};
                                  m_ShadowPipeline->SetConstants(cmd, &parameters, sizeof(parameters));
                                  DrawIndirect(cmd, 1 + cascade, 0, m_OpaqueCount);
                                  if (m_AlphaCount > 0)
                                  {
                                      WriteBindings(*m_ShadowAlphaSet, nullptr);
                                      m_ShadowAlphaPipeline->Bind(cmd);
                                      m_ShadowAlphaSet->Bind(cmd);
                                      m_ShadowAlphaPipeline->SetConstants(cmd, &parameters, sizeof(parameters));
                                      DrawIndirect(cmd, kViewAlphaTest + 1 + cascade, 0, m_AlphaCount);
                                  }
                              }
                              m_Device.Core.CmdEndRendering(cmd);
                          });
        }

        graph.AddPass("GBuffer",
                      {{albedo, GraphUsage::ColorWrite, true},
                       {normal, GraphUsage::ColorWrite, true},
                       {emissive, GraphUsage::ColorWrite, true},
                       {depth, GraphUsage::DepthWrite, true},
                       {commands, GraphUsage::IndirectRead},
                       {counts, GraphUsage::IndirectRead}},
                      [this, albedo, normal, emissive, depth](VriCommandBuffer* cmd, RenderGraph& g) {
                          VriDescriptor* colors[] {g.GetTexture(albedo).View(), g.GetTexture(normal).View(), g.GetTexture(emissive).View()};
                          const float    clear[4] {0, 0, 0, 0};
                          BeginRendering(m_Device, cmd, colors, clear, g.GetTexture(depth).View(), true, g.GetTexture(depth).GetExtent());
                          WriteBindings(*m_GBufferSet, nullptr);
                          m_GBufferPipeline->Bind(cmd);
                          m_GBufferSet->Bind(cmd);
                          BindGeometry(cmd);
                          DrawIndirect(cmd, kViewCamera, 0, m_OpaqueCount);
                          if (m_AlphaCount > 0)
                          {
                              WriteBindings(*m_GBufferAlphaSet, nullptr);
                              m_GBufferAlphaPipeline->Bind(cmd);
                              m_GBufferAlphaSet->Bind(cmd);
                              DrawIndirect(cmd, kViewAlphaTest, 0, m_AlphaCount);
                          }
                          m_Device.Core.CmdEndRendering(cmd);
                      });

        if (m_Settings.AmbientOcclusion)
        {
            const auto raw = graph.CreateTexture("AORaw", RHI::ColorTextureDesc(extent, VriFormat_R16_SFLOAT));
            const auto ao  = graph.CreateTexture("AO", RHI::ColorTextureDesc(extent, VriFormat_R16_SFLOAT));
            addPost("GTAO", *m_GTAOPipeline, raw, {}, {{depth, GraphUsage::Sampled}, {normal, GraphUsage::Sampled}}, [this](RenderGraph&) {
                return PostParameters {{m_Settings.AmbientOcclusionRadius, 2, 8, 0.6f}, {}};
            });
            addPost("AODenoise", *m_AODenoisePipeline, ao, {raw}, {{depth, GraphUsage::Sampled}}, [this](RenderGraph&) {
                return PostParameters {{m_Settings.AmbientOcclusionIntensity, 0, 0, 0}, {}};
            });
            m_AOResource = ao;
        }

        std::vector<RenderGraph::Use> lightingUses {{hdr, GraphUsage::ColorWrite, true},
                                                    {albedo, GraphUsage::Sampled},
                                                    {normal, GraphUsage::Sampled},
                                                    {emissive, GraphUsage::Sampled},
                                                    {depth, GraphUsage::Sampled}};
        for (const auto shadow : shadows)
        {
            lightingUses.push_back({shadow, GraphUsage::Sampled});
        }
        if (m_AOResource)
        {
            lightingUses.push_back({*m_AOResource, GraphUsage::Sampled});
        }
        graph.AddPass("DeferredLighting", lightingUses, [this, hdr](VriCommandBuffer* cmd, RenderGraph& g) {
            auto& target = g.GetTexture(hdr);
            RHI::BeginColorPass(m_Device, cmd, target.View(), target.GetExtent());
            WriteBindings(*m_LightingSet, &g);
            m_LightingPipeline->Bind(cmd);
            m_LightingSet->Bind(cmd);
            const VriDrawDesc draw {3, 1, 0, 0};
            m_Device.Core.CmdDraw(cmd, &draw);
            m_Device.Core.CmdEndRendering(cmd);
        });

        std::vector<RenderGraph::Use> forwardUses {{hdr, GraphUsage::ColorReadWrite},
                                                   {depth, GraphUsage::DepthRead},
                                                   {commands, GraphUsage::IndirectRead},
                                                   {counts, GraphUsage::IndirectRead}};
        for (const auto shadow : shadows)
        {
            forwardUses.push_back({shadow, GraphUsage::Sampled});
        }
        graph.AddPass("Transparent", forwardUses, [this, hdr, depth](VriCommandBuffer* cmd, RenderGraph& g) {
            auto& target = g.GetTexture(hdr);
            VriDescriptor* colors[] {target.View()};
            BeginRendering(m_Device, cmd, colors, nullptr, g.GetTexture(depth).View(), false, target.GetExtent());
            if (m_TransparentCount > 0)
            {
                WriteBindings(*m_ForwardSet, &g);
                m_ForwardPipeline->Bind(cmd);
                m_ForwardSet->Bind(cmd);
                BindGeometry(cmd);
                DrawIndirect(cmd, kViewTransparent, 0, m_TransparentCount);
            }
            m_Device.Core.CmdEndRendering(cmd);
        });

        if (m_Settings.Bloom)
        {
            // down[i]: 1 / 2^(i+1) resolution; up[i] = tent(up[i + 1]) + down[i]; the result is up[0] (half size).
            const uint32_t                     levels = std::clamp(m_Settings.BloomLevels, 1u, 10u);
            std::vector<RenderGraph::Resource> down, up(levels);
            for (uint32_t i = 0; i < levels; ++i)
            {
                const RHI::Extent size {std::max(extent.Width >> (i + 1), 1u), std::max(extent.Height >> (i + 1), 1u)};
                down.push_back(graph.CreateTexture("BloomDown" + std::to_string(i), RHI::ColorTextureDesc(size, VriFormat_RGBA16_SFLOAT)));
                const auto source = i == 0 ? hdr : down[i - 1];
                addPost("BloomDown" + std::to_string(i), *m_BloomDownPipeline, down[i], {source}, {}, [i, source, texelOf](RenderGraph& g) {
                    return PostParameters {{i == 0 ? 1.0f : 0.0f, 0, 0, 0}, glm::vec4(texelOf(g, source), 0, 0)};
                });
            }
            up[levels - 1] = down[levels - 1];
            for (int i = int(levels) - 2; i >= 0; --i)
            {
                const RHI::Extent size {std::max(extent.Width >> (i + 1), 1u), std::max(extent.Height >> (i + 1), 1u)};
                up[i]              = graph.CreateTexture("BloomUp" + std::to_string(i), RHI::ColorTextureDesc(size, VriFormat_RGBA16_SFLOAT));
                const auto smaller = up[i + 1];
                addPost("BloomUp" + std::to_string(i), *m_BloomUpPipeline, up[i], {smaller, down[i]}, {}, [smaller, texelOf](RenderGraph& g) {
                    return PostParameters {{1, 0, 0, 0}, glm::vec4(texelOf(g, smaller), 0, 0)};
                });
            }
            m_BloomResource = up[0];
        }

        const auto toneTarget = m_Settings.FXAA ? graph.CreateTexture("SceneLDR", RHI::ColorTextureDesc(extent, VriFormat_RGBA8_UNORM)) : target;
        std::vector<RenderGraph::Use> toneUses {{hdr, GraphUsage::Sampled}, {toneTarget, GraphUsage::ColorWrite, true}};
        if (m_BloomResource)
        {
            toneUses.push_back({*m_BloomResource, GraphUsage::Sampled});
        }
        graph.AddPass("ToneMapping",
                      toneUses,
                      [this, target = toneTarget](VriCommandBuffer* cmd, RenderGraph& g) {
                          auto& output = g.GetTexture(target);
                          RHI::BeginColorPass(m_Device, cmd, output.View(), output.GetExtent());
                          WriteBindings(*m_ToneMappingSet, &g);
                          m_ToneMappingPipeline->Bind(cmd);
                          m_ToneMappingSet->Bind(cmd);
                          const glm::vec4 parameters {m_Settings.Exposure, 1, m_BloomResource ? m_Settings.BloomStrength : 0.0f, 0};
                          m_ToneMappingPipeline->SetConstants(cmd, &parameters, sizeof(parameters));
                          const VriDrawDesc draw {3, 1, 0, 0};
                          m_Device.Core.CmdDraw(cmd, &draw);
                          m_Device.Core.CmdEndRendering(cmd);
                      });

        if (m_Settings.FXAA)
        {
            addPost("FXAA", *m_FXAAPipeline, target, {toneTarget}, {}, [toneTarget, texelOf](RenderGraph& g) {
                return PostParameters {{}, glm::vec4(texelOf(g, toneTarget), 0, 0)};
            });
        }

        if (m_Settings.EditorOverlay)
        {
            AddEditorPasses(graph, target, commands, counts);
        }

        // In-game UI in display space, over the tone-mapped (and anti-aliased) image.
        graph.AddPass("UI", {{target, GraphUsage::ColorReadWrite}}, [this, target](VriCommandBuffer* cmd, RenderGraph& g) {
            if (m_UI->GetQuadCount() == 0)
            {
                return;
            }
            auto& output = g.GetTexture(target);
            RHI::BeginColorPass(m_Device, cmd, output.View(), output.GetExtent());
            m_UI->Draw(cmd, m_TargetFormat, [this](ShaderBindings& set) { WriteBindings(set, nullptr); });
            m_Device.Core.CmdEndRendering(cmd);
        });
    }
    void SceneRenderer::AddEditorPasses(RenderGraph& graph, RenderGraph::Resource target, RenderGraph::Resource commands, RenderGraph::Resource counts)
    {
        const auto ids   = graph.CreateTexture("EntityID", RHI::ColorTextureDesc(m_Extent, VriFormat_R32_UINT));
        const auto depth = graph.CreateTexture("EntityIDDepth", RHI::DepthTextureDesc(m_Extent));

        // The camera's opaque, alpha-tested and transparent draws (the culling commands of the frame), ids only.
        graph.AddPass("EntityID",
                      {{ids, GraphUsage::ColorWrite, true},
                       {depth, GraphUsage::DepthWrite, true},
                       {commands, GraphUsage::IndirectRead},
                       {counts, GraphUsage::IndirectRead}},
                      [this, ids, depth](VriCommandBuffer* cmd, RenderGraph& g) {
                          VriDescriptor* colors[] {g.GetTexture(ids).View()};
                          const float    clear[4] {0, 0, 0, 0};
                          BeginRendering(m_Device, cmd, colors, clear, g.GetTexture(depth).View(), true, m_Extent);
                          BindGeometry(cmd);
                          WriteBindings(*m_EntityIdSet, nullptr);
                          m_EntityIdPipeline->Bind(cmd);
                          m_EntityIdSet->Bind(cmd);
                          DrawIndirect(cmd, kViewCamera, 0, m_OpaqueCount);
                          DrawIndirect(cmd, kViewTransparent, 0, m_TransparentCount);
                          if (m_AlphaCount > 0)
                          {
                              WriteBindings(*m_EntityIdAlphaSet, nullptr);
                              m_EntityIdAlphaPipeline->Bind(cmd);
                              m_EntityIdAlphaSet->Bind(cmd);
                              DrawIndirect(cmd, kViewAlphaTest, 0, m_AlphaCount);
                          }
                          m_Device.Core.CmdEndRendering(cmd);
                      });

        // One texel to the host when a pick was requested (read in OnFrameComplete).
        graph.AddPass(
            "PickReadback",
            {{ids, GraphUsage::CopySource}},
            [this, ids](VriCommandBuffer* cmd, RenderGraph& g) {
                if (!m_PickRequest)
                {
                    return;
                }
                const auto pixel = glm::min(*m_PickRequest, glm::uvec2(m_Extent.Width - 1, m_Extent.Height - 1));
                m_PickInFlight   = pixel;
                m_PickRequest.reset();
                m_PickReadback->Transition(cmd, {VriAccess_CopyDestinationWrite, VriPipelineStage_Transfer});
                VriBufferTextureCopyDesc region {};
                region.texture.layerNum = 1;
                region.texture.aspect   = VriImageAspect_Color;
                region.texture.x        = int32_t(pixel.x);
                region.texture.y        = int32_t(pixel.y);
                region.texture.width    = 1;
                region.texture.height   = 1;
                region.texture.depth    = 1;
                m_Device.Core.CmdReadbackTextureToBuffer(cmd, m_PickReadback->Handle, g.GetTexture(ids).Handle, &region);
            },
            true);

        graph.AddPass("SelectionOutline", {{target, GraphUsage::ColorReadWrite}, {ids, GraphUsage::Sampled}}, [this, target, ids](VriCommandBuffer* cmd, RenderGraph& g) {
            if (m_Selection.empty())
            {
                return;
            }
            struct OutlineParameters
            {
                glm::uvec4 Ids[kMaxSelection / 4];
                glm::vec4  Color;
                glm::uvec4 Options;
            } parameters {};
            for (size_t i = 0; i < m_Selection.size(); ++i)
            {
                parameters.Ids[i / 4][i % 4] = m_Selection[i];
            }
            parameters.Color   = glm::vec4(m_Settings.SelectionColor, 0.12f);
            parameters.Options = {uint32_t(m_Selection.size()), m_Settings.SelectionOutline, 0, 0};

            m_OutlineSet->Write("entityIdMap", g.GetTexture(ids).View());
            auto& output = g.GetTexture(target);
            RHI::BeginColorPass(m_Device, cmd, output.View(), output.GetExtent());
            m_OutlinePipeline->Bind(cmd);
            m_OutlineSet->Bind(cmd);
            m_OutlinePipeline->SetConstants(cmd, &parameters, sizeof(parameters));
            const VriDrawDesc draw {3, 1, 0, 0};
            m_Device.Core.CmdDraw(cmd, &draw);
            m_Device.Core.CmdEndRendering(cmd);
        });
    }
} // namespace SnowLeopardEngine
