#include "SnowLeopardEngine/Function/Rendering/DzMaterial/DzMaterial.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderManager.h"

namespace SnowLeopardEngine
{
    void DzMaterial::SetInt(const std::string& propertyName, int value)
    {
        // Update property block
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

    void DzMaterial::SetFloat(const std::string& propertyName, float value)
    {
        // Update property block
        std::optional<DzShaderProperty> findProperty;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Float")
            {
                findProperty = property;
                break;
            }
        }

        if (!findProperty.has_value())
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to set float value, property ({0}) not found!",
                                    propertyName);
            return;
        }

        findProperty->Value = std::to_string(value);
    }

    void DzMaterial::SetColor(const std::string& propertyName, const glm::vec4& value)
    {
        // Update property block
        std::optional<DzShaderProperty> findProperty;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Color")
            {
                findProperty = property;
                break;
            }
        }

        if (!findProperty.has_value())
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to set color value, property ({0}) not found!",
                                    propertyName);
            return;
        }

        findProperty->Value = fmt::format("({0}, {1}, {2}, {3})", value.r, value.g, value.b, value.a);
    }

    void DzMaterial::SetTexture(const std::string& propertyName, const std::string& texturePath)
    {
        // Update property block
        std::optional<DzShaderProperty> findProperty;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName &&
                (property.Type == "Texture2D" || property.Type == "Texture2DArray" || property.Type == "Texture3D" ||
                 property.Type == "Cubemap" || property.Type == "CubemapArray"))
            {
                findProperty = property;
                break;
            }
        }

        if (!findProperty.has_value())
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to set texture value, property ({0}) not found!",
                                    propertyName);
            return;
        }

        findProperty->Value = texturePath;
    }

    void DzMaterial::SetVector(const std::string& propertyName, const glm::vec4& value)
    {
        // Update property block
        std::optional<DzShaderProperty> findProperty;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Vector")
            {
                findProperty = property;
                break;
            }
        }

        if (!findProperty.has_value())
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to set vector value, property ({0}) not found!",
                                    propertyName);
            return;
        }

        findProperty->Value = fmt::format("({0}, {1}, {2}, {3})", value.x, value.y, value.z, value.w);
    }

    glm::vec4 DzMaterial::GetColor(const std::string& propertyName)
    {
        std::optional<DzShaderProperty> findProperty;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Color")
            {
                findProperty = property;
                break;
            }
        }

        if (!findProperty.has_value())
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to set color value, property ({0}) not found!",
                                    propertyName);
            return {};
        }

        auto value = findProperty->Value;
        trim(value);

        value                           = value.substr(1, value.size() - 2);
        std::vector<std::string> tokens = split(value);
        float                    r      = std::stof(tokens[0]);
        float                    g      = std::stof(tokens[1]);
        float                    b      = std::stof(tokens[2]);
        float                    a      = std::stof(tokens[3]);

        return glm::vec4(r, g, b, a);
    }

    std::string DzMaterial::GetTag(const std::string& key)
    {
        auto& dzShader = DzShaderManager::GetShader(m_ShaderName);
        if (dzShader.Tags.count(key) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to get tag value, tag key ({0}) not found!", key);
            return "";
        }

        return dzShader.Tags[key];
    }
} // namespace SnowLeopardEngine