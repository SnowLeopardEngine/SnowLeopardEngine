#pragma once

#include "SnowLeopardEngine/Function/Animation/Animator.h"

namespace SnowLeopardEngine
{
    class Transition
    {
    public:
        Transition(Ref<Animator>      sourceAnimator,
                   Ref<Animator>      targetAnimator,
                   int                duration,
                   const std::string& triggerName) :
            m_SourceAnimator(sourceAnimator),
            m_TargetAnimator(targetAnimator), m_Duration(duration), m_TriggerName(triggerName)
        {}

        inline const std::string& GetTriggerName() const { return m_TriggerName; }

        inline Ref<Animator> GetSourceAnimator() const { return m_SourceAnimator; }

        inline Ref<Animator> GetTargetAnimator() const { return m_TargetAnimator; }

    private:
        std::string   m_TriggerName;
        Ref<Animator> m_SourceAnimator;
        Ref<Animator> m_TargetAnimator;
        int           m_Duration;
    };
} // namespace SnowLeopardEngine