#include "SnowLeopardEngine/Platform/SDL3/SDL3Window.h"
#include "SnowLeopardEngine/Core/Event/WindowEvents.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Input/Input.h"
#include "SnowLeopardEngine/Platform/Platform.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

namespace SnowLeopardEngine
{
    namespace
    {
        uint32_t     g_WindowCount = 0;
        SDL_Gamepad* g_Gamepad     = nullptr; // the first connected gamepad feeds the InputSystem

        // SDL scancodes are positional (US layout names), like the GLFW key codes `KeyCode` keeps its values from.
        int ToKeyCode(SDL_Scancode scancode)
        {
            if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
            {
                return static_cast<int>(KeyCode::A) + (scancode - SDL_SCANCODE_A);
            }
            if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9)
            {
                return static_cast<int>(KeyCode::D1) + (scancode - SDL_SCANCODE_1);
            }
            if (scancode >= SDL_SCANCODE_F1 && scancode <= SDL_SCANCODE_F12)
            {
                return static_cast<int>(KeyCode::F1) + (scancode - SDL_SCANCODE_F1);
            }
            if (scancode >= SDL_SCANCODE_F13 && scancode <= SDL_SCANCODE_F24)
            {
                return static_cast<int>(KeyCode::F13) + (scancode - SDL_SCANCODE_F13);
            }
            if (scancode >= SDL_SCANCODE_KP_1 && scancode <= SDL_SCANCODE_KP_9)
            {
                return static_cast<int>(KeyCode::KP1) + (scancode - SDL_SCANCODE_KP_1);
            }

            KeyCode key;
            switch (scancode)
            {
                // clang-format off
                case SDL_SCANCODE_0:              key = KeyCode::D0; break;
                case SDL_SCANCODE_RETURN:         key = KeyCode::Enter; break;
                case SDL_SCANCODE_ESCAPE:         key = KeyCode::Escape; break;
                case SDL_SCANCODE_BACKSPACE:      key = KeyCode::Backspace; break;
                case SDL_SCANCODE_TAB:            key = KeyCode::Tab; break;
                case SDL_SCANCODE_SPACE:          key = KeyCode::Space; break;
                case SDL_SCANCODE_MINUS:          key = KeyCode::Minus; break;
                case SDL_SCANCODE_EQUALS:         key = KeyCode::Equal; break;
                case SDL_SCANCODE_LEFTBRACKET:    key = KeyCode::LeftBracket; break;
                case SDL_SCANCODE_RIGHTBRACKET:   key = KeyCode::RightBracket; break;
                case SDL_SCANCODE_BACKSLASH:      key = KeyCode::Backslash; break;
                case SDL_SCANCODE_NONUSHASH:      key = KeyCode::World1; break;
                case SDL_SCANCODE_SEMICOLON:      key = KeyCode::Semicolon; break;
                case SDL_SCANCODE_APOSTROPHE:     key = KeyCode::Apostrophe; break;
                case SDL_SCANCODE_GRAVE:          key = KeyCode::GraveAccent; break;
                case SDL_SCANCODE_COMMA:          key = KeyCode::Comma; break;
                case SDL_SCANCODE_PERIOD:         key = KeyCode::Period; break;
                case SDL_SCANCODE_SLASH:          key = KeyCode::Slash; break;
                case SDL_SCANCODE_CAPSLOCK:       key = KeyCode::CapsLock; break;
                case SDL_SCANCODE_PRINTSCREEN:    key = KeyCode::PrintScreen; break;
                case SDL_SCANCODE_SCROLLLOCK:     key = KeyCode::ScrollLock; break;
                case SDL_SCANCODE_PAUSE:          key = KeyCode::Pause; break;
                case SDL_SCANCODE_INSERT:         key = KeyCode::Insert; break;
                case SDL_SCANCODE_HOME:           key = KeyCode::Home; break;
                case SDL_SCANCODE_PAGEUP:         key = KeyCode::PageUp; break;
                case SDL_SCANCODE_DELETE:         key = KeyCode::Delete; break;
                case SDL_SCANCODE_END:            key = KeyCode::End; break;
                case SDL_SCANCODE_PAGEDOWN:       key = KeyCode::PageDown; break;
                case SDL_SCANCODE_RIGHT:          key = KeyCode::Right; break;
                case SDL_SCANCODE_LEFT:           key = KeyCode::Left; break;
                case SDL_SCANCODE_DOWN:           key = KeyCode::Down; break;
                case SDL_SCANCODE_UP:             key = KeyCode::Up; break;
                case SDL_SCANCODE_NUMLOCKCLEAR:   key = KeyCode::NumLock; break;
                case SDL_SCANCODE_KP_DIVIDE:      key = KeyCode::KPDivide; break;
                case SDL_SCANCODE_KP_MULTIPLY:    key = KeyCode::KPMultiply; break;
                case SDL_SCANCODE_KP_MINUS:       key = KeyCode::KPSubtract; break;
                case SDL_SCANCODE_KP_PLUS:        key = KeyCode::KPAdd; break;
                case SDL_SCANCODE_KP_ENTER:       key = KeyCode::KPEnter; break;
                case SDL_SCANCODE_KP_0:           key = KeyCode::KP0; break;
                case SDL_SCANCODE_KP_PERIOD:      key = KeyCode::KPDecimal; break;
                case SDL_SCANCODE_KP_EQUALS:      key = KeyCode::KPEqual; break;
                case SDL_SCANCODE_NONUSBACKSLASH: key = KeyCode::World2; break;
                case SDL_SCANCODE_APPLICATION:    key = KeyCode::Menu; break;
                case SDL_SCANCODE_LCTRL:          key = KeyCode::LeftControl; break;
                case SDL_SCANCODE_LSHIFT:         key = KeyCode::LeftShift; break;
                case SDL_SCANCODE_LALT:           key = KeyCode::LeftAlt; break;
                case SDL_SCANCODE_LGUI:           key = KeyCode::LeftSuper; break;
                case SDL_SCANCODE_RCTRL:          key = KeyCode::RightControl; break;
                case SDL_SCANCODE_RSHIFT:         key = KeyCode::RightShift; break;
                case SDL_SCANCODE_RALT:           key = KeyCode::RightAlt; break;
                case SDL_SCANCODE_RGUI:           key = KeyCode::RightSuper; break;
                default:                          return -1;
                    // clang-format on
            }
            return static_cast<int>(key);
        }

