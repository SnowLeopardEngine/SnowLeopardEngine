#pragma once

#include "SnowLeopardEngine/Function/Animation/Animator.h"
#include "SnowLeopardEngine/Function/Animation/Transition.h"
#include <variant>

namespace SnowLeopardEngine
{
    class AnimatorController
    {
    public:
        void RegisterAnimator(const Ref<Animator>& animator);
        void DeleteAnimator(const Ref<Animator>& animator);

        void RegisterTransition(const Ref<Animator>& sourceAnimator, const Ref<Animator>& targetAnimator, int duration);

        void DeleteTransition(const Ref<Animator>& sourceAnimator, const Ref<Animator>& targetAnimator);

        void RegisterParameters(const std::string& parameterName, const std::variant<float, bool>& defaultValue);
        void RegisterParameters(const std::string& parameterName);
        void DeleteParameters(const std::string& parameterName);

        void Blending(const Ref<Animator>& sourceAnimator, const Ref<Animator>& targetAnimator);

        void SetEntryAnimator(const Ref<Animator>& animator);

        void SetTrigger(const std::string& triggerName);
        void SetFloat(const std::string& floatName, float value);
        void SetBoolean(const std::string& booleanName, bool value);

        void InitAnimators();
        void UpdateAnimators(float deltaTime);

        void CheckParameters();

        inline Ref<Animator> GetCurrentAnimator() const { return m_CurrentAnimator; }

    private:
        Ref<Animator>                                                         m_CurrentAnimator;
        std::vector<Ref<Animator>>                                            m_Animators;
        std::map<Ref<Animator>, std::vector<Ref<Transition>>>                 m_TransitionsInfoMap;
        Ref<std::map<std::string, std::variant<float, bool, std::monostate>>> m_Parameters;
    };
} // namespace SnowLeopardEngine