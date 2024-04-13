#include "SnowLeopardEngine/Function/Camera/CameraSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    CameraSystem::CameraSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[CameraSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    CameraSystem::~CameraSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[CameraSystem] Shutting Down...");
        m_State = SystemState::ShutdownOk;
    }

    glm::mat4 CameraSystem::GetViewMatrix(const TransformComponent& cameraTransform)
    {
        // Setup camera matrices
        auto eulerAngles = cameraTransform.GetRotationEuler();

        // Calculate forward (Yaw - 90 to adjust)
        glm::vec3 forward;
        forward.x = cos(glm::radians(eulerAngles.y - 90)) * cos(glm::radians(eulerAngles.x));
        forward.y = sin(glm::radians(eulerAngles.x));
        forward.z = sin(glm::radians(eulerAngles.y - 90)) * cos(glm::radians(eulerAngles.x));
        forward   = glm::normalize(forward);

        // Calculate up
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
        glm::vec3 up    = glm::normalize(glm::cross(right, forward));

        return glm::lookAt(cameraTransform.Position, cameraTransform.Position + forward, up);
    }

    glm::mat4 CameraSystem::GetProjectionMatrix(const CameraComponent& camera)
    {
        if (camera.Projection == CameraProjection::Perspective)
        {
            return glm::perspective(
                glm::radians(camera.FOV), camera.ViewportWidth / camera.ViewportHeight, camera.Near, camera.Far);
        }
        else
        {
            return glm::ortho(0.0f, camera.ViewportWidth, camera.ViewportHeight, 0.0f, -1.0f, 1.0f);
        }
    }
} // namespace SnowLeopardEngine