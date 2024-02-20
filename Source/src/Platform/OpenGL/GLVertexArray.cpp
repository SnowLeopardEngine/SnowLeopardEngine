#include "SnowLeopardEngine/Platform/OpenGL/GLVertexArray.h"

#include "SnowLeopardEngine/Platform/OpenGL/GLIndexBuffer.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLVertexBuffer.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    static GLenum ShaderDataTypeToGLBaseType(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Float:
                return GL_FLOAT;
            case ShaderDataType::Float2:
                return GL_FLOAT;
            case ShaderDataType::Float3:
                return GL_FLOAT;
            case ShaderDataType::Float4:
                return GL_FLOAT;
            case ShaderDataType::Mat3:
                return GL_FLOAT;
            case ShaderDataType::Mat4:
                return GL_FLOAT;
            case ShaderDataType::Int:
                return GL_INT;
            case ShaderDataType::Int2:
                return GL_INT;
            case ShaderDataType::Int3:
                return GL_INT;
            case ShaderDataType::Int4:
                return GL_INT;
            case ShaderDataType::Bool:
                return GL_BOOL;
            case ShaderDataType::Unknown:
                break;
        }

        SNOW_LEOPARD_CORE_ASSERT(false, "[Rendering][InputAssembling] Unknown ShaderDataType!");
        return 0;
    }

    GLVertexArray::GLVertexArray() { glCreateVertexArrays(1, &m_VertexArrayName); }

    GLVertexArray::~GLVertexArray() { glDeleteVertexArrays(1, &m_VertexArrayName); }

    void GLVertexArray::Bind() const { glBindVertexArray(m_VertexArrayName); }

    void GLVertexArray::Unbind() const { glBindVertexArray(0); }

    void GLVertexArray::AddBuffers(const Ref<VertexBuffer>& vertexBuffer, const Ref<IndexBuffer>& indexBuffer)
    {
        SNOW_LEOPARD_CORE_ASSERT(!vertexBuffer->GetLayout().GetElements().empty(),
                                 "[Rendering][InputAssembling] Vertex Buffer has no layout!");

        glVertexArrayVertexBuffer(m_VertexArrayName,
                                  0,
                                  std::dynamic_pointer_cast<GLVertexBuffer>(vertexBuffer)->GetName(),
                                  0,
                                  vertexBuffer->GetLayout().GetStride());
        glVertexArrayElementBuffer(m_VertexArrayName, std::dynamic_pointer_cast<GLIndexBuffer>(indexBuffer)->GetName());

        auto layout = vertexBuffer->GetLayout();
        for (const auto& element : layout)
        {
            switch (element.Type)
            {
                case ShaderDataType::Float:
                case ShaderDataType::Float2:
                case ShaderDataType::Float3:
                case ShaderDataType::Float4: {
                    glEnableVertexArrayAttrib(m_VertexArrayName, m_VertexBufferIndex);
                    glVertexArrayAttribFormat(m_VertexArrayName,
                                              m_VertexBufferIndex,
                                              element.GetComponentCount(),
                                              ShaderDataTypeToGLBaseType(element.Type),
                                              element.Normalized ? GL_TRUE : GL_FALSE,
                                              element.Offset);
                    m_VertexBufferIndex++;
                    break;
                }
                case ShaderDataType::Int:
                case ShaderDataType::Int2:
                case ShaderDataType::Int3:
                case ShaderDataType::Int4:
                case ShaderDataType::Bool: {
                    glEnableVertexArrayAttrib(m_VertexArrayName, m_VertexBufferIndex);
                    glVertexArrayAttribIFormat(m_VertexArrayName,
                                               m_VertexBufferIndex,
                                               element.GetComponentCount(),
                                               ShaderDataTypeToGLBaseType(element.Type),
                                               element.Offset);
                    m_VertexBufferIndex++;
                    break;
                }
                case ShaderDataType::Mat3:
                case ShaderDataType::Mat4: {
                    uint8_t count = element.GetComponentCount();
                    for (uint8_t i = 0; i < count; i++)
                    {
                        glEnableVertexArrayAttrib(m_VertexArrayName, m_VertexBufferIndex);
                        glVertexArrayAttribFormat(m_VertexArrayName,
                                                  m_VertexBufferIndex,
                                                  element.GetComponentCount(),
                                                  ShaderDataTypeToGLBaseType(element.Type),
                                                  element.Normalized ? GL_TRUE : GL_FALSE,
                                                  element.Offset + sizeof(float) * count * i);
                        glVertexAttribDivisor(m_VertexBufferIndex, 1);
                        m_VertexBufferIndex++;
                    }
                    break;
                }
                default:
                    SNOW_LEOPARD_CORE_ASSERT(false, "[Rendering][InputAssembling] Unknown ShaderDataType!");
            }
        }

        m_VertexBuffers.push_back(vertexBuffer);
        m_IndexBuffer = indexBuffer;
    }
} // namespace SnowLeopardEngine