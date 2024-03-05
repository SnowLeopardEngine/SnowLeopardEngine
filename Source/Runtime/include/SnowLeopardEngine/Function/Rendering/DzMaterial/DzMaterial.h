#pragma once

#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"

namespace SnowLeopardEngine
{
    struct DzMaterialPropertyBlock
    {
        bool                          ReadOnly = true;
        std::vector<DzShaderProperty> ShaderProperties;
    };

    class DzMaterial
    {
    public:
        void SetInt(const std::string& propertyName, int value);

        // TODO: More Setters

        // TODO: Getters
    private:
        DzMaterialPropertyBlock m_PropertyBlock;
        std::string             m_ShaderName;
    };
} // namespace SnowLeopardEngine