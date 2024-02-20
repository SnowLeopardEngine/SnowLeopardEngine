#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"

namespace SnowLeopardEngine
{
    class ForwardPipeline : public Pipeline
    {
    public:
        virtual bool Init() override;
    };
} // namespace SnowLeopardEngine