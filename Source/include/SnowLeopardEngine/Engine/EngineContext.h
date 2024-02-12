#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/SingletonWrapper.h"
#include "SnowLeopardEngine/Core/Event/EventSystem.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Function/Window/WindowSystem.h"

namespace SnowLeopardEngine
{
    struct EngineContext
    {
        SingletonWrapper<LogSystem>    LogSys;
        SingletonWrapper<EventSystem>  EventSys;
        SingletonWrapper<WindowSystem> WindowSys;
    };

    extern EngineContext* g_EngineContext;
} // namespace SnowLeopardEngine
