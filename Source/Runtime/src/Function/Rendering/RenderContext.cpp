#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Buffer.h"
#include "SnowLeopardEngine/Function/Rendering/IndexBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/Material.h"
#include "SnowLeopardEngine/Function/Rendering/VertexBuffer.h"

namespace SnowLeopardEngine
{
    // @return {data type, number of components, normalize}
    std::tuple<GLenum, GLint, GLboolean> statAttribute(VertexAttribute::Type type)
    {
        switch (type)
        {
            using enum VertexAttribute::Type;
            case Float:
                return {GL_FLOAT, 1, GL_FALSE};
            case Float2:
                return {GL_FLOAT, 2, GL_FALSE};
            case Float3:
                return {GL_FLOAT, 3, GL_FALSE};
            case Float4:
                return {GL_FLOAT, 4, GL_FALSE};

            case Int:
                return {GL_INT, 1, GL_FALSE};
            case Int4:
                return {GL_INT, 4, GL_FALSE};

            case UByte4_Norm:
                return {GL_UNSIGNED_BYTE, 4, GL_TRUE};
        }
        return {GL_INVALID_INDEX, 0, GL_FALSE};
    }

    GLenum selectTextureMinFilter(TexelFilter minFilter, MipmapMode mipmapMode)
    {
        GLenum result {GL_NONE};
        switch (minFilter)
        {
            case TexelFilter::Nearest:
                switch (mipmapMode)
                {
                    case MipmapMode::None:
                        result = GL_NEAREST;
                        break;
                    case MipmapMode::Nearest:
                        result = GL_NEAREST_MIPMAP_NEAREST;
                        break;
                    case MipmapMode::Linear:
                        result = GL_NEAREST_MIPMAP_LINEAR;
                        break;
                }
                break;

            case TexelFilter::Linear:
                switch (mipmapMode)
                {
                    case MipmapMode::None:
                        result = GL_LINEAR;
                        break;
                    case MipmapMode::Nearest:
                        result = GL_LINEAR_MIPMAP_NEAREST;
                        break;
                    case MipmapMode::Linear:
                        result = GL_LINEAR_MIPMAP_LINEAR;
                        break;
                }
                break;
        }
        assert(result != GL_NONE);
        return result;
    }
    GLenum getIndexDataType(GLsizei stride)
    {
        switch (stride)
        {
            case sizeof(GLubyte):
                return GL_UNSIGNED_BYTE;
            case sizeof(GLushort):
                return GL_UNSIGNED_SHORT;
            case sizeof(GLuint):
                return GL_UNSIGNED_INT;
        }
        assert(false);
        return GL_NONE;
    }

    GLenum getPolygonOffsetCap(PolygonMode polygonMode)
    {
        switch (polygonMode)
        {
            case PolygonMode::Fill:
                return GL_POLYGON_OFFSET_FILL;
            case PolygonMode::Line:
                return GL_POLYGON_OFFSET_LINE;
            case PolygonMode::Point:
                return GL_POLYGON_OFFSET_POINT;
        }
        assert(false);
        return GL_NONE;
    }

    RenderContext::RenderContext() { glCreateVertexArrays(1, &m_DummyVAO); }

    RenderContext::~RenderContext()
    {
        glDeleteVertexArrays(1, &m_DummyVAO);
        for (auto [_, vao] : m_VertexArrays)
            glDeleteVertexArrays(1, &vao);

        m_CurrentPipeline = {};
    }

    RenderContext& RenderContext::SetViewport(const Rect2D& rect)
    {
        auto& current = m_CurrentPipeline.m_Viewport;
        if (rect != current)
        {
            glViewport(rect.Offset.X, rect.Offset.Y, rect.Extent.Width, rect.Extent.Height);
            current = rect;
        }
        return *this;
    }

    Rect2D RenderContext::GetViewport()
    {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        return {{viewport[0], viewport[1]}, {static_cast<uint32_t>(viewport[2]), static_cast<uint32_t>(viewport[3])}};
    }

    RenderContext& RenderContext::SetScissor(const Rect2D& rect)
    {
        auto& current = m_CurrentPipeline.m_Scissor;
        if (rect != current)
        {
            glScissor(rect.Offset.X, rect.Offset.Y, rect.Extent.Width, rect.Extent.Height);
            current = rect;
        }
        return *this;
    }

    Buffer RenderContext::CreateBuffer(GLsizeiptr size, const void* data)
    {
        GLuint buffer;
        glCreateBuffers(1, &buffer);
        glNamedBufferStorage(buffer, size, data, GL_DYNAMIC_STORAGE_BIT);

        return {buffer, size};
    }

