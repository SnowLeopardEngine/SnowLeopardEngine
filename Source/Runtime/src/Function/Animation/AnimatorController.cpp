#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Animation/Transition.h"

namespace SnowLeopardEngine
{
    AnimatorController::AnimatorController()
    {
        m_CurrentAnimationClip = nullptr;
        m_Parameters = CreateRef<std::map<std::string, std::variant<float, bool, std::monostate>>>();
    }

    void AnimatorController::RegisterAnimationClip(const Ref<AnimationClip>& animationClip) { m_AnimationClips.push_back(animationClip); }

    void AnimatorController::DeleteAnimationClip(const Ref<AnimationClip>& animationClip)
    {
        auto it = std::find(m_AnimationClips.begin(), m_AnimationClips.end(), animationClip);
        if (it != m_AnimationClips.end())
        {
            m_AnimationClips.erase(it);
            m_TransitionsInfoMap.erase(animationClip);
        }
        else 
        {
            SNOW_LEOPARD_CORE_ERROR("[AnimatorController] Failed to delete animationClip, AnimationClip haven't been registered.");
        }
    }

    void AnimatorController::SetEntryAnimationClip(const Ref<AnimationClip>& animationClip) { m_CurrentAnimationClip = animationClip; }

    Ref<Transition> AnimatorController::RegisterTransition(const Ref<AnimationClip>& sourceAnimationClip,
                                                const Ref<AnimationClip>& targetAnimationClip,
                                                float                  duration)
    {
        auto transition = CreateRef<Transition>(sourceAnimationClip, targetAnimationClip, duration);
        transition->SetParameters(m_Parameters);
        m_TransitionsInfoMap[sourceAnimationClip].push_back(transition);
        return transition;
    }

    void AnimatorController::DeleteTransition(const Ref<AnimationClip>& sourceAnimationClip, const Ref<AnimationClip>& targetAnimationClip)
    {
        if (m_TransitionsInfoMap.count(sourceAnimationClip) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[AnimatorController] Failed to delete transition, transition haven't been registered.");
            return;
        }

        auto it =
            std::find_if(m_TransitionsInfoMap[sourceAnimationClip].begin(),
                         m_TransitionsInfoMap[sourceAnimationClip].end(),
                         [targetAnimationClip](Ref<Transition> t) { return t->GetTargetAnimationClip() == targetAnimationClip; });
        if (it != m_TransitionsInfoMap[sourceAnimationClip].end())
        {
            m_TransitionsInfoMap[sourceAnimationClip].erase(it);
        }
    }

    void AnimatorController::RegisterParameters(const std::string&               parameterName,
                                                const std::variant<float, bool>& value)
    {
        if (m_Parameters->count(parameterName) != 0)
        {
            SNOW_LEOPARD_CORE_ERROR(
                "[AnimatorController] Failed to register parameter, this parameter has been registered.");
            return;
        }

        std::visit([&](auto&& arg) { (*m_Parameters)[parameterName] = arg; }, value);
    }

    void AnimatorController::RegisterParameters(const std::string& parameterName)
    {
        if (m_Parameters->count(parameterName) != 0)
        {
            SNOW_LEOPARD_CORE_ERROR(
                "[AnimatorController] Failed to register parameter, this parameter has been registered.");
            return;
        }
        (*m_Parameters)[parameterName] = std::monostate {};
    }

    void AnimatorController::DeleteParameters(const std::string& parameterName)
    {
        if (m_Parameters->count(parameterName) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[AnimatorController] Failed to delete parameter, unregistered parameter.");
            return;
        }

        m_Parameters->erase(parameterName);
    }
} // namespace SnowLeopardEngine