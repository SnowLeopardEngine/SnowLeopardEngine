#pragma once

#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"

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
        Transition(Ref<AnimationClip> sourceAnimationClip, Ref<AnimationClip> targetAnimationClip, int duration) :
            m_SourceAnimationClip(sourceAnimationClip), m_TargetAnimationClip(targetAnimationClip), m_Duration(duration)
        {}

        Transition() = default;

        inline Ref<AnimationClip> GetSourceAnimationClip() const { return m_SourceAnimationClip; }

        inline Ref<AnimationClip> GetTargetAnimationClip() const { return m_TargetAnimationClip; }

        inline int GetDuration() const { return m_Duration; }

        bool JudgeCondition(bool isTrigger) const;

        void SetParameters(Ref<std::map<std::string, std::variant<float, bool, std::monostate>>> parameter);

        void SetConditions(std::string const& name, ConditionOperator const& op, bool value);

        void SetConditions(std::string const& name, ConditionOperator const& op, float value);

        void SetConditions(std::string const& name);

    private:
        Ref<std::map<std::string, std::variant<float, bool, std::monostate>>> m_Parameters;
        std::vector<TupleType>                                                m_Conditions;
        Ref<AnimationClip>                                                    m_SourceAnimationClip;
        Ref<AnimationClip>                                                    m_TargetAnimationClip;
        int                                                                   m_Duration;
    };
} // namespace SnowLeopardEngine