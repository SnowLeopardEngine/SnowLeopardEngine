#include "SnowLeopardEngine/Function/Animation/Animator.h"

namespace SnowLeopardEngine
{
    const int MAX_BONES = 128;

    Animator::Animator(Ref<Animation> animation)
    {
        m_CurrentTime      = 0.0;
        m_CurrentAnimation = animation;

        m_FinalBoneMatrices.reserve(MAX_BONES);

        for (int i = 0; i < MAX_BONES; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    void Animator::UpdateAnimation(float dt)
    {
        m_DeltaTime = dt;
        if (m_CurrentAnimation)
        {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    void Animator::PlayAnimation(Ref<Animation> pAnimation)
    {
        m_CurrentAnimation = pAnimation;
        m_CurrentTime      = 0.0f;
    }

    void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
    {
        std::string nodeName      = node->Name;
        glm::mat4   nodeTransform = node->Transformation;

        Bone* bone = m_CurrentAnimation->FindBone(nodeName);

        if (bone)
        {
            bone->Update(m_CurrentTime);
            nodeTransform = bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap[nodeName].Id;
            if (index < MAX_BONES)
            {
                glm::mat4 offset           = boneInfoMap[nodeName].Offset;
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            }
        }

        for (int i = 0; i < node->ChildrenCount; i++)
            CalculateBoneTransform(&node->Children[i], globalTransformation);
    }
} // namespace SnowLeopardEngine