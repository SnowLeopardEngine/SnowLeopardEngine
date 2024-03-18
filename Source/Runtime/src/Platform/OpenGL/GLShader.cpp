#include "SnowLeopardEngine/Platform/OpenGL/GLShader.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    GLShader::GLShader(const std::string& vertexSource, const std::string& fragmentSource)
    {
        const char* vShaderCode = vertexSource.c_str();
        const char* fShaderCode = fragmentSource.c_str();

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
        glCompileShader(vertexShader);

        GLint isCompiled = 0;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(vertexShader);

            SNOW_LEOPARD_CORE_ERROR("{0}", infoLog.data());
            SNOW_LEOPARD_CORE_ASSERT(false, "Failed to compile vertex shader!");
            return;
        }

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(fragmentShader);
            glDeleteShader(vertexShader);

            SNOW_LEOPARD_CORE_ERROR("{0}", infoLog.data());
            SNOW_LEOPARD_CORE_ASSERT(false, "Failed to compile fragment shader!");
            return;
        }

        m_ProgramID    = glCreateProgram();
        GLuint program = m_ProgramID;

        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, static_cast<int*>(&isLinked));
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            SNOW_LEOPARD_CORE_ERROR("{0}", infoLog.data());
            SNOW_LEOPARD_CORE_ASSERT(false, "Failed to link shader!");
            return;
        }

        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
    }

    GLShader::GLShader(const std::unordered_map<std::string, std::string>& shaderStageSources)
    {
        bool hasComputeStage  = shaderStageSources.count("compute") > 0;
        bool hasVertexStage   = shaderStageSources.count("vertex") > 0;
        bool hasFragmentStage = shaderStageSources.count("fragment") > 0;
        bool hasGeometryStage = shaderStageSources.count("geometry") > 0;

        // if has compute stage, only care about compute stage, ignore other stages,
        // because compute shader is a special shader.
        if (hasComputeStage)
        {
            // TODO: Create GL compute shader
            return;
        }

        SNOW_LEOPARD_CORE_ASSERT(hasVertexStage && hasFragmentStage,
                                 "[GLShader] Missing vertex stage or fragment stage");

        GLuint vertexShader, fragmentShader, geometryShader;

        // Vertex stage
        const char* vShaderCode = shaderStageSources.at("vertex").c_str();
        // SNOW_LEOPARD_CORE_INFO("[GLShader] Compiling vertex shader :\n {0}", vShaderCode);

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vShaderCode, nullptr);
        glCompileShader(vertexShader);

        GLint isCompiled = 0;
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(vertexShader);

            SNOW_LEOPARD_CORE_ERROR("{0}", infoLog.data());
            SNOW_LEOPARD_CORE_ASSERT(false, "Failed to compile vertex shader!");
            return;
        }

        // Fragment stage
        const char* fShaderCode = shaderStageSources.at("fragment").c_str();
        // SNOW_LEOPARD_CORE_INFO("[GLShader] Compiling fragment shader :\n {0}", fShaderCode);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fShaderCode, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

            glDeleteShader(fragmentShader);
            glDeleteShader(vertexShader);

            SNOW_LEOPARD_CORE_ERROR("{0}", infoLog.data());
            SNOW_LEOPARD_CORE_ASSERT(false, "Failed to compile fragment shader!");
            return;
        }

        if (hasGeometryStage)
        {
            // Geometry stage
            const char* gShaderCode = shaderStageSources.at("geometry").c_str();
            // SNOW_LEOPARD_CORE_INFO("[GLShader] Compiling geometry shader :\n{0}", gShaderCode);

            geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometryShader, 1, &gShaderCode, nullptr);
            glCompileShader(geometryShader);

            glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE)
            {
                GLint maxLength = 0;
                glGetShaderiv(geometryShader, GL_INFO_LOG_LENGTH, &maxLength);

                // The maxLength includes the NULL character
                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(geometryShader, maxLength, &maxLength, &infoLog[0]);

                glDeleteShader(geometryShader);
                glDeleteShader(fragmentShader);
                glDeleteShader(vertexShader);

                SNOW_LEOPARD_CORE_ERROR("{0}", infoLog.data());
                SNOW_LEOPARD_CORE_ASSERT(false, "Failed to compile geometry shader!");
                return;
            }
        }

        m_ProgramID    = glCreateProgram();
        GLuint program = m_ProgramID;

        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);

        if (hasGeometryStage)
            glAttachShader(program, geometryShader);

        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, static_cast<int*>(&isLinked));
        if (isLinked == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(program);
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            if (hasGeometryStage)
                glDeleteShader(geometryShader);

            SNOW_LEOPARD_CORE_ERROR("{0}", infoLog.data());
            SNOW_LEOPARD_CORE_ASSERT(false, "Failed to link shader!");
            return;
        }

        glDetachShader(program, vertexShader);
        glDetachShader(program, fragmentShader);
        if (hasGeometryStage)
            glDetachShader(program, geometryShader);
    }

    GLShader::~GLShader() { glDeleteProgram(m_ProgramID); }

    void GLShader::Bind() const { glUseProgram(m_ProgramID); }

    void GLShader::Unbind() const { glUseProgram(0); }

    void GLShader::SetInt(const std::string& name, int value) const
    {
        GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
        glUniform1i(location, value);
    }

    void GLShader::SetIntArray(const std::string& name, int* values, uint32_t count) const
    {
        GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
        glUniform1iv(location, count, values);
    }

    void GLShader::SetFloat(const std::string& name, float value) const
    {
        GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
        glUniform1f(location, value);
    }

    void GLShader::SetFloat2(const std::string& name, const glm::vec2& value) const
    {
        GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
        glUniform2f(location, value.x, value.y);
    }

    void GLShader::SetFloat3(const std::string& name, const glm::vec3& value) const
    {
        GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
        glUniform3f(location, value.x, value.y, value.z);
    }

    void GLShader::SetFloat4(const std::string& name, const glm::vec4& value) const
    {
        GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }

    void GLShader::SetMat3(const std::string& name, const glm::mat3& value) const
    {
        GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void GLShader::SetMat4(const std::string& name, const glm::mat4& value) const
    {
        GLint location = glGetUniformLocation(m_ProgramID, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }
} // namespace SnowLeopardEngine