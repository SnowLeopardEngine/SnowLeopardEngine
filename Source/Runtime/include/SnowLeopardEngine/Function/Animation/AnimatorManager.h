#pragma once

#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Function/Animation/Transition.h"

namespace SnowLeopardEngine
{
    class AnimatorManager
    {
    public:
        void Init();
        void Update(float deltaTime);

        void RegisterAnimator(const Ref<Animator>& animator);
        void DeleteAnimator(const Ref<Animator>& animator);

    private:
        void InitAnimators();
        void UpdateAnimator(float deltaTime);

    private:
        Ref<Animator>                                                         m_CurrentAnimator;
    public:
        std::vector<Ref<Animator>>                                            m_Animators;
    };
} // namespace SnowLeopardEngine