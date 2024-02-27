#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Function/Window/Window.h"

namespace SnowLeopardEngine
{
    struct WindowSystemInitInfo
    {
        WindowInitInfo Window;
    };

    class WindowSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(WindowSystem, const WindowSystemInitInfo& props)

        /**
         * @brief Get the window type
         *
         * @return WindowType
         */
        WindowType GetType() const { return m_Window->GetType(); }

        /**
         * @brief Called each frame
         *
         * @return true
         * @return false
         */
        bool Tick() { return m_Window->OnTick(); }

        /**
         * @brief Get the width of window
         *
         * @return uint32_t
         */
        uint32_t GetWidth() const { return m_Window->GetWidth(); }

        /**
         * @brief Get the height of window
         *
         * @return uint32_t
         */
        uint32_t GetHeight() const { return m_Window->GetHeight(); }

        /**
         * @brief Whether the window should close or not.
         *
         * @return true
         * @return false
         */
        bool ShouldClose() { return m_Window->ShouldClose(); }

        /**
         * @brief Whether the window is minimized.
         * 
         * @return true 
         * @return false 
         */
        bool IsMinimized() { return m_Window->IsMinimized(); }

        void MakeCurrentContext() { m_Window->MakeCurrentContext(); }
        void SwapBuffers() { m_Window->SwapBuffers(); }

        void SetHideCursor(bool hide) { m_Window->SetHideCursor(hide); }

        void* GetPlatformWindow() const { return m_Window->GetPlatformWindow(); }
        void* GetNativeWindow() const { return m_Window->GetNativeWindow(); }

    private:
        Window* m_Window = nullptr;
    };
} // namespace SnowLeopardEngine
