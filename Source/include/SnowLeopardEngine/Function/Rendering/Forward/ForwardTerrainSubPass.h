#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/RenderSubDrawPass.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    class ForwardTerrainSubPass : public RenderSubDrawPass
    {
    public:
        ForwardTerrainSubPass();

        virtual void BeginSubPass() override {}
        virtual void EndSubPass() override {}

        virtual void Draw() override final;

    private:
        MeshItem m_TerrainMesh;
    };
} // namespace SnowLeopardEngine