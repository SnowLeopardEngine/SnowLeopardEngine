#pragma once

#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include <string>

namespace SnowLeopardEngine
{
    class Transition
    {
    public:
        Transition(Animator* sourceAnimator, Animator* targetAnimator, int duration, std::string triggerName);

        std::string GetTriggerName();

        Animator* GetSourceAnimator();

        Animator* GetTargetAnimator();

    private:
        std::string m_TriggerName;
        Animator* m_SourceAnimator;
        Animator* m_TargetAnimator;
        int m_Duration;

    };
} // namespace SnowLeopardEngine