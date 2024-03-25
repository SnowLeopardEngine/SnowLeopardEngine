#pragma once

#include "SnowLeopardEngine/Function/Animation/Animator.h"

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
        Transition(Ref<Animator> sourceAnimator, Ref<Animator> targetAnimator, int duration) :
            m_SourceAnimator(sourceAnimator), m_TargetAnimator(targetAnimator), m_Duration(duration)
        {}

        inline Ref<Animator> GetSourceAnimator() const { return m_SourceAnimator; }

        inline Ref<Animator> GetTargetAnimator() const { return m_TargetAnimator; }

        inline int GetDuration() const { return m_Duration; }

        bool JudgeCondition(bool isTrigger) const;

        void SetParameters(Ref<std::map<std::string, std::variant<float, bool, std::monostate>>> parameter);

        void SetConditions(std::string const& name, ConditionOperator const& op, bool value);

        void SetConditions(std::string const& name, ConditionOperator const& op, float value);

        void SetConditions(std::string const& name);

    private:
        Ref<std::map<std::string, std::variant<float, bool, std::monostate>>> m_Parameters;
        std::vector<TupleType>                                                m_Conditions;
        Ref<Animator>                                                         m_SourceAnimator;
        Ref<Animator>                                                         m_TargetAnimator;
        int                                                                   m_Duration;
    };
} // namespace SnowLeopardEngine