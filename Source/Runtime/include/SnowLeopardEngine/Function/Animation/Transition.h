#pragma once

#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include <vector>

namespace SnowLeopardEngine
{
    enum ConditionOperator
    {
        Equal,
        NotEqual,
        Greater,
        GreaterEqual,
        Less,
        LessEqual
    };

    using ThirdElement = std::variant<float, bool, std::monostate>;
    using TupleType    = std::tuple<std::string, std::optional<ConditionOperator>, ThirdElement>;

    class Transition
    {
    public:
        Transition(Ref<AnimationClip> sourceAnimationClip, Ref<AnimationClip> targetAnimationClip, float duration) :
            m_SourceAnimationClip(sourceAnimationClip), m_TargetAnimationClip(targetAnimationClip), m_Duration(duration)
        {}

        Transition() = default;

        inline Ref<AnimationClip> GetSourceAnimationClip() const { return m_SourceAnimationClip; }

        inline Ref<AnimationClip> GetTargetAnimationClip() const { return m_TargetAnimationClip; }

        inline float GetDuration() const { return m_Duration; }

        bool JudgeCondition() const;

        bool HasTrigger(const std::string& triggerName) const;
        void AddTrigger(const std::string& triggerName);

        void SetParameters(Ref<std::map<std::string, std::variant<float, bool, std::monostate>>> parameter);

        void SetConditions(std::string const& name, ConditionOperator const& op, bool value);

        void SetConditions(std::string const& name, ConditionOperator const& op, float value);

        void SetConditions(std::string const& name);

    private:
        Ref<std::map<std::string, std::variant<float, bool, std::monostate>>> m_Parameters;
        std::vector<TupleType>                                                m_Conditions;
        Ref<AnimationClip>                                                    m_SourceAnimationClip;
        Ref<AnimationClip>                                                    m_TargetAnimationClip;
        float                                                                 m_Duration;
        std::unordered_set<std::string>                                       m_TriggerSet;
    };
} // namespace SnowLeopardEngine