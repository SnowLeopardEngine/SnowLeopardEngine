#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"

namespace SnowLeopardEngine
{
    class GLShader : public Shader
    {
    public:
        GLShader(const std::string& vertexSource, const std::string& fragmentSource);
        virtual ~GLShader();

        virtual void Bind() const override;
        virtual void Unbind() const override;

        virtual void SetInt(const std::string& name, int value) const override;
        virtual void SetIntArray(const std::string& name, int* values, uint32_t count) const override;

        virtual void SetFloat(const std::string& name, float value) const override;
        virtual void SetFloat2(const std::string& name, const glm::vec2& value) const override;
        virtual void SetFloat3(const std::string& name, const glm::vec3& value) const override;
        virtual void SetFloat4(const std::string& name, const glm::vec4& value) const override;

        virtual void SetMat3(const std::string& name, const glm::mat3& value) const override;
        virtual void SetMat4(const std::string& name, const glm::mat4& value) const override;

    private:
        uint32_t m_ProgramID;
    };
} // namespace SnowLeopardEngine