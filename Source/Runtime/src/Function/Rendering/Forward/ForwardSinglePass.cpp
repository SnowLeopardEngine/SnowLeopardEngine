#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardGeometrySubPass.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardShadowSubPass.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSkyboxSubPass.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardTerrainSubPass.h"

namespace SnowLeopardEngine
{
    ForwardSinglePass::ForwardSinglePass()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        // Add sub-passes

        // Shadow
        auto shadowSubPass = CreateRef<ForwardShadowSubPass>();
        shadowSubPass->BindOwnerPass(this);
        m_SubDrawPasses.emplace_back(shadowSubPass);

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

        // Prepare pass common data

        // Depth frame buffer
        FrameBufferDesc frameBufferDesc = {};
        frameBufferDesc.Width           = 4096;
        frameBufferDesc.Height          = 4096;

        FrameBufferTextureDesc depthTexture = {};
        depthTexture.TextureFormat          = FrameBufferTextureFormat::DEPTH24;
        frameBufferDesc.AttachmentDesc.Attachments.emplace_back(depthTexture);
        m_DepthBuffer = FrameBuffer::Create(frameBufferDesc);
    }
} // namespace SnowLeopardEngine