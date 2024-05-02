#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BaseGeometryPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Material.h"

namespace SnowLeopardEngine
{
    BaseGeometryPass::BaseGeometryPass(RenderContext& rc) : m_RenderContext(rc) {}

    BaseGeometryPass::~BaseGeometryPass()
    {
        for (auto& [_, pipeline] : m_Pipelines)
            m_RenderContext.Destroy(pipeline);
    }

    void BaseGeometryPass::SetTransform(const glm::mat4& modelMatrix)
    {
        m_RenderContext.SetUniformMat4("model", modelMatrix);
    }

    void BaseGeometryPass::SetTransform(const glm::mat4& modelMatrix, uint32_t instanceId)
    {
        m_RenderContext.SetUniformMat4("models[" + std::to_string(instanceId) + "]", modelMatrix);
    }

    GraphicsPipeline& BaseGeometryPass::GetPipeline(const VertexFormat& vertexFormat, const Material* material)
    {
        SNOW_LEOPARD_CORE_ASSERT(material != nullptr, "[BaseGeometryPass] Material is nullptr!");

        auto hash = vertexFormat.GetHash();
        if (material)
            hashCombine(hash, material->GetHash());

        GraphicsPipeline* basePassPipeline = nullptr;

        if (const auto it = m_Pipelines.find(hash); it != m_Pipelines.cend())
            basePassPipeline = &it->second;

        if (!basePassPipeline)
        {
            auto pipeline = CreateBasePassPipeline(vertexFormat, material);

            SNOW_LEOPARD_CORE_INFO(
                "[RenderPass] Created a pipeline {0} for material: {1}", hash, material->GetDefine().Name);

            const auto& it   = m_Pipelines.insert_or_assign(hash, std::move(pipeline)).first;
            basePassPipeline = &it->second;
        }

        return *basePassPipeline;
    }
} // namespace SnowLeopardEngine