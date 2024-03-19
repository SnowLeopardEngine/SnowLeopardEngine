#include "SnowLeopardEngine/Function/Rendering/DzMaterial/DzMaterial.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Asset/Loaders/TextureLoader.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderManager.h"

namespace SnowLeopardEngine
{
    Ref<DzMaterial> DzMaterial::LoadFromPath(const std::filesystem::path& path)
    {
        std::ifstream            materialFile(path);
        cereal::JSONInputArchive archive(materialFile);
        auto                     blue = CreateRef<DzMaterial>();
        archive(*blue);

        return blue;
    }

    void DzMaterial::SaveToPath(const Ref<DzMaterial>& material, const std::filesystem::path& path)
    {
        // TODO
    }

    void DzMaterial::SetInt(const std::string& propertyName, int value)
    {
        // Update property block
        DzShaderProperty* findProperty = nullptr;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Int")
            {
                findProperty = &property;
                break;
            }
        }

        if (findProperty == nullptr)
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
        DzShaderProperty* findProperty = nullptr;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Float")
            {
                findProperty = &property;
                break;
            }
        }

        if (findProperty == nullptr)
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
        DzShaderProperty* findProperty = nullptr;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Color")
            {
                findProperty = &property;
                break;
            }
        }

        if (findProperty == nullptr)
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to set color value, property ({0}) not found!",
                                    propertyName);
            return;
        }

        findProperty->Value = fmt::format("({0}, {1}, {2}, {3})", value.r, value.g, value.b, value.a);
    }

    void DzMaterial::SetTexture2D(const std::string& propertyName, const std::string& texturePath)
    {
        // Update property block
        DzShaderProperty* findProperty = nullptr;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Texture2D")
            {
                findProperty = &property;
                break;
            }
        }

        if (findProperty == nullptr)
        {
            SNOW_LEOPARD_CORE_ERROR(
                "[DzMaterialPropertyBlock] Failed to set texture2d value, property ({0}) not found!", propertyName);
            return;
        }

        findProperty->Value = texturePath;
    }

    void DzMaterial::SetCubemap(const std::string& propertyName, const std::string& cubemapInfo)
    {
        // Update property block
        DzShaderProperty* findProperty = nullptr;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Cubemap")
            {
                findProperty = &property;
                break;
            }
        }

        if (findProperty == nullptr)
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to set cubemap value, property ({0}) not found!",
                                    propertyName);
            return;
        }

        findProperty->Value = cubemapInfo;
    }

    void DzMaterial::SetVector(const std::string& propertyName, const glm::vec4& value)
    {
        // Update property block
        DzShaderProperty* findProperty = nullptr;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Vector")
            {
                findProperty = &property;
                break;
            }
        }

        if (findProperty == nullptr)
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to set vector value, property ({0}) not found!",
                                    propertyName);
            return;
        }

        findProperty->Value = fmt::format("({0}, {1}, {2}, {3})", value.x, value.y, value.z, value.w);
    }

    int DzMaterial::GetInt(const std::string& propertyName)
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
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to get int value, property ({0}) not found!",
                                    propertyName);
            return INT_MAX;
        }

        return std::stoi(findProperty->Value);
    }

    float DzMaterial::GetFloat(const std::string& propertyName)
    {
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
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to get float value, property ({0}) not found!",
                                    propertyName);
            return INT_MAX * 1.0f;
        }

        return std::stof(findProperty->Value);
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
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to get color value, property ({0}) not found!",
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

    Ref<Texture2D> DzMaterial::GetTexture2D(const std::string& propertyName)
    {
        std::optional<DzShaderProperty> findProperty;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Texture2D")
            {
                findProperty = property;
                break;
            }
        }

        if (!findProperty.has_value())
        {
            SNOW_LEOPARD_CORE_ERROR(
                "[DzMaterialPropertyBlock] Failed to get texture2d value, property ({0}) not found!", propertyName);
            return nullptr;
        }

        if (findProperty->Value.empty())
        {
            findProperty->Value = "Assets/Textures/white_1x1.jpg";
        }

        return TextureLoader::LoadTexture2D(findProperty->Value, false);
    }

    Ref<Cubemap> DzMaterial::GetCubemap(const std::string& propertyName)
    {
        std::optional<DzShaderProperty> findProperty;
        for (auto& property : m_PropertyBlock.ShaderProperties)
        {
            if (property.Name == propertyName && property.Type == "Cubemap")
            {
                findProperty = property;
                break;
            }
        }

        if (!findProperty.has_value())
        {
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to get cubemap value, property ({0}) not found!",
                                    propertyName);
            return nullptr;
        }

        if (findProperty->Value.empty())
        {
            findProperty->Value = "Assets/Textures/Skybox001;right,left,top,bottom,front,back;.jpg";
        }

        auto value = findProperty->Value;
        trim(value);

        auto tokens = split(value, ';');

        auto pathPrefix = std::filesystem::path(tokens[0]);
        auto faces      = tokens[1];
        auto suffix     = tokens[2];

        auto faceTokens = split(faces);

        std::vector<std::filesystem::path> facePaths = {
            pathPrefix / (faceTokens[0] + suffix),
            pathPrefix / (faceTokens[1] + suffix),
            pathPrefix / (faceTokens[2] + suffix),
            pathPrefix / (faceTokens[3] + suffix),
            pathPrefix / (faceTokens[4] + suffix),
            pathPrefix / (faceTokens[5] + suffix),
        };

        return TextureLoader::LoadCubemap(facePaths, false);
    }

    glm::vec4 DzMaterial::GetVector(const std::string& propertyName)
    {
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
            SNOW_LEOPARD_CORE_ERROR("[DzMaterialPropertyBlock] Failed to get vector value, property ({0}) not found!",
                                    propertyName);
            return {};
        }

        auto value = findProperty->Value;
        trim(value);

        value                           = value.substr(1, value.size() - 2);
        std::vector<std::string> tokens = split(value);
        float                    x      = std::stof(tokens[0]);
        float                    y      = std::stof(tokens[1]);
        float                    z      = std::stof(tokens[2]);
        float                    w      = std::stof(tokens[3]);

        return glm::vec4(x, y, z, w);
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