#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"
#include "SnowLeopardEngine/Function/Rendering/RenderGraph/RenderGraph.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Renderer/UIRenderer.h"
#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderAsset.h"
#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderPipeline.h"

#include <array>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace SnowLeopardEngine
{
    class LogicScene;
    class Material;
    struct MeshItem;

    struct SceneRendererSettings
    {
        uint32_t ShadowResolution = 2048;
        float    ShadowDistance   = 200.0f; // cascades cover [near, min(far, this)]
        float    CascadeLambda    = 0.8f;   // 0 uniform .. 1 logarithmic splits
        float    Exposure         = 0.6f;   // linear scale before ACES (the fit expects ~0.6 for the reference curve)
    };

    // GPU-driven scene renderer (Phase 2.4).
    //
    // Data lives on the GPU once: every mesh in one vertex / index arena, every object of the frame in an instance
    // buffer, every material in a material buffer whose textures are indices into a bindless table. A compute pass
    // frustum-culls the instances for each view (camera + 4 shadow cascades) and writes indexed indirect draws, so
    // each geometry pass is a single DrawIndexedIndirectCount however many objects there are.
    //
    // Render queues: Opaque, AlphaTest (cut-out, two-sided, optional wind: foliage) and Transparent (sorted far to
    // near). Identical meshes (e.g. thousands of grass quads, each its own MeshItem) share one arena range.
    //
    // Frame: upload (skinned vertices) -> cull -> 4 shadow cascades -> G-buffer -> deferred lighting + sky ->
    // transparent forward -> tone mapping into the caller's target -> in-game UI (UIRenderer).
    class SceneRenderer
    {
    public:
        explicit SceneRenderer(RHI::Device& device, SceneRendererSettings settings = {});
        ~SceneRenderer();

        SceneRenderer(const SceneRenderer&)            = delete;
        SceneRenderer& operator=(const SceneRenderer&) = delete;

        // CPU half of a frame, before recording: collects the scene, uploads new meshes, materials and the
        // environment, writes the frame / instance / material buffers. False: nothing to draw (no camera).
        bool Prepare(LogicScene* scene, RHI::Extent extent);

        // Adds the scene passes to `graph`; the tone-mapped image is written to `target` (UNORM colour, extent of
        // the last Prepare). Passes read the state of the latest Prepare when the graph executes.
        void AddPasses(RenderGraph& graph, RenderGraph::Resource target, VriFormat targetFormat);

        // Bumped when buffers imported by AddPasses are reallocated: rebuild the graph before executing it again.
        uint64_t GetGraphVersion() const { return m_GraphVersion; }

        // Scene GPU caches (meshes, materials) refer to the previous scene's objects: drop them.
        void OnSceneChanged();

        // Culling views: per queue, the camera and the 4 shadow cascades.
        static constexpr uint32_t kViews           = 11;
        static constexpr uint32_t kViewCamera      = 0; // opaque; 1 - 4: opaque shadow cascades
        static constexpr uint32_t kViewTransparent = 5;
        static constexpr uint32_t kViewAlphaTest   = 6; // alpha-tested; 7 - 10: their shadow cascades

        struct Statistics
        {
            uint32_t Instances    = 0;
            uint32_t Transparent  = 0;
            uint32_t AlphaTest    = 0;
            uint32_t UIQuads      = 0;
            uint32_t UniqueMeshes = 0;
            uint32_t Materials    = 0;
            uint32_t Textures     = 0;
            uint64_t ArenaVertices = 0;
            uint64_t ArenaIndices  = 0;
        };
        const Statistics& GetStatistics() const { return m_Stats; }

        // Draw counts the GPU culling produced in the last executed frame, per view (kView*). Blocking readback, for
        // tests and diagnostics.
        std::array<uint32_t, kViews> ReadDrawCounts();

        UIRenderer& GetUI() { return *m_UI; }

    public:
        // GPU layouts, mirrored in Assets/Shaders/Slang/Scene.slangh.
        struct GPUPointLight
        {
            glm::vec4 Position;
            glm::vec4 Color;
            glm::vec4 Attenuation;
        };
        static constexpr uint32_t kMaxPointLights = 16;
        static constexpr uint32_t kMaxTextures    = 1024;
        static constexpr uint32_t kCascades       = 4;

        struct GPUFrame
        {
            glm::mat4     View;
            glm::mat4     Projection;
            glm::mat4     ViewProjection;
            glm::mat4     InverseViewProjection;
            glm::mat4     LightViewProjection[kCascades];
            glm::vec4     CameraPosition;
            glm::vec4     Resolution;
            glm::vec4     LightDirection;
            glm::vec4     LightColor;
            glm::vec4     CascadeSplits;
            glm::vec4     CascadeTexel;
            glm::vec4     Shadow;
            glm::vec4     Environment;
            glm::vec4     SkyColor;
            glm::uvec4    Counts;
            GPUPointLight PointLights[kMaxPointLights];
        };

        struct GPUInstance
        {
            glm::mat4  Model;
            glm::mat4  NormalMatrix;
            glm::vec4  BoundingSphere;
            glm::uvec4 Draw;  // index count, first index, vertex offset, material
            glm::uvec4 Flags; // casts shadows
        };

        struct GPUMaterial
        {
            glm::vec4  Albedo;
            glm::vec4  Emissive;
            glm::vec4  Surface;
            glm::vec4  Alpha; // cutoff, wind strength, wind direction (xz)
            glm::uvec4 Maps;
            glm::uvec4 Maps2; // emissive, wind noise
        };

    private:
        struct MeshRecord
        {
            uint32_t    FirstIndex   = 0;
            uint32_t    IndexCount   = 0;
            uint32_t    VertexOffset = 0;
            uint32_t    VertexCount  = 0;
            glm::vec4   LocalSphere {0};
            const void* Source = nullptr; // Vertices.data() at upload: detects a reused MeshItem address
        };

        struct DynamicCopy
        {
            const void* Data;     // CPU-skinned vertices (valid until the frame is recorded)
            uint64_t Source;      // offset in the frame's staging buffer
            uint64_t Destination; // byte offset in the vertex arena
            uint64_t Size;
        };

        void CreateSamplersAndDefaults();
        void LoadShaders();
        void CreatePipelines();

        const MeshRecord* UploadMesh(const MeshItem& mesh);
        void              FlushMeshUploads();
        void              GrowArena(uint64_t vertices, uint64_t indices);
        uint32_t          MaterialIndex(const Material& material);
        uint32_t          TextureIndex(RHI::Texture* texture);
        void              SetEnvironment(const std::filesystem::path& path);
        void              EnsureInstanceCapacity(uint32_t count);

        // Writes the renderer resources into a pass's descriptor set (names the pass does not use are skipped).
        void WriteBindings(ShaderBindings& bindings, RenderGraph* graph);
        void DrawIndirect(VriCommandBuffer* cmd, uint32_t view, uint32_t first, uint32_t count);
        void BindGeometry(VriCommandBuffer* cmd);

    private:
        RHI::Device&          m_Device;
        SceneRendererSettings m_Settings;
        bool                  m_IndirectCount = false; // DrawIndexedIndirectCount available

        // Shaders and pipelines
        ShaderAsset          m_Standard, m_Culling, m_Lighting, m_ToneMapping, m_EnvironmentShader;
        Scope<ShaderPipeline> m_GBufferPipeline, m_GBufferAlphaPipeline, m_ForwardPipeline, m_ShadowPipeline, m_ShadowAlphaPipeline,
            m_CullPipeline, m_LightingPipeline, m_ToneMappingPipeline;
        Scope<ShaderBindings> m_GBufferSet, m_GBufferAlphaSet, m_ForwardSet, m_ShadowSet, m_ShadowAlphaSet, m_CullSet, m_LightingSet,
            m_ToneMappingSet;
        Scope<UIRenderer>     m_UI;
        VriFormat             m_TargetFormat = VriFormat_Unknown;

        // Samplers and placeholder textures
        VriDescriptor*      m_MaterialSampler    = nullptr;
        VriDescriptor*      m_LinearClamp        = nullptr;
        VriDescriptor*      m_EnvironmentSampler = nullptr;
        VriDescriptor*      m_ShadowSampler      = nullptr;
        Scope<RHI::Texture> m_White, m_Black;

        // Environment (image-based lighting)
        std::filesystem::path m_EnvironmentPath;
        RHI::Texture*         m_Radiance = nullptr; // owned by the texture cache
        Scope<RHI::Texture>   m_Irradiance, m_Specular, m_BrdfLut;

        // Mesh arena
        Scope<RHI::Buffer>                                 m_Vertices, m_Indices;
        uint64_t                                           m_VertexCapacity = 0, m_IndexCapacity = 0;
        uint64_t                                           m_VertexCount = 0, m_IndexCount = 0;
        std::unordered_map<const MeshItem*, MeshRecord>    m_Meshes;
        std::unordered_map<uint64_t, MeshRecord>           m_SharedMeshes; // content hash -> range (static meshes)
        std::vector<MeshVertexData>                        m_PendingVertices; // appended this frame, uploaded at once
        std::vector<uint32_t>                              m_PendingIndices;
        uint64_t                                           m_UploadedVertices = 0, m_UploadedIndices = 0;

        // Materials and the bindless table
        std::unordered_map<const Material*, uint32_t> m_MaterialIndices;
        std::vector<GPUMaterial>                      m_Materials;
        std::vector<RHI::Texture*>                    m_Textures; // [0] = white, "no texture"
        std::unordered_map<RHI::Texture*, uint32_t>   m_TextureIndices;

        // Per-frame data (host visible; one frame in flight)
        Scope<RHI::Buffer>       m_FrameBuffer, m_InstanceBuffer, m_MaterialBuffer, m_Staging;
        uint32_t                 m_InstanceCapacity = 0;
        Scope<RHI::Buffer>       m_InstanceStream; // identity 0 .. capacity - 1, per-instance vertex stream
        Scope<RHI::Buffer>       m_DrawCommands, m_DrawCounts;
        std::vector<DynamicCopy> m_DynamicCopies;
        GPUFrame                 m_Frame {};
        std::array<glm::vec4, 6> m_ViewPlanes[kViews] {};
        uint32_t                 m_OpaqueCount = 0, m_AlphaCount = 0, m_TransparentCount = 0;
        RHI::Extent              m_Extent {};
        uint64_t                 m_GraphVersion = 1;
        uint64_t                 m_FrameIndex   = 0;
        std::unordered_map<const ShaderBindings*, uint64_t> m_WrittenFrame;
        Statistics               m_Stats;
    };
} // namespace SnowLeopardEngine
