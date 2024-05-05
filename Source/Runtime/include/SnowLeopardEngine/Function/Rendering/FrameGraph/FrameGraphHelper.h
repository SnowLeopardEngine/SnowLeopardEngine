#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

#include "fg/FrameGraphResource.hpp"

class FrameGraph;
class FrameGraphPassResources;

namespace SnowLeopardEngine
{
    class Texture;

    FrameGraphResource importTexture(FrameGraph&, const std::string& name, Texture*);
    Texture&           getTexture(FrameGraphPassResources&, FrameGraphResource id);

    class Buffer;

    FrameGraphResource importBuffer(FrameGraph&, const std::string& name, Buffer*);

    Buffer& getBuffer(FrameGraphPassResources&, FrameGraphResource id);
} // namespace SnowLeopardEngine