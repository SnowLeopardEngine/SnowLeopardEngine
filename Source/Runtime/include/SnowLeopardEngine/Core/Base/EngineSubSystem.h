#pragma once

#include "SnowLeopardEngine/Core/Base/SingletonWrapper.h"

namespace SnowLeopardEngine
{
    class EngineSubSystem
    {
    public:
        enum class SystemState
        {
            NoInit,
            InitOk,
            ShutdownOk,
            Error
        };

    public:
        SNOW_LEOPARD_DEFAULT_NO_COPY_MOVE_CONSTRUCTOR(EngineSubSystem)

        virtual ~EngineSubSystem() = default;

        /**
         * @brief Get the name of the sub system.
         * 
         * @return std::string 
         */
        inline virtual std::string GetName() const { return "UndefinedRuntimeSystem"; }

        /**
         * @brief Get the state of the sub system.
         * 
         * @return SystemState 
         */
        virtual SystemState        GetState() const { return m_State; }

    protected:
        SystemState m_State = SystemState::NoInit;
    };

#define DECLARE_SUBSYSTEM(className, ...) \
    SNOW_LEOPARD_NO_COPY_MOVE_CONSTRUCTOR(className) \
    friend SingletonWrapper<className>; \
\
private: \
    className(__VA_ARGS__); \
    ~className() override; \
\
public: \
    std::string GetName() const override \
    { \
        return #className; \
    }
} // namespace SnowLeopardEngine
