#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/RenderSubDrawPass.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    class ForwardSkyboxSubPass : public RenderSubDrawPass
    {
    public:
        ForwardSkyboxSubPass();

        virtual void BeginSubPass() override {}
        virtual void EndSubPass() override {}

        virtual void Draw() override final;

    private:
        MeshItem m_SkyboxCubeMesh;
    };
} // namespace SnowLeopardEngine