#pragma once

#include "SnowLeopardEngine/Function/Rendering/IndexBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Texture.h"
#include "SnowLeopardEngine/Function/Rendering/VertexAttributes.h"
#include "SnowLeopardEngine/Function/Rendering/VertexBuffer.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    class Material;

    using UniformBuffer = Buffer;
    using StorageBuffer = Buffer;

    struct ImageData
    {
        GLenum      Format {GL_NONE};
        GLenum      DataType {GL_NONE};
        const void* Pixels {nullptr};
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

    enum class PrimitiveTopology : GLenum
    {
        Undefined = GL_NONE,

        PointList = GL_POINTS,
        LineList  = GL_LINES,
        LineStrip = GL_LINE_STRIP,

        TriangleList  = GL_TRIANGLES,
        TriangleStrip = GL_TRIANGLE_STRIP,

        PatchList = GL_PATCHES
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

        Buffer       CreateBuffer(GLsizeiptr size, const void* data = nullptr);
        VertexBuffer CreateVertexBuffer(GLsizei stride, int64_t capacity, const void* data = nullptr);
        IndexBuffer  CreateIndexBuffer(IndexType, int64_t capacity, const void* data = nullptr);

        GLuint GetVertexArray(const VertexAttributes&);

        GLuint CreateGraphicsProgram(const std::string&                vertSource,
                                     const std::string&                fragSource,
                                     const std::optional<std::string>& geomSource = std::nullopt);

        GLuint CreateComputeProgram(const std::string& compSource);

        Texture CreateTexture2D(Extent2D extent, PixelFormat, uint32_t numMipLevels = 1u, uint32_t numLayers = 0u);
        Texture CreateTexture3D(Extent2D, uint32_t depth, PixelFormat);
        Texture CreateCubemap(uint32_t size, PixelFormat, uint32_t numMipLevels = 1u, uint32_t numLayers = 0u);

        RenderContext& GenerateMipmaps(Texture&);

        RenderContext& SetupSampler(Texture&, const SamplerInfo&);
        GLuint         CreateSampler(const SamplerInfo&);

        RenderContext& Clear(Texture&);
        // Upload Texture2D
        RenderContext& Upload(Texture&, GLint mipLevel, glm::uvec2 dimensions, const ImageData&);
        // Upload Cubemap face
        RenderContext& Upload(Texture&, GLint mipLevel, GLint face, glm::uvec2 dimensions, const ImageData&);
        RenderContext&
        Upload(Texture&, GLint mipLevel, const glm::uvec3& dimensions, GLint face, GLsizei layer, const ImageData&);

        RenderContext& Clear(Buffer&);
        RenderContext& Upload(Buffer&, GLintptr offset, GLsizeiptr size, const void* data);
        void*          Map(Buffer&);
        RenderContext& Unmap(Buffer&);

        RenderContext& Destroy(Buffer&);
        RenderContext& Destroy(Texture&);
        RenderContext& Destroy(GraphicsPipeline&);

        RenderContext& Dispatch(GLuint computeProgram, const glm::uvec3& numGroups);

        GLuint         BeginRendering(const RenderingInfo& info);
        RenderContext& BeginRendering(const Rect2D&            area,
                                      std::optional<glm::vec4> clearColor   = {},
                                      std::optional<float>     clearDepth   = {},
                                      std::optional<int>       clearStencil = {});
        RenderContext& EndRendering(GLuint frameBufferID);

        RenderContext& SetUniform1f(const std::string& name, float);
        RenderContext& SetUniform1i(const std::string& name, int32_t);
        RenderContext& SetUniform1ui(const std::string& name, uint32_t);

        RenderContext& SetUniformVec3(const std::string& name, const glm::vec3&);
        RenderContext& SetUniformVec4(const std::string& name, const glm::vec4&);

        RenderContext& SetUniformMat3(const std::string& name, const glm::mat3&);
        RenderContext& SetUniformMat4(const std::string& name, const glm::mat4&);

        RenderContext& BindGraphicsPipeline(const GraphicsPipeline& pipeline);
        RenderContext& BindImage(GLuint unit, const Texture&, GLint mipLevel, GLenum access);
        RenderContext& BindTexture(GLuint unit, const Texture&, std::optional<GLuint> samplerId = {});
        RenderContext& BindUniformBuffer(GLuint index, const UniformBuffer&);
        RenderContext& BindMaterial(Material*);

        RenderContext& DrawFullScreenTriangle();
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
        GLuint CreateVertexArray(const VertexAttributes&);

        Texture CreateImmutableTexture(Extent2D,
                                       uint32_t depth,
                                       PixelFormat,
                                       uint32_t numFaces,
                                       uint32_t numMipLevels,
                                       uint32_t numLayers);

        void CreateFaceView(Texture& cubeMap, GLuint mipLevel, GLuint layer, GLuint face);
        void AttachTexture(GLuint framebuffer, GLenum attachment, const AttachmentInfo&);

        GLuint CreateShaderProgram(std::initializer_list<GLuint> shaders);
        GLuint CreateShaderObject(GLenum type, const std::string& shaderSource);

        void SetShaderProgram(GLuint);
        void SetVertexArray(GLuint);
        void SetVertexBuffer(const VertexBuffer&);
        void SetIndexBuffer(const IndexBuffer&);

        void SetDepthTest(bool enabled, CompareOp);
        void SetDepthWrite(bool enabled);

        void SetPolygonMode(PolygonMode);
        void SetPolygonOffset(std::optional<PolygonOffset>);
        void SetCullMode(CullMode);
        void SetDepthClamp(bool enabled);
        void SetScissorTest(bool enabled);

        void SetBlendState(GLuint index, const BlendState&);

    private:
        bool             m_RenderingStarted = false;
        GraphicsPipeline m_CurrentPipeline;

        GLuint                                  m_DummyVAO {GL_NONE};
        std::unordered_map<std::size_t, GLuint> m_VertexArrays;
    };
} // namespace SnowLeopardEngine