#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"

namespace SnowLeopardEngine
{
    class Shader
    {
    public:
        virtual ~Shader() = default;

        virtual void Bind() const   = 0;
        virtual void Unbind() const = 0;

        virtual void SetInt(const std::string& name, int value) const                        = 0;
        virtual void SetIntArray(const std::string& name, int* values, uint32_t count) const = 0;

        virtual void SetFloat(const std::string& name, float value) const             = 0;
        virtual void SetFloat2(const std::string& name, const glm::vec2& value) const = 0;
        virtual void SetFloat3(const std::string& name, const glm::vec3& value) const = 0;
        virtual void SetFloat4(const std::string& name, const glm::vec4& value) const = 0;

        virtual void SetMat3(const std::string& name, const glm::mat3& value) const = 0;
        virtual void SetMat4(const std::string& name, const glm::mat4& value) const = 0;

        static Ref<Shader> Create(const std::string& vertexSource, const std::string& fragmentSource);
    };
} // namespace SnowLeopardEngine