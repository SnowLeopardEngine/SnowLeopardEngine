#include "SnowLeopardEngine/Function/Util/Util.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine
{
    physx::PxTransform PhysXGLMHelpers::GetPhysXTransform(TransformComponent* transform)
    {
        physx::PxTransform pxTransform = {};
        pxTransform.p                  = GetPhysXVec3(transform->Position);
        pxTransform.q                  = GetPhysXQuat(transform->GetRotation());
        return pxTransform;
    }
} // namespace SnowLeopardEngine