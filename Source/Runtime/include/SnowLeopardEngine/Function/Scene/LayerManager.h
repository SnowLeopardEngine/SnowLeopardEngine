#pragma once

#include "SnowLeopardEngine/Core/Base/BitFlags.h"

namespace SnowLeopardEngine
{
    using Layer = uint32_t;

    enum class LayerMask : Layer
    {
        Default        = 0,
        TransparentFX  = 1 << 0,
        IgnoreRaycast  = 1 << 1,
        Water          = 1 << 2,
        UI             = 1 << 3,
        PostProcessing = 1 << 4,
        Extend001      = 1 << 5,
        Extend002      = 1 << 6,
        Extend003      = 1 << 7,
        Extend004      = 1 << 8,
        Extend005      = 1 << 9,
        Extend006      = 1 << 10,
        Extend007      = 1 << 11,
        Extend008      = 1 << 12,
        Extend009      = 1 << 13,
        Extend010      = 1 << 14,
        Extend011      = 1 << 15,
        Extend012      = 1 << 16,
        Extend013      = 1 << 17,
        Extend014      = 1 << 18,
        Extend015      = 1 << 19,
        Extend016      = 1 << 20,
        Extend017      = 1 << 21,
        Extend018      = 1 << 22,
        Extend019      = 1 << 23,
        Extend020      = 1 << 24,
        Extend021      = 1 << 25,
        Extend022      = 1 << 26,
        Extend023      = 1 << 27,
        Extend024      = 1 << 28,
        Extend025      = 1 << 29,
        Extend026      = 1 << 30
    };
    DEFINE_BITFLAGS(LayerMask, Layer)
    const Layer NextUserDefinedLayer = static_cast<Layer>(LayerMask::Extend001);

    class LayerMaskManager
    {
    public:
        static void Init();

        static bool TryGetLayerByName(const std::string& name, Layer& layer);
        static bool TryAppendNewLayer(const std::string& name, Layer& layer);

        static std::unordered_map<std::string, Layer> s_LayerMap;
        static Layer                                  s_LayerSeed;
    };
} // namespace SnowLeopardEngine