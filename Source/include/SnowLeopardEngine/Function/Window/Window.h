#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    struct WindowInitInfo
    {
        std::string Title  = "SnowLeopardEngine";
        uint32_t    Width  = 1024;
        uint32_t    Height = 768;
        bool        VSync  = true;

        WindowInitInfo() = default;
        WindowInitInfo(const std::string& title, uint32_t width, uint32_t height, bool vsync) :
            Title(title), Width(width), Height(height), VSync(vsync)
        {}
    };

    enum class WindowType
    {
        GLFW = 0,
    };

    class Window
    {
    public:
        /**
         * @brief Get the window type
         *
         * @return WindowType
         */
        virtual WindowType GetType() = 0;

        /**
         * @brief Initialize the window
         *
         * @param props
         */
        virtual void Init(const WindowInitInfo& props) = 0;

        /**
         * @brief Cleanup resources and shutdown the window.
         *
         */
        virtual void Shutdown() = 0;

        /**
         * @brief Called each frame
         *
         * @return true
         * @return false
         */
        virtual bool OnTick() = 0;

        /**
         * @brief Get the width of window
         *
         * @return uint32_t
         */
        virtual uint32_t GetWidth() const = 0;

        /**
         * @brief Get the height of window
         *
         * @return uint32_t
         */
        virtual uint32_t GetHeight() const = 0;

        virtual bool ShouldClose() const = 0;

        virtual void MakeCurrentContext() = 0;
        virtual void SwapBuffers()        = 0;

        virtual void SetHideCursor(bool hide) = 0;

        virtual void* GetPlatformWindow() const = 0;
        virtual void* GetNativeWindow() const   = 0;
    };
} // namespace SnowLeopardEngine
