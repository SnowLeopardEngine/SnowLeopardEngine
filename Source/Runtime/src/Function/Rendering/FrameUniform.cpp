#include "SnowLeopardEngine/Function/Rendering/FrameUniform.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    void uploadFrameUniform(FrameGraph& fg, FrameGraphBlackboard& blackboard, const FrameUniform& frameUniform)
    {
        blackboard.add<FrameData>() = fg.addCallbackPass<FrameData>(
            "Upload FrameUniform",
            [&](FrameGraph::Builder& builder, FrameData& data) {
                data.FrameUniform = builder.create<FrameGraphBuffer>("FrameUniform", {.Size = sizeof(FrameUniform)});
                data.FrameUniform = builder.write(data.FrameUniform);
            },
            [=](const FrameData& data, FrameGraphPassResources& resources, void* ctx) {
                static_cast<RenderContext*>(ctx)->Upload(
                    getBuffer(resources, data.FrameUniform), 0, sizeof(FrameUniform), &frameUniform);
            });
    }
} // namespace SnowLeopardEngine