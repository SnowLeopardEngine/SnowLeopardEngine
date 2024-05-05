#include "SnowLeopardEngine/Function/Animation/Transition.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    bool Transition::JudgeCondition() const
    {
        if (!m_TriggerSet.empty() && m_Conditions.empty())
        {
            return false;
        }

        for (const auto& condition : m_Conditions)
        {
            if (!get<1>(condition).has_value())
                return false;

            const auto& name = get<0>(condition);

            // Skip trigger
            if (m_TriggerSet.count(name) > 0)
                return false;

            auto value          = m_Parameters->at(name);
            auto conditionValue = get<2>(condition);

            if (std::holds_alternative<bool>(value))
            {
                switch (get<1>(condition).value())
                {
                    case Equal: {
                        if (std::get<bool>(conditionValue) != std::get<bool>(value))
                            return false;
                        break;
                    }
                    case NotEqual: {
                        if (std::get<bool>(conditionValue) == std::get<bool>(value))
                            return false;
                        break;
                    }
                    default:
                        break;
                }
            }
            else if (std::holds_alternative<float>(value))
            {
                switch (get<1>(condition).value())
                {
                    case Equal: {
                        if (std::abs(std::get<float>(conditionValue) - std::get<float>(value)) > 1e-6)
                            return false;
                        break;
                    }
                    case NotEqual: {
                        if (std::abs(std::get<float>(conditionValue) - std::get<float>(value)) < 1e-6)
                            return false;
                        break;
                    }
                    case Greater: {
                        if (std::get<float>(conditionValue) >= std::get<float>(value))
                            return false;
                        break;
                    }
                    case GreaterEqual: {
                        if (std::get<float>(conditionValue) > std::get<float>(value))
                            return false;
                        break;
                    }
                    case Less: {
                        if (std::get<float>(conditionValue) <= std::get<float>(value))
                            return false;
                        break;
                    }
                    case LessEqual: {
                        if (std::get<float>(conditionValue) < std::get<float>(value))
                            return false;
                        break;
                    }
                }
            }
        }

        return true;
    }

    bool Transition::HasTrigger(const std::string& triggerName) const { return m_TriggerSet.count(triggerName) > 0; }

    void Transition::AddTrigger(const std::string& triggerName) { m_TriggerSet.insert(triggerName); }

    void Transition::SetParameters(Ref<std::map<std::string, std::variant<float, bool, std::monostate>>> parameter)
    {
        m_Parameters = parameter;
    }

    void Transition::SetConditions(std::string const& name, ConditionOperator const& op, bool value)
    {
        if (m_Parameters->count(name) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[Transition] Failed to set condition, unregistered parameter.");
            return;
        }

        if (std::holds_alternative<bool>(m_Parameters->at(name)))
        {
            if (op == ConditionOperator::Equal || op == ConditionOperator::NotEqual)
            {
                m_Conditions.emplace_back(name, op, value);
            }
            else
            {
                SNOW_LEOPARD_CORE_ERROR("[Transition] Invalid condition operator: {0}", op);
            }
        }
        else
        {
            SNOW_LEOPARD_CORE_ERROR("[Transition] Invalid parameter type: {0}", name);
        }
    }

    void Transition::SetConditions(std::string const& name, ConditionOperator const& op, float value)
    {
        if (m_Parameters->count(name) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[Transition] Failed to set condition, unregistered parameter.");
            return;
        }

        if (std::holds_alternative<float>(m_Parameters->at(name)))
        {
            m_Conditions.emplace_back(name, op, value);
        }
        else
        {
            SNOW_LEOPARD_CORE_ERROR("[Transition] Invalid parameter type: {0}", name);
        }
    }

    void Transition::SetConditions(std::string const& name)
    {
        if (m_Parameters->count(name) == 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[Transition] Failed to set condition, unregistered parameter.");
            return;
        }

        if (std::holds_alternative<std::monostate>(m_Parameters->at(name)))
        {
            m_Conditions.emplace_back(name, std::nullopt, std::monostate {});
        }
        else
        {
            SNOW_LEOPARD_CORE_ERROR("[Transition] Invalid parameter type: {0}", name);
        }
    }
} // namespace SnowLeopardEngine