        // SDL: 1 = left, 2 = middle, 3 = right, 4 / 5 = X1 / X2. MouseCode: 0 = left, 1 = right, 2 = middle, ...
        int ToMouseCode(uint8_t button)
        {
            switch (button)
            {
                case SDL_BUTTON_LEFT:
                    return static_cast<int>(MouseCode::ButtonLeft);
                case SDL_BUTTON_RIGHT:
                    return static_cast<int>(MouseCode::ButtonRight);
                case SDL_BUTTON_MIDDLE:
                    return static_cast<int>(MouseCode::ButtonMiddle);
                default:
                    return button - 1;
            }
        }

        bool ToGamepadButton(uint8_t button, GamepadButton& out)
        {
            switch (static_cast<SDL_GamepadButton>(button))
            {
                // clang-format off
                case SDL_GAMEPAD_BUTTON_SOUTH:          out = GamepadButton::South; return true;
                case SDL_GAMEPAD_BUTTON_EAST:           out = GamepadButton::East; return true;
                case SDL_GAMEPAD_BUTTON_WEST:           out = GamepadButton::West; return true;
                case SDL_GAMEPAD_BUTTON_NORTH:          out = GamepadButton::North; return true;
                case SDL_GAMEPAD_BUTTON_BACK:           out = GamepadButton::Back; return true;
                case SDL_GAMEPAD_BUTTON_GUIDE:          out = GamepadButton::Guide; return true;
                case SDL_GAMEPAD_BUTTON_START:          out = GamepadButton::Start; return true;
                case SDL_GAMEPAD_BUTTON_LEFT_STICK:     out = GamepadButton::LeftStick; return true;
                case SDL_GAMEPAD_BUTTON_RIGHT_STICK:    out = GamepadButton::RightStick; return true;
                case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:  out = GamepadButton::LeftShoulder; return true;
                case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: out = GamepadButton::RightShoulder; return true;
                case SDL_GAMEPAD_BUTTON_DPAD_UP:        out = GamepadButton::DPadUp; return true;
                case SDL_GAMEPAD_BUTTON_DPAD_DOWN:      out = GamepadButton::DPadDown; return true;
                case SDL_GAMEPAD_BUTTON_DPAD_LEFT:      out = GamepadButton::DPadLeft; return true;
                case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     out = GamepadButton::DPadRight; return true;
                default:                                return false;
                    // clang-format on
            }
        }

