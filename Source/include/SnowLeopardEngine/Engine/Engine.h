#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Engine/Tickable.h"

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
         * @brief Add a tickable object
         *
         * @param tickable
         */
        void AddTickable(const Ref<Tickable>& tickable) { m_Tickables.emplace_back(tickable); }

        /**
         * @brief Tick for one frame (logic + render)
         *
         * @param deltaTime Delta Time
         */
        void TickOneFrame(float deltaTime);

        /**
         * @brief Tick for one frame in a fixed delta time (physics)
         *
         */
        void FixedTickOneFrame();

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

    private:
        std::vector<Ref<Tickable>> m_Tickables;
    };
} // namespace SnowLeopardEngine
