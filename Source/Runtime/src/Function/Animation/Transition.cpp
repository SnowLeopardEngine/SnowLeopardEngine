# include "SnowLeopardEngine/Function/Animation/Transition.h"

namespace SnowLeopardEngine
{
    Transition::Transition(Animator* sourceAnimator, Animator* targetAnimator, int duration, std::string triggerName)
    {
        m_SourceAnimator = sourceAnimator;
        m_TargetAnimator = targetAnimator;
        m_Duration = duration;
        m_TriggerName = triggerName;
    }

    std::string Transition::GetTriggerName()
    {
        return m_TriggerName;
    }

    Animator* Transition::GetSourceAnimator()
    {
        return m_SourceAnimator;
    }

    Animator* Transition::GetTargetAnimator()
    {
        return m_TargetAnimator;
    }
}