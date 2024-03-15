#include "SnowLeopardEngine/Function/Scene/LayerManager.h"

namespace SnowLeopardEngine
{
    std::unordered_map<std::string, Layer> LayerMaskManager::s_LayerMap;
    Layer                                  LayerMaskManager::s_LayerSeed = NextUserDefinedLayer;

#define REGISTER_LAYER(layerName) s_LayerMap[#layerName] = static_cast<Layer>(LayerMask::layerName);

    void LayerMaskManager::Init()
    {
        REGISTER_LAYER(Default)
        REGISTER_LAYER(TransparentFX)
        REGISTER_LAYER(IgnoreRaycast)
        REGISTER_LAYER(Water)
        REGISTER_LAYER(UI)
        REGISTER_LAYER(PostProcessing)
    }

    bool LayerMaskManager::TryGetLayerByName(const std::string& name, Layer& layer)
    {
        if (s_LayerMap.count(name) == 0)
        {
            return false;
        }

        layer = s_LayerMap[name];
        return true;
    }

    bool LayerMaskManager::TryAppendNewLayer(const std::string& name, Layer& layer)
    {
        if (s_LayerMap.count(name) > 0)
        {
            return false;
        }

        layer            = s_LayerSeed++;
        s_LayerMap[name] = layer;
        return true;
    }
} // namespace SnowLeopardEngine