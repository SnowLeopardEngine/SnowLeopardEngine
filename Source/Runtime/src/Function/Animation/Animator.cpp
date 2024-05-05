#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Util/Util.h"

#include "ozz/geometry/runtime/skinning_job.h"

namespace SnowLeopardEngine
{
    Animator::Animator(const Ref<AnimationClip>& clip)
    {
        m_CurrentTime = 0.0;
        m_CurrentClip = clip;
    }

    void Animator::Update(float dt)
    {
        m_DeltaTime = dt;

        if (m_CurrentClip)
        {
            auto* meshItem = m_CurrentClip->MeshPtr;
            auto  ozzMesh  = meshItem->OzzMesh;
            m_CurrentTime += dt;

            // Calculate ratio
            float duration = m_CurrentClip->Animation.duration();
            if (m_CurrentTime >= duration && !m_Loop)
            {
                return;
            }
            float ratio = fmodf(m_CurrentTime / duration, 1.0f);

            // Sampling
            ozz::animation::SamplingJob samplingJob;
            samplingJob.animation = &m_CurrentClip->Animation;
            samplingJob.context   = &m_CurrentClip->SamplingJobContext;
            samplingJob.ratio     = ratio;
            samplingJob.output    = make_span(m_CurrentClip->LocalTransforms);
            samplingJob.Run();

            // Local to model
            ozz::animation::LocalToModelJob localToModelJob;
            localToModelJob.skeleton = &m_CurrentClip->Skeleton;
            localToModelJob.input    = make_span(m_CurrentClip->LocalTransforms);
            localToModelJob.output   = make_span(m_CurrentClip->ModelMatrices);
            localToModelJob.Run();

            // Skinning matrices
            for (uint32_t i = 0; i < ozzMesh.joint_remaps.size(); ++i)
            {
                m_CurrentClip->SkinningMatrices[i] =
                    m_CurrentClip->ModelMatrices[ozzMesh.joint_remaps[i]] * ozzMesh.inverse_bind_poses[i];
            }

            // CPU-Skinning by using ozz::geometry::SkinningJob
            uint32_t vertexIndex = 0;
            for (const auto& part : ozzMesh.parts)
            {
                ozz::geometry::SkinningJob skinningJob;
                skinningJob.vertex_count = static_cast<int>(part.vertex_count());
                // Clamps joints influence count according to the option.
                const int partInfluencesCount = part.influences_count();
                skinningJob.influences_count  = partInfluencesCount;

                // Setup skinning matrices, that came from the animation stage before being
                // multiplied by inverse model-space bind-pose.
                skinningJob.joint_matrices = make_span(m_CurrentClip->SkinningMatrices);

                // Setup joint's indices.
                skinningJob.joint_indices        = make_span(part.joint_indices);
                skinningJob.joint_indices_stride = sizeof(uint16_t) * partInfluencesCount;

                // Setup joint's weights.
                if (partInfluencesCount > 1)
                {
                    skinningJob.joint_weights        = make_span(part.joint_weights);
                    skinningJob.joint_weights_stride = sizeof(float) * (partInfluencesCount - 1);
                }

                // Setup input positions, coming from the loaded mesh.
                const uint32_t positionStride   = sizeof(float) * ozz::sample::Mesh::Part::kPositionsCpnts;
                skinningJob.in_positions        = make_span(part.positions);
                skinningJob.in_positions_stride = positionStride;

                // Setup output positions
                ozz::vector<float> outPositions;
                outPositions.resize(part.positions.size());
                skinningJob.out_positions        = make_span(outPositions);
                skinningJob.out_positions_stride = positionStride;

                // Setup input normals
                const uint32_t normalStride   = sizeof(float) * ozz::sample::Mesh::Part::kNormalsCpnts;
                skinningJob.in_normals        = make_span(part.normals);
                skinningJob.in_normals_stride = normalStride;

                // Setup output normals
                ozz::vector<float> outNormals;
                outNormals.resize(part.normals.size());
                skinningJob.out_normals        = make_span(outNormals);
                skinningJob.out_normals_stride = normalStride;

                // Execute the job
                if (!skinningJob.Run())
                {
                    SNOW_LEOPARD_CORE_ERROR("[Animator] Failed to execute skinning job!");
                    return;
                }

                for (uint32_t i = 0; i < part.vertex_count(); ++i)
                {
                    auto& vertex = meshItem->Data.Vertices[vertexIndex + i];

                    auto px         = outPositions[i * part.kPositionsCpnts];
                    auto py         = outPositions[i * part.kPositionsCpnts + 1];
                    auto pz         = outPositions[i * part.kPositionsCpnts + 2];
                    vertex.Position = {px, py, pz};

                    auto nx       = outNormals[i * part.kNormalsCpnts];
                    auto ny       = outNormals[i * part.kNormalsCpnts + 1];
                    auto nz       = outNormals[i * part.kNormalsCpnts + 2];
                    vertex.Normal = {nx, ny, nz};
                }

                vertexIndex += part.vertex_count();
            }
        }
    }

    void Animator::Play(const Ref<AnimationClip>& clip)
    {
        m_CurrentClip = clip;
        m_CurrentTime = 0.0f;
    }
} // namespace SnowLeopardEngine