#pragma once

#include "SnowLeopardEngine/Function/Rendering/IndexBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Texture.h"
#include "SnowLeopardEngine/Function/Rendering/VertexAttributes.h"
#include "SnowLeopardEngine/Function/Rendering/VertexBuffer.h"

namespace SnowLeopardEngine
{
    class Material;

    using UniformBuffer = Buffer;
    using StorageBuffer = Buffer;

    // Layout of CPU pixel data passed to RenderContext::Upload.
    enum class ImageFormat : uint8_t
    {
        Unknown = 0,
        Red,
        RG,
        RGB,
        RGBA
    };

    enum class ImageDataType : uint8_t
    {
        Unknown = 0,
        UByte,
        Float
    };

    struct ImageData
    {
        ImageFormat   Format {ImageFormat::Unknown};
        ImageDataType DataType {ImageDataType::Unknown};
        const void*   Pixels {nullptr};
    };

    enum class ImageAccess : uint8_t
    {
        ReadOnly = 0,
        WriteOnly,
        ReadWrite
    };

    template<typename T>
    using OptionalReference = std::optional<std::reference_wrapper<T>>;

    using ClearValue = std::variant<glm::vec4, float>;

    struct AttachmentInfo
    {
        Texture&                  Image;
        uint32_t                  MipLevel {0};
        std::optional<uint32_t>   Layer {};
        std::optional<uint32_t>   Face {};
        std::optional<ClearValue> ClearValue {};
    };
    struct RenderingInfo
    {
        Rect2D                        Area;
        std::vector<AttachmentInfo>   ColorAttachments;
        std::optional<AttachmentInfo> DepthAttachment {};
    };

    enum class PrimitiveTopology : uint8_t
    {
        Undefined = 0,

        PointList,
        LineList,
        LineStrip,

        TriangleList,
        TriangleStrip,

        PatchList
    };

    // struct GeometryInfo
    // {
    //     PrimitiveTopology Topology {PrimitiveTopology::TriangleList};
    //     uint32_t          VertexOffset {0};
    //     uint32_t          NumVertices {0};
    //     uint32_t          IndexOffset {0};
    //     uint32_t          NumIndices {0};

    //     // clang-format off
    //     auto operator<=> (const GeometryInfo&) const = default;
    //     // clang-format on
    // };

    class RenderContext
    {
    public:
        RenderContext();
        ~RenderContext();

        RenderContext& SetViewport(const Rect2D& rect);
        Rect2D         GetViewport();

        RenderContext& SetScissor(const Rect2D& rect);

        Buffer       CreateBuffer(int64_t size, const void* data = nullptr);
        VertexBuffer CreateVertexBuffer(int32_t stride, int64_t capacity, const void* data = nullptr);
        IndexBuffer  CreateIndexBuffer(IndexType, int64_t capacity, const void* data = nullptr);

        RenderHandle GetVertexArray(const VertexAttributes&);

        RenderHandle CreateGraphicsProgram(const std::string&                vertSource,
                                           const std::string&                fragSource,
                                           const std::optional<std::string>& geomSource = std::nullopt);

        RenderHandle CreateComputeProgram(const std::string& compSource);

        Texture CreateTexture2D(Extent2D extent, PixelFormat, uint32_t numMipLevels = 1u, uint32_t numLayers = 0u);
        Texture CreateTexture3D(Extent2D, uint32_t depth, PixelFormat);
        Texture CreateCubemap(uint32_t size, PixelFormat, uint32_t numMipLevels = 1u, uint32_t numLayers = 0u);

        RenderContext& GenerateMipmaps(Texture&);

        RenderContext& SetupSampler(Texture&, const SamplerInfo&);

        // Identifier of a texture for the UI backend (ImTextureID); for OpenGL the texture name.
        static uint64_t GetUITextureID(const Texture&);
        RenderHandle   CreateSampler(const SamplerInfo&);

        RenderContext& Clear(Texture&);
        // Upload Texture2D
        RenderContext& Upload(Texture&, int32_t mipLevel, glm::uvec2 dimensions, const ImageData&);
        // Upload Cubemap face
        RenderContext& Upload(Texture&, int32_t mipLevel, int32_t face, glm::uvec2 dimensions, const ImageData&);
        RenderContext& Upload(
            Texture&, int32_t mipLevel, const glm::uvec3& dimensions, int32_t face, int32_t layer, const ImageData&);

        RenderContext& Clear(Buffer&);
        RenderContext& Upload(Buffer&, int64_t offset, int64_t size, const void* data);
        void*          Map(Buffer&);
        RenderContext& Unmap(Buffer&);

