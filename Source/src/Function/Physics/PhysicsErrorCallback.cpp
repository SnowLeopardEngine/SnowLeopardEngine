#include "SnowLeopardEngine/Function/Physics/PhysicsErrorCallback.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    void
    PhysicsErrorCallback::reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line)
    {
        SNOW_LEOPARD_CORE_ERROR(
            "[PhysicsSystem] ErrorCode: {0}, Message: {1}, File: {2}, Line: {3}", code, message, file, line);
    }
} // namespace SnowLeopardEngine