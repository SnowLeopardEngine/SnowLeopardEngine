#pragma once

#include "SnowLeopardEngine/Function/Window/Window.h"

// SDL stays out of public headers; only SDL3Window.cpp includes it.
struct SDL_Window;
struct SDL_GLContextState;

namespace SnowLeopardEngine
{
    class SDL3Window final : public Window
    {
    public:
        WindowType GetType() override { return WindowType::SDL3; }

        void Init(const WindowInitInfo& initInfo) override;
        void Shutdown() override;
        bool OnTick() override;

        uint32_t GetWidth() const override { return m_Width; }
        uint32_t GetHeight() const override { return m_Height; }
        float    GetContentScale() const override;

        bool ShouldClose() const override { return m_ShouldClose; }
        bool IsMinimized() const override { return m_IsMinimized; }

        void MakeCurrentContext() override;
        void SwapBuffers() override;
        void SetVSync(bool enabled) override;
        void* GetProcAddressLoader() const override;

        void SetHideCursor(bool hide) override;

        void* GetPlatformWindow() const override { return m_Window; }
        void* GetNativeWindow() const override;
        void* GetGraphicsContext() const override { return m_GLContext; }

        NativeEventListenerId AddNativeEventListener(NativeEventListener listener) override;
        void                  RemoveNativeEventListener(NativeEventListenerId id) override;

    private:
        void UpdatePixelSize();

    private:
        SDL_Window*         m_Window    = nullptr;
        SDL_GLContextState* m_GLContext = nullptr;

        uint32_t m_Width       = 0;
        uint32_t m_Height      = 0;
        bool     m_ShouldClose = false;
        bool     m_IsMinimized = false;

        // With a hidden (relative-mode) cursor SDL reports motion deltas only; the engine keeps an unbounded virtual
        // position, like GLFW's disabled cursor, so position-based camera code keeps working.
        bool  m_RelativeMouse = false;
        float m_MouseX        = 0.0f;
        float m_MouseY        = 0.0f;

        std::vector<std::pair<NativeEventListenerId, NativeEventListener>> m_Listeners;
        NativeEventListenerId                                              m_NextListenerId = 1;
    };
} // namespace SnowLeopardEngine
