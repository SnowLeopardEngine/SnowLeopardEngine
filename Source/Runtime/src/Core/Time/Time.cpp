#include "SnowLeopardEngine/Core/Time/Time.h"

namespace SnowLeopardEngine
{
    float Time::DeltaTime      = 0;
    float Time::FixedDeltaTime = 1.0 / 50.0;
    float Time::ElapsedTime    = 0;
} // namespace SnowLeopardEngine
