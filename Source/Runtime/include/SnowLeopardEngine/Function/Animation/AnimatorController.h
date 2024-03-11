#pragma once

#include "SnowLeopardEngine/Function/Animation/Animator.h"

#include "SnowLeopardEngine/Function/Animation/Transition.h"
#include <vector>

namespace SnowLeopardEngine
{
    // class IAgent 
    // {

    // };

    class AnimatorController
    {
    public:
        AnimatorController();

        void RegisterAnimator(Animator* animator);

        void DeleteAnimator(Animator* animator);

        void RegisterTransition(Animator* sourceAnimator, Animator* targetAnimator, int duration, std::string triggerName);

        void DeleteTransition(Animator* sourceAnimator, std::string triggerName);

        void Blending(Animator* sourceAnimator, Animator* targetAnimator);

        void SetEntryAnimator(Animator* animator);

        void SetTrigger(std::string triggerName);

    private:
        Animator* m_CurrentAnimator;
        std::vector<Animator*> m_Animators;
        std::map<Animator*, std::vector<Transition*>> m_TransitionsInfoMap;
    };
} // namespace SnowLeopardEngine