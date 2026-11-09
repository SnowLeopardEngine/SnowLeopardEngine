#pragma once

#include "SnowLeopardEngine/Core/Base/Hash.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"

#include <cereal/cereal.hpp>

namespace SnowLeopardEngine
{
    const std::string RenderQueue_Opaque      = "Opaque";
    const std::string RenderQueue_AlphaTest   = "AlphaTest"; // cut-out (foliage, fences): two-sided, alpha cutoff
    const std::string RenderQueue_Transparent = "Transparent";

    struct DzMaterialProperty
    {
        std::string Name;
        std::string Type;
        std::string Value;
        // Texture2D from a cooked material: the texture asset (nil: load `Value` as a file). Not part of the
        // .dzmaterial text.
        CoreUUID TextureId;

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

        int TileSize = 1;

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
                    CEREAL_NVP(TextureFlipY),
                    CEREAL_NVP(TileSize));
        }
        // NOLINTEND
    };

    // A .dzmaterial definition. GPU state (pipeline, descriptor sets) is created by the renderer (Phase 2.3 / 2.4).
    class Material
    {
    public:
        Material() = default;
        explicit Material(const DzMaterial& define) : m_Define(define) {}

        void              SetDefine(const DzMaterial& define) { m_Define = define; }
        const DzMaterial& GetDefine() const { return m_Define; }

        size_t GetHash() const;

    private:
        DzMaterial m_Define;
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