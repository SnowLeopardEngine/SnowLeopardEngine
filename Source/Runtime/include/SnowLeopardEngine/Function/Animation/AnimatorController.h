#pragma once

#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Function/Animation/Transition.h"

namespace SnowLeopardEngine
{
    class AnimatorController
    {
    public:
        void RegisterAnimator(const Ref<Animator>& animator);
        void DeleteAnimator(const Ref<Animator>& animator);

        void RegisterTransition(const Ref<Animator>& sourceAnimator,
                                const Ref<Animator>& targetAnimator,
                                int                  duration,
                                std::string          triggerName);

        void DeleteTransition(const Ref<Animator>& sourceAnimator, std::string triggerName);

        void Blending(const Ref<Animator>& sourceAnimator, const Ref<Animator>& targetAnimator);

        void SetEntryAnimator(const Ref<Animator>& animator);

        void SetTrigger(std::string triggerName);

        void InitAnimators();
        void UpdateAnimators(float deltaTime);

        inline Ref<Animator> GetCurrentAnimator() const { return m_CurrentAnimator; }

    private:
        Ref<Animator>                                         m_CurrentAnimator;
        std::vector<Ref<Animator>>                            m_Animators;
        std::map<Ref<Animator>, std::vector<Ref<Transition>>> m_TransitionsInfoMap;
    };
} // namespace SnowLeopardEngine