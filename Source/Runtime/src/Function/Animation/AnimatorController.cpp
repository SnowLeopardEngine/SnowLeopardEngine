#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Animation/Transition.h"

namespace SnowLeopardEngine
{
    AnimatorController::AnimatorController()
    {
        m_Parameters = CreateRef<std::map<std::string, std::variant<float, bool, std::monostate>>>();
    }

    void AnimatorController::Init() { InitAnimators(); }

    void AnimatorController::Update(float deltaTime)
    {
        CheckParameters();
        UpdateCurrentAnimator(deltaTime);
    }

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

    Ref<Transition> AnimatorController::RegisterTransition(const Ref<Animator>& sourceAnimator,
                                                           const Ref<Animator>& targetAnimator,
                                                           int                  duration)
    {
        auto transition = CreateRef<Transition>(sourceAnimator, targetAnimator, duration);
        transition->SetParameters(m_Parameters);
        m_TransitionsInfoMap[sourceAnimator].push_back(transition);
        return transition;
    }

    void AnimatorController::DeleteTransition(const Ref<Animator>& sourceAnimator, const Ref<Animator>& targetAnimator)
    {
        if (m_TransitionsInfoMap.count(sourceAnimator) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[AnimatorController] Failed to delete transition, unregisterd animator.");
            return;
        }

        auto it =
            std::find_if(m_TransitionsInfoMap[sourceAnimator].begin(),
                         m_TransitionsInfoMap[sourceAnimator].end(),
                         [targetAnimator](Ref<Transition> t) { return t->GetTargetAnimator() == targetAnimator; });
        if (it != m_TransitionsInfoMap[sourceAnimator].end())
        {
            m_TransitionsInfoMap[sourceAnimator].erase(it);
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

    void AnimatorController::Blending(const Ref<Animator>& sourceAnimator, const Ref<Animator>& targetAnimator)
    {
        // targetAnimator->PlayAnimation();
        m_CurrentAnimator = targetAnimator;
        m_CurrentAnimator->Reset();
        m_CurrentAnimator->Update(0);
    }

    void AnimatorController::SetEntryAnimator(const Ref<Animator>& animator) { m_CurrentAnimator = animator; }

    void AnimatorController::SetTrigger(const std::string& triggerName)
    {
        if (m_Parameters->count(triggerName) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[AnimatorController] Failed to set trigger, unregistered parameter.");
            return;
        }

        for (const auto& transition : m_TransitionsInfoMap[m_CurrentAnimator])
        {
            if (transition->HasTrigger(triggerName))
            {
                Blending(transition->GetSourceAnimator(), transition->GetTargetAnimator());
                break;
            }
        }
    }

    void AnimatorController::SetFloat(const std::string& floatName, float value)
    {
        if (m_Parameters->count(floatName) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[AnimatorController] Failed to modify the value, unregistered parameter.");
            return;
        }

        if (std::holds_alternative<float>(m_Parameters->at(floatName)))
        {
            (*m_Parameters)[floatName] = value;
        }
        else
        {
            SNOW_LEOPARD_CORE_ERROR("[AnimatorController] Failed to modify the value, parameter is not float.");
        }
    }

    void AnimatorController::SetBoolean(const std::string& booleanName, bool value)
    {
        if (m_Parameters->count(booleanName) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[AnimatorController] Failed to modify the value, unregistered parameter.");
            return;
        }

        if (std::holds_alternative<bool>(m_Parameters->at(booleanName)))
        {
            (*m_Parameters)[booleanName] = value;
        }
        else
        {
            SNOW_LEOPARD_CORE_ERROR("[AnimatorController] Failed to modify the value, parameter is not bool.");
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

    void AnimatorController::UpdateCurrentAnimator(float deltaTime) { m_CurrentAnimator->Update(deltaTime); }

    void AnimatorController::CheckParameters()
    {
        for (const auto& transition : m_TransitionsInfoMap[m_CurrentAnimator])
        {
            if (transition->JudgeCondition())
            {
                Blending(transition->GetSourceAnimator(), transition->GetTargetAnimator());
                break;
            }
        }
    }
} // namespace SnowLeopardEngine