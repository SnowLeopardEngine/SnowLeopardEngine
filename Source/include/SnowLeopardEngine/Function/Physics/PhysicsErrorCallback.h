#pragma once

#include <PxPhysicsAPI.h>

namespace SnowLeopardEngine
{
    class PhysicsErrorCallback : public physx::PxErrorCallback
    {
    public:
        virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line);
    };
} // namespace SnowLeopardEngine