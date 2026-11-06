#include "SnowLeopardEngine/Function/Rendering/Material.h"

namespace SnowLeopardEngine
{
    size_t Material::GetHash() const
    {
        size_t hash {0u};
        hashCombine(hash, m_Define);

        return hash;
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