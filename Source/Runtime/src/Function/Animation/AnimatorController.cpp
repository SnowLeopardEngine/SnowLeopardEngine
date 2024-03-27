#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Function/Animation/Transition.h"

namespace SnowLeopardEngine
{
    void AnimatorController::RegisterAnimator(const Ref<Animator>& animator) { m_Animators.push_back(animator); }

    void AnimatorController::DeleteAnimator(const Ref<Animator>& animator)
    {
        auto it = std::find(m_Animators.begin(), m_Animators.end(), animator);
        if (it != m_Animators.end())
        {
            m_Animators.erase(it);
            m_TransitionsInfoMap.erase(animator);
        }
    }

    void AnimatorController::RegisterTransition(const Ref<Animator>& sourceAnimator,
                                                const Ref<Animator>& targetAnimator,
                                                int                  duration,
                                                std::string          triggerName)
    {
        auto transition = CreateRef<Transition>(sourceAnimator, targetAnimator, duration, triggerName);
        m_TransitionsInfoMap[sourceAnimator].push_back(transition);
    }

    void AnimatorController::DeleteTransition(const Ref<Animator>& sourceAnimator, std::string triggerName)
    {
        if (m_TransitionsInfoMap.count(sourceAnimator) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[AnimatorController] Failed to delete transition, unregisterd animator.");
            return;
        }

        auto it = std::find_if(m_TransitionsInfoMap[sourceAnimator].begin(),
                               m_TransitionsInfoMap[sourceAnimator].end(),
                               [triggerName](Ref<Transition> t) { return t->GetTriggerName() == triggerName; });
        if (it != m_TransitionsInfoMap[sourceAnimator].end())
        {
            m_TransitionsInfoMap[sourceAnimator].erase(it);
        }
    }

    void AnimatorController::Blending(const Ref<Animator>& sourceAnimator, const Ref<Animator>& targetAnimator)
    {
        // targetAnimator->PlayAnimation();
        m_CurrentAnimator = targetAnimator;
    }

    void AnimatorController::SetEntryAnimator(const Ref<Animator>& animator) { m_CurrentAnimator = animator; }

    void AnimatorController::SetTrigger(std::string triggerName)
    {
        for (const auto& transition : m_TransitionsInfoMap[m_CurrentAnimator])
        {
            if (transition->GetTriggerName() == triggerName)
            {
                Blending(transition->GetSourceAnimator(), transition->GetTargetAnimator());
            }
        }
    }

    void AnimatorController::InitAnimators()
    {
        for (const auto& animator : m_Animators)
        {
            animator->Reset();
            animator->Update(0);
        }
    }

    void AnimatorController::UpdateAnimators(float deltaTime)
    {
        for (const auto& animator : m_Animators)
        {
            animator->Update(deltaTime);
        }
    }
} // namespace SnowLeopardEngine