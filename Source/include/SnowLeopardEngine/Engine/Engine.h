#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Event/WindowEvents.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Engine/LifeTimeComponent.h"

namespace SnowLeopardEngine
{
    struct EngineInitInfo
    {
        WindowInitInfo Window;
    };

    class Engine
    {
    public:
        /**
         * @brief Initialize the engine
         *
         * @return true no error
         * @return false has some errors
         */
        bool Init(const EngineInitInfo& initInfo);

        /**
         * @brief Post initialized the engine
         *
         * @return true
         * @return false
         */
        bool PostInit();

        /**
         * @brief Add a LifeTimeComponent object
         *
         * @param lifeTimeComponent
         */
        void AddLifeTimeComponent(const Ref<LifeTimeComponent>& lifeTimeComponent)
        {
            m_LiftTimeComponents.emplace_back(lifeTimeComponent);
        }

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
         * @brief Event callback when window is resizing
         *
         * @param e
         */
        void OnWindowResize(const WindowResizeEvent& e);

        /**
         * @brief Get the Context
         *
         * @return Ref<EngineContext> the context of engine
         */
        static EngineContext* GetContext();

    private:
        std::vector<Ref<LifeTimeComponent>> m_LiftTimeComponents;

        bool m_IsWindowMinimized = false;

        EventHandler<WindowResizeEvent> m_WindowResizeHandler = [this](const WindowResizeEvent& e) {
            OnWindowResize(e);
        };
    };
} // namespace SnowLeopardEngine
