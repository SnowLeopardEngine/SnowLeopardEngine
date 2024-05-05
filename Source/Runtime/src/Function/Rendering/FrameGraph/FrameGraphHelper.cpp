#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"

#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    FrameGraphResource importTexture(FrameGraph& fg, const std::string& name, Texture* texture)
    {
        assert(texture && *texture);
        return fg.import <FrameGraphTexture>(name,
                                             {
                                                 .Extent       = texture->GetExtent(),
                                                 .NumMipLevels = texture->GetNumMipLevels(),
                                                 .Layers       = texture->GetNumLayers(),
                                                 .Format       = texture->GetPixelFormat(),
                                             },
                                             {texture});
    }
    Texture& getTexture(FrameGraphPassResources& resources, FrameGraphResource id)
    {
        return *resources.get<FrameGraphTexture>(id).Handle;
    }

    FrameGraphResource importBuffer(FrameGraph& fg, const std::string& name, Buffer* buffer)
    {
        assert(buffer && *buffer);
        return fg.import <FrameGraphBuffer>(name, {.Size = buffer->GetSize()}, {buffer});
    }
    Buffer& getBuffer(FrameGraphPassResources& resources, FrameGraphResource id)
    {
        return *resources.get<FrameGraphBuffer>(id).Handle;
    }
} // namespace SnowLeopardEngine