# include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Function/Animation/Transition.h"
#include <vector>

namespace SnowLeopardEngine
{
    AnimatorController::AnimatorController()
    {

    }

    void AnimatorController::RegisterAnimator(Animator* animator)
    {
        m_Animators.push_back(animator);
    }

    void AnimatorController::DeleteAnimator(Animator* animator)
    {
        for (auto it = m_Animators.begin(); it != m_Animators.end(); ) 
        {
            if (*it == animator) {
                delete *it; 
                it = m_Animators.erase(it); 
                m_TransitionsInfoMap.erase(animator);
            } else {
                ++it;
            }
        }
    }

    void AnimatorController::RegisterTransition(Animator* sourceAnimator, Animator* targetAnimator, int duration, std::string triggerName)
    {
        Transition transition(sourceAnimator, targetAnimator, duration, triggerName);
        m_TransitionsInfoMap[sourceAnimator].push_back(&transition);
    }

    void AnimatorController::DeleteTransition(Animator* sourceAnimator, std::string triggerName)
    {
        for (auto it = m_TransitionsInfoMap[sourceAnimator].begin(); it != m_TransitionsInfoMap[sourceAnimator].end(); ) 
        {
            if ((*it)->GetTriggerName() == triggerName) {
                delete *it; 
                it = m_TransitionsInfoMap[sourceAnimator].erase(it); 
            } else {
                ++it;
            }
        }
    }

    void AnimatorController::Blending(Animator* sourceAnimator, Animator* targetAnimator)
    {
        //targetAnimator->PlayAnimation();
        m_CurrentAnimator = targetAnimator;
    }

    void AnimatorController::SetEntryAnimator(Animator* animator)
    {
        m_CurrentAnimator = animator;
    }

    void AnimatorController::SetTrigger(std::string triggerName)
    {
        for(auto transition : m_TransitionsInfoMap[m_CurrentAnimator])
        {
            if(transition->GetTriggerName() == triggerName)
            {
                Blending(transition->GetSourceAnimator(), transition->GetTargetAnimator());
            }
        }
    }
}