#pragma once

#include <cstdint>

namespace JPH
{
    class CharacterVirtual;
} // namespace JPH

namespace SnowLeopardEngine
{
    // Opaque handle to a body owned by the PhysicsSystem (the raw value of a JPH::BodyID).
    using PhysicsBodyHandle = uint32_t;

    // Same bit pattern as JPH::BodyID::cInvalidBodyID.
    inline constexpr PhysicsBodyHandle InvalidPhysicsBody = 0xffffffff;

    // Character controllers are owned by the PhysicsSystem; components only keep a non-owning pointer.
    using PhysicsCharacterHandle = JPH::CharacterVirtual*;
} // namespace SnowLeopardEngine
