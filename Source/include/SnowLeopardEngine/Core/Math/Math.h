#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SnowLeopardEngine::Math
{
    static glm::mat4 GetTransformMatrix(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
    {
        glm::mat4 result(1.0f);
        result = glm::translate(result, position);
        result = glm::rotate(result, angle(rotation), axis(rotation));
        result = glm::scale(result, scale);
        return result;
    }
} // namespace SnowLeopardEngine::Math
