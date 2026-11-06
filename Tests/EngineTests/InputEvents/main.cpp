// Input regression test for the SDL3 window.
//
// Pushes synthetic SDL events, ticks one engine frame (which polls them) and checks what the InputSystem reports:
// layout-independent key mapping, down / held / up edges, mouse buttons, wheel, motion (absolute and hidden-cursor
// relative), and the close request. Gamepads need a real (or virtual) device and are not covered. Exit code = number of failures.

#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Input/Input.h"

#include <SDL3/SDL.h>

#include <cmath>
#include <cstdio>
#include <string>

using namespace SnowLeopardEngine;

namespace
{
    int g_Failures = 0;

    void Check(bool condition, const std::string& name, const std::string& detail = "")
    {
        std::printf("[%s] %-44s %s\n", condition ? "PASS" : "FAIL", name.c_str(), detail.c_str());
        if (!condition)
        {
            ++g_Failures;
        }
    }

    SDL_WindowID WindowId()
    {
        return SDL_GetWindowID(static_cast<SDL_Window*>(g_EngineContext->WindowSys->GetPlatformWindow()));
    }

    void PushKey(SDL_Scancode scancode, bool down, bool repeat = false)
    {
        SDL_Event event {};
        event.type         = down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        event.key.windowID = WindowId();
        event.key.scancode = scancode;
        event.key.key      = SDL_GetKeyFromScancode(scancode, SDL_KMOD_NONE, false);
        event.key.down     = down;
        event.key.repeat   = repeat;
        SDL_PushEvent(&event);
    }

    void PushMouseButton(uint8_t button, bool down)
    {
        SDL_Event event {};
        event.type            = down ? SDL_EVENT_MOUSE_BUTTON_DOWN : SDL_EVENT_MOUSE_BUTTON_UP;
        event.button.windowID = WindowId();
        event.button.button   = button;
        event.button.down     = down;
        SDL_PushEvent(&event);
    }

    void PushMotion(float x, float y, float dx, float dy)
    {
        SDL_Event event {};
        event.type            = SDL_EVENT_MOUSE_MOTION;
        event.motion.windowID = WindowId();
        event.motion.x        = x;
        event.motion.y        = y;
        event.motion.xrel     = dx;
        event.motion.yrel     = dy;
        SDL_PushEvent(&event);
    }

    void PushWheel(float x, float y)
    {
        SDL_Event event {};
        event.type           = SDL_EVENT_MOUSE_WHEEL;
        event.wheel.windowID = WindowId();
        event.wheel.x        = x;
        event.wheel.y        = y;
        SDL_PushEvent(&event);
    }

    std::string Vec(const glm::vec2& v)
    {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "(%.1f, %.1f)", v.x, v.y);
        return buffer;
    }
} // namespace

