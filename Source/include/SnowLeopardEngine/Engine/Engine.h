#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    class Engine
    {
    public:
        /**
         * @brief Initialize the engine
         *
         * @return true no error
         * @return false has some errors
         */
        bool Init();

        /**
         * @brief Tick for one frame (logic + render)
         *
         * @param deltaTime Delta Time
         */
        void TickOneFrame(float deltaTime);

        /**
         * @brief Cleanup resources and shutdown the engine
         *
         */
        void Shutdown();

        /**
         * @brief Get the Context
         *
         * @return Ref<EngineContext> the context of engine
         */
        static EngineContext* GetContext();
    };
} // namespace SnowLeopardEngine
