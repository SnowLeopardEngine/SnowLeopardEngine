#include "SnowLeopardEngine/Function/Rendering/DzMaterial/DzMaterial.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    void DzMaterial::SetInt(const std::string& propertyName, int value)
    {
        std::optional<DzShaderProperty> findProperty;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Int")
            {
                findProperty = property;
                break;
            }
        }

        if (!findProperty.has_value())
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to set int value, property ({0}) not found!",
                                    propertyName);
            return;
        }

        findProperty->Value = std::to_string(value);
    }
} // namespace SnowLeopardEngine