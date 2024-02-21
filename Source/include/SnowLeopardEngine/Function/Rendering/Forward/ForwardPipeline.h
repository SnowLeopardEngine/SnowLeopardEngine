#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"

namespace SnowLeopardEngine
{
    class ForwardPipeline : public Pipeline
    {
    public:
        virtual bool Init(const PipelineInitInfo& initInfo) override;
    };
} // namespace SnowLeopardEngine