    VertexBuffer RenderContext::CreateVertexBuffer(GLsizei stride, int64_t capacity, const void* data)
    {
        return VertexBuffer {CreateBuffer(stride * capacity, data), stride};
    }

    IndexBuffer RenderContext::CreateIndexBuffer(IndexType indexType, int64_t capacity, const void* data)
    {
        const auto stride = static_cast<GLsizei>(indexType);
        return IndexBuffer {CreateBuffer(stride * capacity, data), indexType};
    }

    GLuint RenderContext::GetVertexArray(const VertexAttributes& attributes)
    {
        assert(!attributes.empty());

        std::size_t hash {0};
        for (const auto& [location, attribute] : attributes)
            hashCombine(hash, location, attribute);

        auto it = m_VertexArrays.find(hash);
        if (it == m_VertexArrays.cend())
        {
            it = m_VertexArrays.emplace(hash, CreateVertexArray(attributes)).first;
            SNOW_LEOPARD_CORE_INFO("[RenderContext] Created VAO: {0}", hash);
        }

        return it->second;
    }

    GLuint RenderContext::CreateGraphicsProgram(const std::string&                vertSource,
                                                const std::string&                fragSource,
                                                const std::optional<std::string>& geomSource)
    {
        return CreateShaderProgram({
            CreateShaderObject(GL_VERTEX_SHADER, vertSource),
            geomSource ? CreateShaderObject(GL_GEOMETRY_SHADER, *geomSource) : GL_NONE,
            CreateShaderObject(GL_FRAGMENT_SHADER, fragSource),
        });
    }

    GLuint RenderContext::CreateComputeProgram(const std::string& compSource)
    {
        return CreateShaderProgram({
            CreateShaderObject(GL_COMPUTE_SHADER, compSource),
        });
    }

    Texture
    RenderContext::CreateTexture2D(Extent2D extent, PixelFormat pixelFormat, uint32_t numMipLevels, uint32_t numLayers)
    {
        assert(extent.Width > 0 && extent.Height > 0 && pixelFormat != PixelFormat::Unknown);

        if (numMipLevels <= 0)
            numMipLevels = calcMipLevels(glm::max(extent.Width, extent.Height));

        return CreateImmutableTexture(extent, 0, pixelFormat, 1, numMipLevels, numLayers);
    }

    Texture RenderContext::CreateTexture3D(Extent2D extent, uint32_t depth, PixelFormat pixelFormat)
    {
        return CreateImmutableTexture(extent, depth, pixelFormat, 1, 1, 0);
    }

    Texture
    RenderContext::CreateCubemap(uint32_t size, PixelFormat pixelFormat, uint32_t numMipLevels, uint32_t numLayers)
    {
        assert(size > 0 && pixelFormat != PixelFormat::Unknown);

        if (numMipLevels <= 0)
            numMipLevels = calcMipLevels(size);

        return CreateImmutableTexture({size, size}, 0, pixelFormat, 6, numMipLevels, numLayers);
    }

    RenderContext& RenderContext::GenerateMipmaps(Texture& texture)
    {
        assert(texture);
        glGenerateTextureMipmap(texture.m_Id);

        return *this;
    }

