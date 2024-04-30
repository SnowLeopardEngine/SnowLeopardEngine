#include "SnowLeopardEngine/Function/Rendering/Material.h"
#include "SnowLeopardEngine/Function/IO/TextureLoader.h"

namespace SnowLeopardEngine
{
    size_t Material::GetHash() const
    {
        size_t hash {0u};
        hashCombine(hash, m_Define);

        return hash;
    }

    void Material::AutoBind(RenderContext& rc)
    {
        m_TextureSlot = 0;

        for (const auto& property : m_Define.Properties)
        {
            if (property.Type == "Int")
            {
                int value = std::stoi(property.Value);
                SetInt(rc, property.Name, value);
            }
            else if (property.Type == "Float")
            {
                float value = std::stof(property.Value);
                SetFloat(rc, property.Name, value);
            }
            else if (property.Type == "Color")
            {
                auto value = property.Value;
                trim(value);

                value                           = value.substr(1, value.size() - 2);
                std::vector<std::string> tokens = split(value);
                float                    r      = std::stof(tokens[0]);
                float                    g      = std::stof(tokens[1]);
                float                    b      = std::stof(tokens[2]);
                float                    a      = std::stof(tokens[3]);

                SetColor(rc, property.Name, glm::vec4(r, g, b, a));
            }
            else if (property.Type == "Texture2D")
            {
                SetTexture2D(rc, property.Name, property.Value);
            }
            else if (property.Type == "Cubemap")
            {
                auto value = property.Value;
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

                SetCubemap(rc, property.Name, facePaths);
            }
            else if (property.Type == "Vector")
            {
                auto value = property.Value;
                trim(value);

                value                           = value.substr(1, value.size() - 2);
                std::vector<std::string> tokens = split(value);
                float                    x      = std::stof(tokens[0]);
                float                    y      = std::stof(tokens[1]);
                float                    z      = std::stof(tokens[2]);
                float                    w      = std::stof(tokens[3]);

                SetVector(rc, property.Name, glm::vec4(x, y, z, w));
            }
        }
    }

    void Material::SetInt(RenderContext& rc, const std::string& propertyName, int value)
    {
        rc.SetUniform1i(propertyName, value);
    }

    void Material::SetFloat(RenderContext& rc, const std::string& propertyName, float value)
    {
        rc.SetUniform1f(propertyName, value);
    }

    void Material::SetColor(RenderContext& rc, const std::string& propertyName, const glm::vec4& value)
    {
        rc.SetUniformVec4(propertyName, value);
    }

    void Material::SetTexture2D(RenderContext& rc, const std::string& propertyName, const std::string& texturePath)
    {
        auto* texture = IO::Load(texturePath, rc, m_Define.TextureFlipY);
        if (texture == nullptr)
        {
            return;
        }

        rc.BindTexture(m_TextureSlot++, *texture);
    }

    void Material::SetCubemap(RenderContext&                            rc,
                              const std::string&                        propertyName,
                              const std::vector<std::filesystem::path>& cubemapInfo)
    {
        // TODO
    }

    void Material::SetVector(RenderContext& rc, const std::string& propertyName, const glm::vec4& value)
    {
        rc.SetUniformVec4(propertyName, value);
    }
} // namespace SnowLeopardEngine

std::size_t
std::hash<SnowLeopardEngine::DzMaterial>::operator()(const SnowLeopardEngine::DzMaterial& dzMaterial) const noexcept
{
    std::size_t h {0};
    SnowLeopardEngine::hashCombine(h,
                                   dzMaterial.Name,
                                   dzMaterial.RenderQueue,
                                   dzMaterial.UserFragPath,
                                   dzMaterial.UserVertPath,
                                   dzMaterial.EnableInstancing,
                                   dzMaterial.TextureFlipY);

    for (const auto& property : dzMaterial.Properties)
    {
        SnowLeopardEngine::hashCombine(h, property.Name, property.Type, property.Value);
    }

    return h;
}