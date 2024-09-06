#include "SnowLeopardEngine/Core/Base/Hash.h"
#include "SnowLeopardEngine/Function/Rendering/VertexAttributes.h"

std::size_t std::hash<SnowLeopardEngine::VertexAttribute>::operator()(
    const SnowLeopardEngine::VertexAttribute& attribute) const noexcept
{
    std::size_t h {0};
    hashCombine(h, attribute.VertType, attribute.Offset);
    return h;
}