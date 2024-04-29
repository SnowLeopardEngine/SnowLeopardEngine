#include "SnowLeopardEngine/Function/Rendering/LightUniform.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/LightData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    void uploadLightUniform(FrameGraph& fg, FrameGraphBlackboard& blackboard, const LightUniform& lightUniform)
    {
        blackboard.add<LightData>() = fg.addCallbackPass<LightData>(
            "Upload LightUniform",
            [&](FrameGraph::Builder& builder, LightData& data) {
                data.LightUniform = builder.create<FrameGraphBuffer>("LightUniform", {.Size = sizeof(LightUniform)});
                data.LightUniform = builder.write(data.LightUniform);
            },
            [=](const LightData& data, FrameGraphPassResources& resources, void* ctx) {
                static_cast<RenderContext*>(ctx)->Upload(
                    getBuffer(resources, data.LightUniform), 0, sizeof(LightUniform), &lightUniform);
            });
    }
} // namespace SnowLeopardEngine