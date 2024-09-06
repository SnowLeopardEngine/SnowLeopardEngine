#include "SnowLeopardEngine/Function/Scene/LayerManager.h"

namespace SnowLeopardEngine
{
    std::unordered_map<std::string, Layer> LayerMaskManager::s_Name2LayerMap;
    std::unordered_map<Layer, std::string> LayerMaskManager::s_Layer2NameMap;
    Layer                                  LayerMaskManager::s_LayerSeed = NextUserDefinedLayer;

#define REGISTER_LAYER(layerName) \
    s_Name2LayerMap[#layerName]                               = static_cast<Layer>(LayerMask::layerName); \
    s_Layer2NameMap[static_cast<Layer>(LayerMask::layerName)] = #layerName;

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
        if (s_Name2LayerMap.count(name) == 0)
        {
            return false;
        }

        layer = s_Name2LayerMap[name];
        return true;
    }

    bool LayerMaskManager::TryGetLayerNameByLayer(const Layer& layer, std::string& name)
    {
        if (s_Layer2NameMap.count(layer) == 0)
        {
            return false;
        }

        name = s_Layer2NameMap[layer];
        return true;
    }

    bool LayerMaskManager::TryAppendNewLayer(const std::string& name, Layer& layer)
    {
        if (s_Name2LayerMap.count(name) > 0)
        {
            return false;
        }

        layer                  = s_LayerSeed++;
        s_Name2LayerMap[name]  = layer;
        s_Layer2NameMap[layer] = name;
        return true;
    }

    std::vector<std::string> LayerMaskManager::GetNamedLayerNames()
    {
        std::vector<std::string> layerNames;

        for (Layer layer = 0; layer < s_LayerSeed; ++layer)
        {
            layerNames.emplace_back(s_Layer2NameMap[layer]);
        }

        return layerNames;
    }
} // namespace SnowLeopardEngine