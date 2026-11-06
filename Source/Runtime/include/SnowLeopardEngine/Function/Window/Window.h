#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

#include <functional>

namespace SnowLeopardEngine
{
    struct WindowInitInfo
    {
        std::string Title      = "SnowLeopardEngine";
        uint32_t    Width      = 1024;
        uint32_t    Height     = 768;
        bool        Resizable  = false;
        bool        Fullscreen = false;

        WindowInitInfo() = default;
        WindowInitInfo(const std::string& title, uint32_t width, uint32_t height, bool resizable, bool fullscreen) :
            Title(title), Width(width), Height(height), Resizable(resizable), Fullscreen(fullscreen)
        {}
    };

    enum class WindowType
    {
        SDL3 = 0,
    };

    class Window
    {
    public:
        // Receives every platform event (an `SDL_Event*` for SDL3) before the engine handles it; used by the editor's
        // ImGui backend.
        using NativeEventListener   = std::function<void(const void* nativeEvent)>;
        using NativeEventListenerId = uint32_t;

        virtual ~Window() = default;

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

        // Display scale of the window (1 = 96 dpi on Windows).
        virtual float GetContentScale() const = 0;

        virtual bool ShouldClose() const = 0;
        virtual bool IsMinimized() const = 0;

        virtual void SetHideCursor(bool hide) = 0;

        virtual void* GetPlatformWindow() const = 0;
        virtual void* GetNativeWindow() const   = 0;

        virtual NativeEventListenerId AddNativeEventListener(NativeEventListener listener) = 0;
        virtual void                  RemoveNativeEventListener(NativeEventListenerId id)  = 0;
    };
} // namespace SnowLeopardEngine
