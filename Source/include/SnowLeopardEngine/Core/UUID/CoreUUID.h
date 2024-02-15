#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

#include <uuid.h>

namespace SnowLeopardEngine
{
    using CoreUUID = uuids::uuid;

    class CoreUUIDHelper
    {
    public:
        static CoreUUID CreateStandardUUID();
        static CoreUUID GetFromName(const std::string& name);

    private:
        static Ref<uuids::uuid_name_generator> s_NameUUIDGenerator;
    };
} // namespace SnowLeopardEngine
