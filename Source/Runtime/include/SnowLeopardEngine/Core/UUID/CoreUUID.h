#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

#include <uuid.h>

namespace uuids
{
    template<class Archive>
    void save(Archive& archive, const uuid& id)
    {
        std::string idStr = to_string(id);
        archive(idStr);
    }

    template<class Archive>
    void load(Archive& archive, uuid& id)
    {
        std::string idStr;
        archive(idStr);
        id = uuid::from_string(idStr).value();
    }
} // namespace uuids

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
