#pragma once

#include "SnowLeopardEngine/Function/Animation/Animation.h"

namespace SnowLeopardEngine
{
    class Animator
    {
    public:
        explicit Animator(Ref<Animation> animation);
        std::vector<glm::mat4> GetFinalBoneMatrices() { return m_FinalBoneMatrices; }

    private:
        void UpdateAnimation(float dt);
        void PlayAnimation(Ref<Animation> pAnimation);
        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

        friend class AnimatorController;

    private:
        std::vector<glm::mat4> m_FinalBoneMatrices;
        Ref<Animation>         m_CurrentAnimation;
        float                  m_CurrentTime;
        float                  m_DeltaTime;
    };
} // namespace SnowLeopardEngine