int main() TRY
{
    EngineInitInfo initInfo {};
    initInfo.Window.Title = "Test - InputEvents";

    Engine engine;
    if (!engine.Init(initInfo))
    {
        std::fprintf(stderr, "Failed to init engine!\n");
        return 1;
    }
    engine.PostInit();

    auto& input = g_EngineContext->InputSys;
    auto  tick  = [&engine]() { engine.TickOneFrame(1.0f / 60.0f); };
    tick(); // drain window-creation events

    // Keyboard: SDL scancodes map to KeyCode by position.
    PushKey(SDL_SCANCODE_W, true);
    tick();
    Check(input->GetKeyDown(KeyCode::W) && input->GetKey(KeyCode::W), "W pressed: down edge + held");
    Check(!input->GetKey(KeyCode::S), "other keys untouched");

    PushKey(SDL_SCANCODE_W, true, true); // OS key repeat
    tick();
    Check(!input->GetKeyDown(KeyCode::W) && input->GetKey(KeyCode::W), "repeat: no new down edge, still held");

    PushKey(SDL_SCANCODE_W, false);
    tick();
    Check(input->GetKeyUp(KeyCode::W) && !input->GetKey(KeyCode::W), "W released: up edge");
    tick();
    Check(!input->GetKeyUp(KeyCode::W), "edges last one frame");

    struct Mapping
    {
        SDL_Scancode Scancode;
        KeyCode      Key;
        const char*  Name;
    };
    const Mapping mappings[] = {
        {SDL_SCANCODE_SPACE, KeyCode::Space, "Space"},
        {SDL_SCANCODE_ESCAPE, KeyCode::Escape, "Escape"},
        {SDL_SCANCODE_LSHIFT, KeyCode::LeftShift, "LeftShift"},
        {SDL_SCANCODE_0, KeyCode::D0, "0"},
        {SDL_SCANCODE_9, KeyCode::D9, "9"},
        {SDL_SCANCODE_F12, KeyCode::F12, "F12"},
        {SDL_SCANCODE_KP_5, KeyCode::KP5, "Keypad 5"},
        {SDL_SCANCODE_UP, KeyCode::Up, "Up"},
        {SDL_SCANCODE_Z, KeyCode::Z, "Z"},
    };
    for (const auto& mapping : mappings)
    {
        PushKey(mapping.Scancode, true);
        tick();
        Check(input->GetKeyDown(mapping.Key), std::string("maps ") + mapping.Name);
        PushKey(mapping.Scancode, false);
        tick();
    }

    // Mouse buttons: SDL 1 / 2 / 3 = left / middle / right; MouseCode 0 / 1 / 2 = left / right / middle.
    PushMouseButton(SDL_BUTTON_RIGHT, true);
    tick();
    Check(input->GetMouseButtonDown(MouseCode::ButtonRight) && !input->GetMouseButton(MouseCode::ButtonMiddle),
          "right button maps to ButtonRight");
    PushMouseButton(SDL_BUTTON_RIGHT, false);
    PushMouseButton(SDL_BUTTON_MIDDLE, true);
    tick();
    Check(input->GetMouseButtonUp(MouseCode::ButtonRight) && input->GetMouseButtonDown(MouseCode::ButtonMiddle),
          "middle button maps to ButtonMiddle");
    PushMouseButton(SDL_BUTTON_MIDDLE, false);
    tick();

    // Wheel deltas within one frame add up and reset the next frame.
    PushWheel(0, 1);
    PushWheel(0, 2);
    tick();
    Check(input->GetMouseScrollDelta() == glm::vec2(0, 3), "wheel deltas accumulate", Vec(input->GetMouseScrollDelta()));
    tick();
    Check(input->GetMouseScrollDelta() == glm::vec2(0, 0), "wheel resets next frame");

    // Absolute motion, in framebuffer pixels; FlipY measures from the bottom edge.
    const float density = SDL_GetWindowPixelDensity(static_cast<SDL_Window*>(g_EngineContext->WindowSys->GetPlatformWindow()));
    PushMotion(100, 50, 0, 0);
    tick();
    const glm::vec2 expected(100 * density, 50 * density);
    Check(input->GetMousePosition() == expected, "absolute mouse position", Vec(input->GetMousePosition()));
    Check(std::fabs(input->GetMousePositionFlipY().y - (g_EngineContext->WindowSys->GetHeight() - expected.y)) < 0.01f,
          "flipped Y position",
          Vec(input->GetMousePositionFlipY()));

    // Hidden cursor (relative mode): the position keeps integrating deltas, like GLFW's disabled cursor.
    g_EngineContext->WindowSys->SetHideCursor(true);
    PushMotion(0, 0, 10, -4);
    PushMotion(0, 0, 5, 1);
    tick();
    const glm::vec2 relative = expected + glm::vec2(15 * density, -3 * density);
    Check(input->GetMousePosition() == relative, "relative motion integrates deltas", Vec(input->GetMousePosition()));
    g_EngineContext->WindowSys->SetHideCursor(false);

    // Close request -> ShouldClose.
    SDL_Event close {};
    close.type            = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
    close.window.windowID = WindowId();
    SDL_PushEvent(&close);
    tick();
    Check(g_EngineContext->WindowSys->ShouldClose(), "close request sets ShouldClose");

    std::printf("\n%d failure(s)\n", g_Failures);

    engine.Shutdown();
    return g_Failures;
}
CATCH
