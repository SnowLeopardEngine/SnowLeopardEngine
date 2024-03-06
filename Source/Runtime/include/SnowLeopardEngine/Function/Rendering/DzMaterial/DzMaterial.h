#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"

namespace SnowLeopardEngine
{
    class Texture2D;

    struct DzMaterialPropertyBlock
    {
        std::vector<DzShaderProperty> ShaderProperties;
        bool                          ReadOnly = true;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(ShaderProperties), CEREAL_NVP(ReadOnly));
        }
        // NOLINTEND
    };

    class DzMaterial
    {
    public:
        static Ref<DzMaterial> LoadFromPath(const std::filesystem::path& path);
        static void            SaveToPath(const Ref<DzMaterial>& material, const std::filesystem::path& path);

        void SetInt(const std::string& propertyName, int value);
        void SetFloat(const std::string& propertyName, float value);
        void SetColor(const std::string& propertyName, const glm::vec4& value);
        void SetTexture(const std::string& propertyName, const std::string& texturePath);
        void SetVector(const std::string& propertyName, const glm::vec4& value);

        int            GetInt(const std::string& propertyName);
        float          GetFloat(const std::string& propertyName);
        glm::vec4      GetColor(const std::string& propertyName);
        Ref<Texture2D> GetTexture(const std::string& propertyName);
        glm::vec4      GetVector(const std::string& propertyName);

        std::string GetTag(const std::string& key);

        DzMaterialPropertyBlock GetPropertyBlock() const { return m_PropertyBlock; }

        void                  SetShaderName(const std::string& shaderName) { m_ShaderName = shaderName; }
        const std::string&    GetShaderName() const { return m_ShaderName; }
        std::filesystem::path GetShaderPath() const
        {
            return std::filesystem::path("Assets/Shaders") / (m_ShaderName + ".dzshader");
        }

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(m_ShaderName), CEREAL_NVP(m_PropertyBlock));
        }
        // NOLINTEND

    private:
        DzMaterialPropertyBlock m_PropertyBlock;
        std::string             m_ShaderName;
    };
} // namespace SnowLeopardEngine