#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardGeometrySubPass.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSkyboxSubPass.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardTerrainSubPass.h"

namespace SnowLeopardEngine
{
    void ForwardSinglePass::BeginPass()
    {
        // Add sub-passes

        // Geometry
        auto geometrySubPass = CreateRef<ForwardGeometrySubPass>();
        geometrySubPass->BindOwnerPass(this);
        m_SubDrawPasses.emplace_back(geometrySubPass);

        // Terrain
        auto terrainSubPass = CreateRef<ForwardTerrainSubPass>();
        terrainSubPass->BindOwnerPass(this);
        m_SubDrawPasses.emplace_back(terrainSubPass);

        // Skybox
        auto skyboxSubPass = CreateRef<ForwardSkyboxSubPass>();
        skyboxSubPass->BindOwnerPass(this);
        m_SubDrawPasses.emplace_back(skyboxSubPass);
    }
} // namespace SnowLeopardEngine