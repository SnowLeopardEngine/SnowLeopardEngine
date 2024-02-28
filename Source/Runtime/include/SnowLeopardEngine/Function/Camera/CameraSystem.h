#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine
{
    class CameraSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(CameraSystem)

        glm::mat4 GetViewMatrix(const TransformComponent& cameraTransform);
        glm::mat4 GetProjectionMatrix(const CameraComponent& camera);
    };
} // namespace SnowLeopardEngine