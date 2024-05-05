#pragma once

#include "SnowLeopardEngine/Core/Base/Hash.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <cereal/cereal.hpp>

namespace SnowLeopardEngine
{
    const std::string RenderQueue_Opaque      = "Opaque";
    const std::string RenderQueue_Transparent = "Transparent";

    struct DzMaterialProperty
    {
        std::string Name;
        std::string Type;
        std::string Value;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Name), CEREAL_NVP(Type), CEREAL_NVP(Value));
        }
        // NOLINTEND
    };

    struct DzMaterial
    {
        std::string Name;

        std::string RenderQueue = RenderQueue_Opaque;

        std::vector<DzMaterialProperty> Properties;

        std::string UserVertPath;
        std::string UserFragPath;

        bool EnableInstancing = false;
        bool TextureFlipY     = false;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Name),
                    CEREAL_NVP(RenderQueue),
                    CEREAL_NVP(Properties),
                    CEREAL_NVP(UserVertPath),
                    CEREAL_NVP(UserFragPath),
                    CEREAL_NVP(EnableInstancing),
                    CEREAL_NVP(TextureFlipY));
        }
        // NOLINTEND
    };

    class Material
    {
        friend class RenderContext;

    public:
        Material() = default;
        explicit Material(const DzMaterial& define) : m_Define(define) {}

        void              SetDefine(const DzMaterial& define) { m_Define = define; }
        const DzMaterial& GetDefine() const { return m_Define; }

        size_t GetHash() const;

    private:
        void AutoBind(RenderContext& rc);

        void SetInt(RenderContext& rc, const std::string& propertyName, int value);
        void SetFloat(RenderContext& rc, const std::string& propertyName, float value);
        void SetColor(RenderContext& rc, const std::string& propertyName, const glm::vec4& value);
        void SetTexture2D(RenderContext& rc, const std::string& propertyName, const std::string& texturePath);
        void SetCubemap(RenderContext&                            rc,
                        const std::string&                        propertyName,
                        const std::vector<std::filesystem::path>& cubemapInfo);
        void SetVector(RenderContext& rc, const std::string& propertyName, const glm::vec4& value);

    private:
        DzMaterial m_Define;

        uint32_t m_TextureSlot = 0;
    };
} // namespace SnowLeopardEngine

namespace std
{
    template<>
    struct hash<SnowLeopardEngine::DzMaterial>
    {
        std::size_t operator()(const SnowLeopardEngine::DzMaterial&) const noexcept;
    };
} // namespace std