        bool ToGamepadAxis(uint8_t axis, GamepadAxis& out)
        {
            switch (static_cast<SDL_GamepadAxis>(axis))
            {
                // clang-format off
                case SDL_GAMEPAD_AXIS_LEFTX:         out = GamepadAxis::LeftX; return true;
                case SDL_GAMEPAD_AXIS_LEFTY:         out = GamepadAxis::LeftY; return true;
                case SDL_GAMEPAD_AXIS_RIGHTX:        out = GamepadAxis::RightX; return true;
                case SDL_GAMEPAD_AXIS_RIGHTY:        out = GamepadAxis::RightY; return true;
                case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:  out = GamepadAxis::LeftTrigger; return true;
                case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: out = GamepadAxis::RightTrigger; return true;
                default:                             return false;
                    // clang-format on
            }
        }

        // Returns true when a gamepad is open afterwards.
        bool OpenFirstGamepad()
        {
            if (g_Gamepad != nullptr)
            {
                return true;
            }

            int             count = 0;
            SDL_JoystickID* ids   = SDL_GetGamepads(&count);
            if (ids != nullptr && count > 0)
            {
                g_Gamepad = SDL_OpenGamepad(ids[0]);
                if (g_Gamepad != nullptr)
                {
                    SNOW_LEOPARD_CORE_INFO("[SDL3Window] Gamepad connected: {0}", SDL_GetGamepadName(g_Gamepad));
                }
            }
            SDL_free(ids);
            return g_Gamepad != nullptr;
        }
    } // namespace

    void SDL3Window::Init(const WindowInitInfo& initInfo)
    {
        SNOW_LEOPARD_CORE_INFO("[SDL3Window] Creating window, name: {0}, resolution: {1} x {2}",
                               initInfo.Title,
                               initInfo.Width,
                               initInfo.Height);

        if (g_WindowCount == 0)
        {
            const bool result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
            SNOW_LEOPARD_CORE_ASSERT(result, "[SDL3Window] SDL_Init failed: {0}", SDL_GetError());
        }

        // No graphics context: the render system creates a VRI (Vulkan) swapchain from the native window.
        SDL_WindowFlags flags   = SDL_WINDOW_HIGH_PIXEL_DENSITY;
        const bool      offline = initInfo.IsOffline();
        int             width   = static_cast<int>(initInfo.Width);
        int             height  = static_cast<int>(initInfo.Height);
        if (initInfo.Resizable && !offline)
        {
            flags |= SDL_WINDOW_RESIZABLE;
        }
        if (offline)
        {
            // Never shown. A fullscreen app keeps the desktop resolution so offline frames match on-screen ones.
            flags |= SDL_WINDOW_HIDDEN;
            if (initInfo.Fullscreen)
            {
                if (const auto* mode = SDL_GetDesktopDisplayMode(SDL_GetPrimaryDisplay()); mode != nullptr)
                {
                    width  = mode->w;
                    height = mode->h;
                }
            }
        }
        else if (initInfo.Fullscreen)
        {
            flags |= SDL_WINDOW_FULLSCREEN; // borderless, at the desktop resolution
        }

        m_Window = SDL_CreateWindow(initInfo.Title.c_str(), width, height, flags);
        SNOW_LEOPARD_CORE_ASSERT(m_Window, "[SDL3Window] SDL_CreateWindow failed: {0}", SDL_GetError());
        ++g_WindowCount;

        // Where window coordinates are pixels (Windows, X11), scale a windowed size by the display scale, as GLFW's
        // GLFW_SCALE_TO_MONITOR did. Where they are points (macOS, Wayland) the OS already does it.
        if (!initInfo.Fullscreen && !offline && SDL_GetWindowPixelDensity(m_Window) <= 1.0f)
        {
            const float scale = GetContentScale();
            if (scale > 1.0f)
            {
                SDL_SetWindowSize(m_Window,
                                  static_cast<int>(std::lround(initInfo.Width * scale)),
                                  static_cast<int>(std::lround(initInfo.Height * scale)));
                SDL_SetWindowPosition(m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                SDL_SyncWindow(m_Window);
            }
        }

        UpdatePixelSize();

        // TODO: Set icon

        SNOW_LEOPARD_CORE_INFO("[SDL3Window] Initialized (SDL {0}.{1}.{2}, {3} x {4} pixels, scale {5})",
                               SDL_VERSIONNUM_MAJOR(SDL_GetVersion()),
                               SDL_VERSIONNUM_MINOR(SDL_GetVersion()),
                               SDL_VERSIONNUM_MICRO(SDL_GetVersion()),
                               m_Width,
                               m_Height,
                               GetContentScale());
    }

    void SDL3Window::Shutdown()
    {
        SNOW_LEOPARD_CORE_INFO("[SDL3Window] Shutting Down...");

        m_Listeners.clear();

        if (g_Gamepad != nullptr)
        {
            SDL_CloseGamepad(g_Gamepad);
            g_Gamepad = nullptr;
        }

        if (m_Window != nullptr)
        {
            SDL_DestroyWindow(m_Window);
            m_Window = nullptr;
            --g_WindowCount;
        }

        if (g_WindowCount == 0)
        {
            SDL_Quit();
        }
    }

    bool SDL3Window::OnTick()
    {
        auto& input = g_EngineContext->InputSys;
        input->ClearStates();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            for (auto& [id, listener] : m_Listeners)
            {
                listener(&event);
            }

            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED: {
                    if (!m_ShouldClose)
                    {
                        m_ShouldClose = true;
                        QueueEvent(CreateScope<WindowCloseEvent>());
                    }
                    break;
                }

                case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
                    UpdatePixelSize();
                    if (m_Width > 0 && m_Height > 0)
                    {
                        WindowResizeEvent resizeEvent(m_Width, m_Height);
                        TriggerEvent(resizeEvent);
                    }
                    break;
                }

                case SDL_EVENT_WINDOW_MINIMIZED: {
                    m_IsMinimized = true;
                    WindowMinimizeEvent minimizeEvent;
                    TriggerEvent(minimizeEvent);
                    break;
                }

                case SDL_EVENT_WINDOW_RESTORED:
                case SDL_EVENT_WINDOW_MAXIMIZED:
                    m_IsMinimized = false;
                    break;

                case SDL_EVENT_KEY_DOWN:
                case SDL_EVENT_KEY_UP: {
                    if (event.key.repeat)
                    {
                        break;
                    }
                    const int key = ToKeyCode(event.key.scancode);
                    if (key >= 0)
                    {
                        input->SetKeyState(key,
                                           static_cast<int>(event.key.down ? InputAction::Press : InputAction::Release));
                    }
                    break;
                }

                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    input->SetMouseButtonState(
                        ToMouseCode(event.button.button),
                        static_cast<int>(event.button.down ? InputAction::Press : InputAction::Release));
                    break;

                case SDL_EVENT_MOUSE_WHEEL:
                    input->AddMouseScrollDelta(glm::vec2(event.wheel.x, event.wheel.y));
                    break;

                case SDL_EVENT_MOUSE_MOTION: {
                    // Engine mouse coordinates are framebuffer pixels (GUI hit tests and picking compare them with
                    // the framebuffer size).
                    const float density = SDL_GetWindowPixelDensity(m_Window);
                    if (m_RelativeMouse)
                    {
                        m_MouseX += event.motion.xrel * density;
                        m_MouseY += event.motion.yrel * density;
                    }
                    else
                    {
                        m_MouseX = event.motion.x * density;
                        m_MouseY = event.motion.y * density;
                    }
                    input->SetMousePosition(glm::vec2(m_MouseX, m_MouseY));
                    input->SetMousePositionFlipY(glm::vec2(m_MouseX, static_cast<float>(m_Height) - m_MouseY));
                    break;
                }

                case SDL_EVENT_GAMEPAD_ADDED:
                    if (OpenFirstGamepad())
                    {
                        input->SetGamepadConnected(true);
                    }
                    break;

                case SDL_EVENT_GAMEPAD_REMOVED: {
                    if (g_Gamepad != nullptr && SDL_GetGamepadID(g_Gamepad) == event.gdevice.which)
                    {
                        SDL_CloseGamepad(g_Gamepad);
                        g_Gamepad = nullptr;
                        input->SetGamepadConnected(OpenFirstGamepad());
                    }
                    break;
                }

                case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
                case SDL_EVENT_GAMEPAD_BUTTON_UP: {
                    GamepadButton button;
                    if (g_Gamepad != nullptr && SDL_GetGamepadID(g_Gamepad) == event.gbutton.which &&
                        ToGamepadButton(event.gbutton.button, button))
                    {
                        input->SetGamepadButtonState(button, event.gbutton.down);
                    }
                    break;
                }

                case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
                    GamepadAxis axis;
                    if (g_Gamepad != nullptr && SDL_GetGamepadID(g_Gamepad) == event.gaxis.which &&
                        ToGamepadAxis(event.gaxis.axis, axis))
                    {
                        // SDL range: sticks [-32768, 32767], triggers [0, 32767].
                        const float value = std::clamp(static_cast<float>(event.gaxis.value) / 32767.0f, -1.0f, 1.0f);
                        input->SetGamepadAxis(axis, value);
                    }
                    break;
                }

                default:
                    break;
            }
        }

        return true;
    }

    float SDL3Window::GetContentScale() const
    {
        const float scale = SDL_GetWindowDisplayScale(m_Window);
        return scale > 0.0f ? scale : 1.0f;
    }

    void SDL3Window::SetHideCursor(bool hide)
    {
        m_RelativeMouse = hide;
        SDL_SetWindowRelativeMouseMode(m_Window, hide);
    }

    void* SDL3Window::GetNativeWindow() const
    {
        const SDL_PropertiesID properties = SDL_GetWindowProperties(m_Window);
#if SNOW_LEOPARD_PLATFORM_WINDOWS
        return SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif SNOW_LEOPARD_PLATFORM_DARWIN
        return SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#elif SNOW_LEOPARD_PLATFORM_LINUX
        if (void* wayland = SDL_GetPointerProperty(properties, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr))
        {
            return wayland;
        }
        return reinterpret_cast<void*>(
            static_cast<uintptr_t>(SDL_GetNumberProperty(properties, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0)));
#else
        return nullptr;
#endif
    }

    Window::NativeEventListenerId SDL3Window::AddNativeEventListener(NativeEventListener listener)
    {
        const auto id = m_NextListenerId++;
        m_Listeners.emplace_back(id, std::move(listener));
        return id;
    }

    void SDL3Window::RemoveNativeEventListener(NativeEventListenerId id)
    {
        std::erase_if(m_Listeners, [id](const auto& entry) { return entry.first == id; });
    }

    void SDL3Window::UpdatePixelSize()
    {
        int width = 0, height = 0;
        SDL_GetWindowSizeInPixels(m_Window, &width, &height);
        m_Width  = static_cast<uint32_t>(std::max(width, 0));
        m_Height = static_cast<uint32_t>(std::max(height, 0));
    }
} // namespace SnowLeopardEngine
