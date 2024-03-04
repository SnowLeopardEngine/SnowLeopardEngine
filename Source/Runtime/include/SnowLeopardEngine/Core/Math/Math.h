#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SnowLeopardEngine/Platform/Platform.h"

#if SNOW_LEOPARD_PLATFORM_WINDOWS
#define M_PI 3.1415926535
#endif

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