        RenderContext& Destroy(Buffer&);
        RenderContext& Destroy(Texture&);
        RenderContext& Destroy(GraphicsPipeline&);

        RenderContext& Dispatch(RenderHandle computeProgram, const glm::uvec3& numGroups);

        RenderHandle   BeginRendering(const RenderingInfo& info);
        RenderContext& BeginRendering(const Rect2D&            area,
                                      std::optional<glm::vec4> clearColor   = {},
                                      std::optional<float>     clearDepth   = {},
                                      std::optional<int>       clearStencil = {});
        RenderContext& EndRendering(RenderHandle frameBufferID);

        RenderContext& SetUniform1f(const std::string& name, float);
        RenderContext& SetUniform1i(const std::string& name, int32_t);
        RenderContext& SetUniform1ui(const std::string& name, uint32_t);

        RenderContext& SetUniformVec3(const std::string& name, const glm::vec3&);
        RenderContext& SetUniformVec4(const std::string& name, const glm::vec4&);

        RenderContext& SetUniformMat3(const std::string& name, const glm::mat3&);
        RenderContext& SetUniformMat4(const std::string& name, const glm::mat4&);

        RenderContext& BindGraphicsPipeline(const GraphicsPipeline& pipeline);
        RenderContext& BindImage(uint32_t unit, const Texture&, int32_t mipLevel, ImageAccess access);
        RenderContext& BindTexture(uint32_t unit, const Texture&, std::optional<RenderHandle> samplerId = {});
        RenderContext& BindUniformBuffer(uint32_t index, const UniformBuffer&);
        RenderContext& BindMaterial(Material*);

        RenderContext& DrawFullScreenTriangle();
        RenderContext& DrawCube();
        RenderContext& Draw(OptionalReference<const VertexBuffer> vertexBuffer,
                            OptionalReference<const IndexBuffer>  indexBuffer,
                            uint32_t                              numIndices,
                            uint32_t                              numVertices,
                            uint32_t                              numInstances = 1);

        struct ResourceDeleter
        {
            void operator()(auto* ptr)
            {
                Context.Destroy(*ptr);
                delete ptr;
            }

            RenderContext& Context;
        };

    private:
        RenderHandle CreateVertexArray(const VertexAttributes&);

        Texture CreateImmutableTexture(Extent2D,
                                       uint32_t depth,
                                       PixelFormat,
                                       uint32_t numFaces,
                                       uint32_t numMipLevels,
                                       uint32_t numLayers);

        void CreateFaceView(Texture& cubeMap, uint32_t mipLevel, uint32_t layer, uint32_t face);
        // attachment: backend attachment point (GL_COLOR_ATTACHMENTi / GL_DEPTH_ATTACHMENT for OpenGL)
        void AttachTexture(RenderHandle framebuffer, uint32_t attachment, const AttachmentInfo&);

        RenderHandle CreateShaderProgram(std::initializer_list<RenderHandle> shaders);
        // type: backend shader stage (GL_VERTEX_SHADER, ... for OpenGL)
        RenderHandle CreateShaderObject(uint32_t type, const std::string& shaderSource);

        void SetShaderProgram(RenderHandle);
        void SetVertexArray(RenderHandle);
        void SetVertexBuffer(const VertexBuffer&);
        void SetIndexBuffer(const IndexBuffer&);

        void SetDepthTest(bool enabled, CompareOp);
        void SetDepthWrite(bool enabled);

        void SetPolygonMode(PolygonMode);
        void SetPolygonOffset(std::optional<PolygonOffset>);
        void SetCullMode(CullMode);
        void SetDepthClamp(bool enabled);
        void SetScissorTest(bool enabled);

        void SetBlendState(uint32_t index, const BlendState&);

    private:
        bool             m_RenderingStarted = false;
        GraphicsPipeline m_CurrentPipeline;

        RenderHandle                                  m_DummyVAO {kNullHandle};
        std::unordered_map<std::size_t, RenderHandle> m_VertexArrays;
    };

    class DebugMarker
    {
    public:
        explicit DebugMarker(std::string_view name);
        ~DebugMarker();
    };

#if 1
#define NAMED_DEBUG_MARKER(name) \
    const DebugMarker dm##__LINE__ { name }
#define DEBUG_MARKER() \
    const DebugMarker dm##__LINE__ { __FUNCTION__ }
#else
#define NAMED_DEBUG_MARKER(name)
#define DEBUG_MARKER()
#endif
} // namespace SnowLeopardEngine