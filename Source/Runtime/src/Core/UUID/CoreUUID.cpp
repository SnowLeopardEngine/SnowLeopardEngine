#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"

namespace SnowLeopardEngine
{
    Ref<uuids::uuid_name_generator> CoreUUIDHelper::s_NameUUIDGenerator =
        CreateRef<uuids::uuid_name_generator>(CoreUUIDHelper::CreateStandardUUID());

    CoreUUID CoreUUIDHelper::CreateStandardUUID()
    {
        std::random_device rd;

        auto seed_data = std::array<int, std::mt19937::state_size> {};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq                seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937                 generator(seq);
        uuids::uuid_random_generator gen {generator};

        return gen();
    }

    CoreUUID CoreUUIDHelper::GetFromName(const std::string& name) { return (*s_NameUUIDGenerator)(name); }
} // namespace SnowLeopardEngine