    RenderContext& RenderContext::SetupSampler(Texture& texture, const SamplerInfo& samplerInfo)
    {
        assert(texture);

        glTextureParameteri(
            texture.m_Id, GL_TEXTURE_MIN_FILTER, selectTextureMinFilter(samplerInfo.MinFilter, samplerInfo.MipmapMode));
        glTextureParameteri(texture.m_Id, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(samplerInfo.MagFilter));
        glTextureParameteri(texture.m_Id, GL_TEXTURE_WRAP_S, static_cast<GLenum>(samplerInfo.AddressModeS));
        glTextureParameteri(texture.m_Id, GL_TEXTURE_WRAP_T, static_cast<GLenum>(samplerInfo.AddressModeT));
        glTextureParameteri(texture.m_Id, GL_TEXTURE_WRAP_R, static_cast<GLenum>(samplerInfo.AddressModeR));

        glTextureParameterf(texture.m_Id, GL_TEXTURE_MAX_ANISOTROPY, samplerInfo.MaxAnisotropy);

        if (samplerInfo.CompareOperator.has_value())
        {
            glTextureParameteri(texture.m_Id, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTextureParameteri(
                texture.m_Id, GL_TEXTURE_COMPARE_FUNC, static_cast<GLenum>(*samplerInfo.CompareOperator));
        }
        glTextureParameterfv(texture.m_Id, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(samplerInfo.BorderColor));

        return *this;
    }

    GLuint RenderContext::CreateSampler(const SamplerInfo& samplerInfo)
    {
        GLuint sampler {GL_NONE};
        glCreateSamplers(1, &sampler);

        glSamplerParameteri(
            sampler, GL_TEXTURE_MIN_FILTER, selectTextureMinFilter(samplerInfo.MinFilter, samplerInfo.MipmapMode));
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(samplerInfo.MagFilter));
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, static_cast<GLenum>(samplerInfo.AddressModeS));
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, static_cast<GLenum>(samplerInfo.AddressModeT));
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, static_cast<GLenum>(samplerInfo.AddressModeR));

        glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY, samplerInfo.MaxAnisotropy);

        if (samplerInfo.CompareOperator.has_value())
        {
            glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC, static_cast<GLenum>(*samplerInfo.CompareOperator));
        }
        glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(samplerInfo.BorderColor));

        return sampler;
    }

    RenderContext& RenderContext::Clear(Texture& texture)
    {
        assert(texture);
        uint8_t v {0};
        glClearTexImage(texture.m_Id, 0, GL_RED, GL_UNSIGNED_BYTE, &v);

        return *this;
    }

    RenderContext&
    RenderContext::Upload(Texture& texture, GLint mipLevel, glm::uvec2 dimensions, const ImageData& image)
    {
        return Upload(texture, mipLevel, {dimensions, 0}, 0, 0, image);
    }

    RenderContext&
    RenderContext::Upload(Texture& texture, GLint mipLevel, GLint face, glm::uvec2 dimensions, const ImageData& image)
    {
        return Upload(texture, mipLevel, {dimensions, 0}, face, 0, image);
    }

    RenderContext& RenderContext::Upload(Texture&          texture,
                                         GLint             mipLevel,
                                         const glm::uvec3& dimensions,
                                         GLint             face,
                                         GLsizei           layer,
                                         const ImageData&  image)
    {
        assert(texture && image.Pixels != nullptr);

        switch (texture.m_Type)
        {
            case GL_TEXTURE_1D:
                glTextureSubImage1D(
                    texture.m_Id, mipLevel, 0, dimensions.x, image.Format, image.DataType, image.Pixels);
                break;
            case GL_TEXTURE_1D_ARRAY:
                glTextureSubImage2D(
                    texture.m_Id, mipLevel, 0, 0, dimensions.x, layer, image.Format, image.DataType, image.Pixels);
                break;
            case GL_TEXTURE_2D:
                glTextureSubImage2D(texture.m_Id,
                                    mipLevel,
                                    0,
                                    0,
                                    dimensions.x,
                                    dimensions.y,
                                    image.Format,
                                    image.DataType,
                                    image.Pixels);
                break;
            case GL_TEXTURE_2D_ARRAY:
                glTextureSubImage3D(texture.m_Id,
                                    mipLevel,
                                    0,
                                    0,
                                    layer,
                                    dimensions.x,
                                    dimensions.y,
                                    1,
                                    image.Format,
                                    image.DataType,
                                    image.Pixels);
                break;
            case GL_TEXTURE_3D:
                glTextureSubImage3D(texture.m_Id,
                                    mipLevel,
                                    0,
                                    0,
                                    0,
                                    dimensions.x,
                                    dimensions.y,
                                    dimensions.z,
                                    image.Format,
                                    image.DataType,
                                    image.Pixels);
                break;
            case GL_TEXTURE_CUBE_MAP:
                glTextureSubImage3D(texture.m_Id,
                                    mipLevel,
                                    0,
                                    0,
                                    face,
                                    dimensions.x,
                                    dimensions.y,
                                    1,
                                    image.Format,
                                    image.DataType,
                                    image.Pixels);
                break;
            case GL_TEXTURE_CUBE_MAP_ARRAY: {
                const auto zoffset = (layer * 6) + face; // desired layer-face
                // depth = how many layer-faces
                glTextureSubImage3D(texture.m_Id,
                                    mipLevel,
                                    0,
                                    0,
                                    zoffset,
                                    dimensions.x,
                                    dimensions.y,
                                    6 * texture.m_NumLayers,
                                    image.Format,
                                    image.DataType,
                                    image.Pixels);
            }
            break;

            default:
                assert(false);
        }
        return *this;
    }

    RenderContext& RenderContext::Clear(Buffer& buffer)
    {
        assert(buffer);

        uint8_t v {0};
        glClearNamedBufferData(buffer.m_Id, GL_R8, GL_RED, GL_UNSIGNED_BYTE, &v);

        return *this;
    }

    RenderContext& RenderContext::Upload(Buffer& buffer, GLintptr offset, GLsizeiptr size, const void* data)
    {
        assert(buffer);

        if (size > 0 && data != nullptr)
            glNamedBufferSubData(buffer.m_Id, offset, size, data);

        return *this;
    }

    void* RenderContext::Map(Buffer& buffer)
    {
        assert(buffer);

        if (!buffer.IsMapped())
            buffer.m_MappedMemory = glMapNamedBuffer(buffer.m_Id, GL_WRITE_ONLY);

        return buffer.m_MappedMemory;
    }

    RenderContext& RenderContext::Unmap(Buffer& buffer)
    {
        assert(buffer);

        if (buffer.IsMapped())
        {
            glUnmapNamedBuffer(buffer.m_Id);
            buffer.m_MappedMemory = nullptr;
        }

        return *this;
    }

    RenderContext& RenderContext::Destroy(Buffer& buffer)
    {
        if (buffer)
        {
            glDeleteBuffers(1, &buffer.m_Id);
            buffer = {};
        }

        return *this;
    }

    RenderContext& RenderContext::Destroy(Texture& texture)
    {
        if (texture)
        {
            glDeleteTextures(1, &texture.m_Id);
            if (texture.m_View != GL_NONE)
                glDeleteTextures(1, &texture.m_View);
            texture = {};
        }
        return *this;
    }

    RenderContext& RenderContext::Destroy(GraphicsPipeline& gp)
    {
        if (gp.m_Program != GL_NONE)
        {
            glDeleteProgram(gp.m_Program);
            gp.m_Program = GL_NONE;
        }
        gp.m_VAO = GL_NONE;

        return *this;
    }

    RenderContext& RenderContext::Dispatch(GLuint computeProgram, const glm::uvec3& numGroups)
    {
        SetShaderProgram(computeProgram);
        glDispatchCompute(numGroups.x, numGroups.y, numGroups.z);

        return *this;
    }

    GLuint RenderContext::BeginRendering(const RenderingInfo& renderingInfo)
    {
        SNOW_LEOPARD_CORE_ASSERT(!m_RenderingStarted,
                                 "[RenderContext] Failed to begin rendering, already in rendering.");

        GLuint framebuffer;
        glCreateFramebuffers(1, &framebuffer);
        if (renderingInfo.DepthAttachment.has_value())
        {
            AttachTexture(framebuffer, GL_DEPTH_ATTACHMENT, *renderingInfo.DepthAttachment);
        }
        for (size_t i {0}; i < renderingInfo.ColorAttachments.size(); ++i)
        {
            AttachTexture(framebuffer, GL_COLOR_ATTACHMENT0 + i, renderingInfo.ColorAttachments[i]);
        }
        if (const auto n = renderingInfo.ColorAttachments.size(); n > 0)
        {
            std::vector<GLenum> colorBuffers(n);
            std::iota(colorBuffers.begin(), colorBuffers.end(), GL_COLOR_ATTACHMENT0);
            glNamedFramebufferDrawBuffers(framebuffer, colorBuffers.size(), colorBuffers.data());
        }
#ifdef _DEBUG
        const auto status = glCheckNamedFramebufferStatus(framebuffer, GL_DRAW_FRAMEBUFFER);
        assert(GL_FRAMEBUFFER_COMPLETE == status);
#endif

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
        SetViewport(renderingInfo.Area);
        SetScissorTest(false);

        if (renderingInfo.DepthAttachment.has_value())
            if (renderingInfo.DepthAttachment->ClearValue.has_value())
            {
                SetDepthWrite(true);

                const auto clearValue = std::get<float>(*renderingInfo.DepthAttachment->ClearValue);
                glClearNamedFramebufferfv(framebuffer, GL_DEPTH, 0, &clearValue);
            }
        for (int32_t i {0}; const auto& attachment : renderingInfo.ColorAttachments)
        {
            if (attachment.ClearValue.has_value())
            {
                const auto& clearValue = std::get<glm::vec4>(*attachment.ClearValue);
                glClearNamedFramebufferfv(framebuffer, GL_COLOR, i, glm::value_ptr(clearValue));
            }
            ++i;
        }

        m_RenderingStarted = true;

        return framebuffer;
    }

    RenderContext& RenderContext::BeginRendering(const Rect2D&            area,
                                                 std::optional<glm::vec4> clearColor,
                                                 std::optional<float>     clearDepth,
                                                 std::optional<int>       clearStencil)
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GL_NONE);
        SetViewport(area);
        SetScissorTest(false);

        if (clearDepth.has_value())
        {
            SetDepthWrite(true);
            glClearNamedFramebufferfv(GL_NONE, GL_DEPTH, 0, &clearDepth.value());
        }
        if (clearStencil.has_value())
            glClearNamedFramebufferiv(GL_NONE, GL_STENCIL, 0, &clearStencil.value());
        if (clearColor.has_value())
        {
            glClearNamedFramebufferfv(GL_NONE, GL_COLOR, 0, glm::value_ptr(clearColor.value()));
        }

        return *this;
    }

    RenderContext& RenderContext::EndRendering(GLuint frameBufferID)
    {
        assert(m_RenderingStarted && frameBufferID != GL_NONE);

        glDeleteFramebuffers(1, &frameBufferID);
        m_RenderingStarted = false;

        return *this;
    }

    RenderContext& RenderContext::BindGraphicsPipeline(const GraphicsPipeline& gp)
    {
        {
            const auto& state = gp.m_DepthStencilState;
            SetDepthTest(state.DepthTest, state.DepthCompareOp);
            SetDepthWrite(state.DepthWrite);
        }

        {
            const auto& state = gp.m_RasterizerState;
            SetPolygonMode(state.PolygonMode);
            SetCullMode(state.CullMode);
            SetPolygonOffset(state.PolygonOffset);
            SetDepthClamp(state.DepthClampEnable);
            SetScissorTest(state.ScissorTest);
        }

        for (int32_t i {0}; i < gp.m_BlendStates.size(); ++i)
            SetBlendState(i, gp.m_BlendStates[i]);

        SetVertexArray(gp.m_VAO);
        SetShaderProgram(gp.m_Program);

        return *this;
    }

    RenderContext& RenderContext::SetUniform1f(const std::string& name, float f)
    {
        const auto location = glGetUniformLocation(m_CurrentPipeline.m_Program, name.data());
        if (location != GL_INVALID_INDEX)
            glProgramUniform1f(m_CurrentPipeline.m_Program, location, f);
        return *this;
    }

    RenderContext& RenderContext::SetUniform1i(const std::string& name, int32_t i)
    {
        const auto location = glGetUniformLocation(m_CurrentPipeline.m_Program, name.data());
        if (location != GL_INVALID_INDEX)
            glProgramUniform1i(m_CurrentPipeline.m_Program, location, i);
        return *this;
    }

    RenderContext& RenderContext::SetUniform1ui(const std::string& name, uint32_t i)
    {
        const auto location = glGetUniformLocation(m_CurrentPipeline.m_Program, name.data());
        if (location != GL_INVALID_INDEX)
            glProgramUniform1ui(m_CurrentPipeline.m_Program, location, i);
        return *this;
    }

    RenderContext& RenderContext::SetUniformVec3(const std::string& name, const glm::vec3& v)
    {
        const auto location = glGetUniformLocation(m_CurrentPipeline.m_Program, name.data());
        if (location != GL_INVALID_INDEX)
        {
            glProgramUniform3fv(m_CurrentPipeline.m_Program, location, 1, glm::value_ptr(v));
        }
        return *this;
    }

    RenderContext& RenderContext::SetUniformVec4(const std::string& name, const glm::vec4& v)
    {
        const auto location = glGetUniformLocation(m_CurrentPipeline.m_Program, name.data());
        if (location != GL_INVALID_INDEX)
        {
            glProgramUniform4fv(m_CurrentPipeline.m_Program, location, 1, glm::value_ptr(v));
        }
        return *this;
    }

    RenderContext& RenderContext::SetUniformMat3(const std::string& name, const glm::mat3& m)
    {
        const auto location = glGetUniformLocation(m_CurrentPipeline.m_Program, name.data());
        if (location != GL_INVALID_INDEX)
        {
            glProgramUniformMatrix3fv(m_CurrentPipeline.m_Program, location, 1, GL_FALSE, glm::value_ptr(m));
        }
        return *this;
    }

    RenderContext& RenderContext::SetUniformMat4(const std::string& name, const glm::mat4& m)
    {
        const auto location = glGetUniformLocation(m_CurrentPipeline.m_Program, name.data());
        if (location != GL_INVALID_INDEX)
        {
            glProgramUniformMatrix4fv(m_CurrentPipeline.m_Program, location, 1, GL_FALSE, glm::value_ptr(m));
        }
        return *this;
    }

    RenderContext& RenderContext::BindImage(GLuint unit, const Texture& texture, GLint mipLevel, GLenum access)
    {
        assert(texture && mipLevel < texture.m_NumMipLevels);
        glBindImageTexture(
            unit, texture.m_Id, mipLevel, GL_FALSE, 0, access, static_cast<GLenum>(texture.m_PixelFormat));
        return *this;
    }

    RenderContext& RenderContext::BindTexture(GLuint unit, const Texture& texture, std::optional<GLuint> samplerId)
    {
        assert(texture);
        glBindTextureUnit(unit, texture.m_Id);
        if (samplerId.has_value())
            glBindSampler(unit, *samplerId);
        return *this;
    }

    RenderContext& RenderContext::BindUniformBuffer(GLuint index, const UniformBuffer& buffer)
    {
        assert(buffer);
        glBindBufferBase(GL_UNIFORM_BUFFER, index, buffer.m_Id);
        return *this;
    }

    RenderContext& RenderContext::BindMaterial(Material* material)
    {
        assert(material);
        material->AutoBind(*this);
        return *this;
    }

    RenderContext& RenderContext::DrawFullScreenTriangle() { return Draw({}, {}, 0, 3); }

    RenderContext& RenderContext::DrawCube() { return Draw({}, {}, 0, 36); }

    RenderContext& RenderContext::Draw(OptionalReference<const VertexBuffer> vertexBuffer,
                                       OptionalReference<const IndexBuffer>  indexBuffer,
                                       uint32_t                              numIndices,
                                       uint32_t                              numVertices,
                                       uint32_t                              numInstances)
    {
        if (vertexBuffer.has_value())
            SetVertexBuffer(*vertexBuffer);

        if (numIndices > 0)
        {
            assert(indexBuffer.has_value());
            SetIndexBuffer(*indexBuffer);
            glDrawElementsInstanced(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, nullptr, numInstances);
        }
        else
        {
            glDrawArraysInstanced(GL_TRIANGLES, 0, numVertices, numInstances);
        }
        return *this;
    }

    GLuint RenderContext::CreateVertexArray(const VertexAttributes& attributes)
    {
        GLuint vao;
        glCreateVertexArrays(1, &vao);

        for (const auto& [location, attribute] : attributes)
        {
            const auto [type, size, normalized] = statAttribute(attribute.VertType);
            assert(type != GL_INVALID_INDEX);

            glEnableVertexArrayAttrib(vao, location);
            if (attribute.VertType == VertexAttribute::Type::Int4)
            {
                glVertexArrayAttribIFormat(vao, location, size, type, attribute.Offset);
            }
            else
            {
                glVertexArrayAttribFormat(vao, location, size, type, normalized, attribute.Offset);
            }
            glVertexArrayAttribBinding(vao, location, 0);
        }
        return vao;
    }

    Texture RenderContext::CreateImmutableTexture(Extent2D    extent,
                                                  uint32_t    depth,
                                                  PixelFormat pixelFormat,
                                                  uint32_t    numFaces,
                                                  uint32_t    numMipLevels,
                                                  uint32_t    numLayers)
    {
        assert(numMipLevels > 0);

        // http://github.khronos.org/KTX-Specification/#_texture_type

        GLenum target = numFaces == 6     ? GL_TEXTURE_CUBE_MAP :
                        depth > 0         ? GL_TEXTURE_3D :
                        extent.Height > 0 ? GL_TEXTURE_2D :
                                            GL_TEXTURE_1D;
        assert(target == GL_TEXTURE_CUBE_MAP ? extent.Width == extent.Height : true);

        if (numLayers > 0)
        {
            switch (target)
            {
                case GL_TEXTURE_1D:
                    target = GL_TEXTURE_1D_ARRAY;
                    break;
                case GL_TEXTURE_2D:
                    target = GL_TEXTURE_2D_ARRAY;
                    break;
                case GL_TEXTURE_CUBE_MAP:
                    target = GL_TEXTURE_CUBE_MAP_ARRAY;
                    break;

                default:
                    assert(false);
            }
        }

        GLuint id {GL_NONE};
        glCreateTextures(target, 1, &id);

        const auto internalFormat = static_cast<GLenum>(pixelFormat);
        switch (target)
        {
            case GL_TEXTURE_1D:
                glTextureStorage1D(id, numMipLevels, internalFormat, extent.Width);
                break;
            case GL_TEXTURE_1D_ARRAY:
                glTextureStorage2D(id, numMipLevels, internalFormat, extent.Width, numLayers);
                break;

            case GL_TEXTURE_2D:
                glTextureStorage2D(id, numMipLevels, internalFormat, extent.Width, extent.Height);
                break;
            case GL_TEXTURE_2D_ARRAY:
                glTextureStorage3D(id, numMipLevels, internalFormat, extent.Width, extent.Height, numLayers);
                break;

            case GL_TEXTURE_3D:
                glTextureStorage3D(id, numMipLevels, internalFormat, extent.Width, extent.Height, depth);
                break;

            case GL_TEXTURE_CUBE_MAP:
                glTextureStorage2D(id, numMipLevels, internalFormat, extent.Width, extent.Height);
                break;
            case GL_TEXTURE_CUBE_MAP_ARRAY:
                glTextureStorage3D(id, numMipLevels, internalFormat, extent.Width, extent.Height, numLayers * 6);
                break;
        }

        return Texture {
            id,
            target,
            pixelFormat,
            extent,
            depth,
            numMipLevels,
            numLayers,
        };
    }

    void RenderContext::CreateFaceView(Texture& cubeMap, GLuint mipLevel, GLuint layer, GLuint face)
    {
        assert(cubeMap.m_Type == GL_TEXTURE_CUBE_MAP || cubeMap.m_Type == GL_TEXTURE_CUBE_MAP_ARRAY);

        if (cubeMap.m_View != GL_NONE)
            glDeleteTextures(1, &cubeMap.m_View);
        glGenTextures(1, &cubeMap.m_View);

        glTextureView(cubeMap.m_View,
                      GL_TEXTURE_2D,
                      cubeMap.m_Id,
                      static_cast<GLenum>(cubeMap.m_PixelFormat),
                      mipLevel,
                      1,
                      (layer * 6) + face,
                      1);
    }

    void RenderContext::AttachTexture(GLuint framebuffer, GLenum attachment, const AttachmentInfo& info)
    {
        const auto& [image, mipLevel, maybeLayer, maybeFace, _] = info;

        switch (image.m_Type)
        {
            case GL_TEXTURE_CUBE_MAP:
            case GL_TEXTURE_CUBE_MAP_ARRAY:
                CreateFaceView(image, mipLevel, maybeLayer.value_or(0), maybeFace.value_or(0));
                glNamedFramebufferTexture(framebuffer, attachment, image.m_View, 0);
                break;

            case GL_TEXTURE_2D:
                glNamedFramebufferTexture(framebuffer, attachment, image.m_Id, mipLevel);
                break;

            case GL_TEXTURE_2D_ARRAY:
                assert(maybeLayer.has_value());
                glNamedFramebufferTextureLayer(framebuffer, attachment, image.m_Id, mipLevel, maybeLayer.value_or(0));
                break;

            case GL_TEXTURE_3D:
                glNamedFramebufferTexture(framebuffer, attachment, image.m_Id, 0);
                break;

            default:
                assert(false);
        }
    }

    GLuint RenderContext::CreateShaderProgram(std::initializer_list<GLuint> shaders)
    {
        const auto program = glCreateProgram();

        for (auto shader : shaders)
            if (shader != GL_NONE)
                glAttachShader(program, shader);

        glLinkProgram(program);

        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (GL_FALSE == status)
        {
            GLint infoLogLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
            assert(infoLogLength > 0);
            std::string infoLog("", infoLogLength);
            glGetProgramInfoLog(program, infoLogLength, nullptr, infoLog.data());
            throw std::runtime_error {infoLog};
        }
        for (auto shader : shaders)
        {
            if (shader != GL_NONE)
            {
                glDetachShader(program, shader);
                glDeleteShader(shader);
            }
        }

        return program;
    }

    GLuint RenderContext::CreateShaderObject(GLenum type, const std::string& shaderSource)
    {
        auto          id = glCreateShader(type);
        const GLchar* strings {shaderSource.data()};
        glShaderSource(id, 1, &strings, nullptr);
        glCompileShader(id);

        GLint status;
        glGetShaderiv(id, GL_COMPILE_STATUS, &status);
        if (GL_FALSE == status)
        {
            GLint infoLogLength;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
            assert(infoLogLength > 0);
            std::string infoLog("", infoLogLength);
            glGetShaderInfoLog(id, infoLogLength, nullptr, infoLog.data());
            throw std::runtime_error {infoLog};
        };

        return id;
    }

    void RenderContext::SetShaderProgram(GLuint program)
    {
        assert(program != GL_NONE);
        if (auto& current = m_CurrentPipeline.m_Program; current != program)
        {
            glUseProgram(program);
            current = program;
        }
    }

    void RenderContext::SetVertexArray(GLuint vao)
    {
        if (GL_NONE == vao)
            vao = m_DummyVAO;

        if (auto& current = m_CurrentPipeline.m_VAO; vao != current)
        {
            glBindVertexArray(vao);
            current = vao;
        }
    }

    void RenderContext::SetVertexBuffer(const VertexBuffer& vertexBuffer)
    {
        const auto vao = m_CurrentPipeline.m_VAO;
        assert(vertexBuffer && vao != GL_NONE);
        glVertexArrayVertexBuffer(vao, 0, vertexBuffer.m_Id, 0, vertexBuffer.GetStride());
    }

    void RenderContext::SetIndexBuffer(const IndexBuffer& indexBuffer)
    {
        const auto vao = m_CurrentPipeline.m_VAO;
        assert(indexBuffer && vao != GL_NONE);
        glVertexArrayElementBuffer(vao, indexBuffer.m_Id);
    }

    void RenderContext::SetDepthTest(bool enabled, CompareOp depthFunc)
    {
        auto& current = m_CurrentPipeline.m_DepthStencilState;
        if (enabled != current.DepthTest)
        {
            enabled ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
            current.DepthTest = enabled;
        }
        if (enabled && depthFunc != current.DepthCompareOp)
        {
            glDepthFunc(static_cast<GLenum>(depthFunc));
            current.DepthCompareOp = depthFunc;
        }
    }

    void RenderContext::SetDepthWrite(bool enabled)
    {
        auto& current = m_CurrentPipeline.m_DepthStencilState;
        if (enabled != current.DepthWrite)
        {
            glDepthMask(enabled);
            current.DepthWrite = enabled;
        }
    }

    void RenderContext::SetPolygonMode(PolygonMode polygonMode)
    {
        auto& current = m_CurrentPipeline.m_RasterizerState.PolygonMode;
        if (polygonMode != current)
        {
            glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(polygonMode));
            current = polygonMode;
        }
    }

    void RenderContext::SetPolygonOffset(std::optional<PolygonOffset> polygonOffset)
    {
        auto& current = m_CurrentPipeline.m_RasterizerState;
        if (polygonOffset != current.PolygonOffset)
        {
            const auto offsetCap = getPolygonOffsetCap(current.PolygonMode);
            if (polygonOffset.has_value())
            {
                glEnable(offsetCap);
                glPolygonOffset(polygonOffset->Factor, polygonOffset->Units);
            }
            else
            {
                glDisable(offsetCap);
            }
            current.PolygonOffset = polygonOffset;
        }
    }

    void RenderContext::SetCullMode(CullMode cullMode)
    {
        auto& current = m_CurrentPipeline.m_RasterizerState.CullMode;
        if (cullMode != current)
        {
            if (cullMode != CullMode::None)
            {
                if (current == CullMode::None)
                    glEnable(GL_CULL_FACE);
                glCullFace(static_cast<GLenum>(cullMode));
            }
            else
            {
                glDisable(GL_CULL_FACE);
            }
            current = cullMode;
        }
    }

    void RenderContext::SetDepthClamp(bool enabled)
    {
        auto& current = m_CurrentPipeline.m_RasterizerState.DepthClampEnable;
        if (enabled != current)
        {
            enabled ? glEnable(GL_DEPTH_CLAMP) : glDisable(GL_DEPTH_CLAMP);
            current = enabled;
        }
    }

    void RenderContext::SetScissorTest(bool enabled)
    {
        auto& current = m_CurrentPipeline.m_RasterizerState.ScissorTest;
        if (enabled != current)
        {
            enabled ? glEnable(GL_SCISSOR_TEST) : glDisable(GL_SCISSOR_TEST);
            current = enabled;
        }
    }

    void RenderContext::SetBlendState(GLuint index, const BlendState& state)
    {
        auto& current = m_CurrentPipeline.m_BlendStates[index];
        if (state != current)
        {
            if (state.Enabled != current.Enabled)
            {
                state.Enabled ? glEnablei(GL_BLEND, index) : glDisablei(GL_BLEND, index);
                current.Enabled = state.Enabled;
            }
            if (state.Enabled)
            {
                if (state.ColorOp != current.ColorOp || state.AlphaOp != current.AlphaOp)
                {
                    glBlendEquationSeparatei(
                        index, static_cast<GLenum>(state.ColorOp), static_cast<GLenum>(state.AlphaOp));

                    current.ColorOp = state.ColorOp;
                    current.AlphaOp = state.AlphaOp;
                }
                if (state.SrcColor != current.SrcColor || state.DestColor != current.DestColor ||
                    state.SrcAlpha != current.SrcAlpha || state.DestAlpha != current.DestAlpha)
                {
                    glBlendFuncSeparatei(index,
                                         static_cast<GLenum>(state.SrcColor),
                                         static_cast<GLenum>(state.DestColor),
                                         static_cast<GLenum>(state.SrcAlpha),
                                         static_cast<GLenum>(state.DestAlpha));

                    current.SrcColor  = state.SrcColor;
                    current.DestColor = state.DestColor;
                    current.SrcAlpha  = state.SrcAlpha;
                    current.DestAlpha = state.DestAlpha;
                }
            }
        }
    }

    DebugMarker::DebugMarker(std::string_view name)
    {
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name.data());
    }
    DebugMarker::~DebugMarker() { glPopDebugGroup(); }
} // namespace SnowLeopardEngine