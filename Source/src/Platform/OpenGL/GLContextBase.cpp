#include "SnowLeopardEngine/Platform/OpenGL/GLContextBase.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

namespace SnowLeopardEngine
{
    void GLContextBase::Init()
    {
        std::string tag = "[" + GetName() + "Context]";

        g_EngineContext->WindowSys->MakeCurrentContext();
        int version = LoadGL();

        SNOW_LEOPARD_CORE_ASSERT(version, "[{0}] Failed to initialize Glad!", tag);
        if (version)
        {
            auto glName   = GetName();
            int  minMajor = GetMinMajor();
            int  minMinor = GetMinMinor();
            SNOW_LEOPARD_CORE_INFO("[{0}Context] Loaded {1} {2}.{3}", glName, glName, GLVersion.major, GLVersion.minor);

            std::stringstream ss;
            ss << "    Vendor:       " << glGetString(GL_VENDOR) << std::endl;
            ss << "    Version:      " << glGetString(GL_VERSION) << std::endl;
            ss << "    Renderer:     " << glGetString(GL_RENDERER) << std::endl;
            ss << "    GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
            SNOW_LEOPARD_CORE_INFO("[{0}Context] GL Context Info:\n{1}", glName, ss.str());

            SNOW_LEOPARD_CORE_ASSERT(GLVersion.major > minMajor ||
                                         (GLVersion.major == minMajor && GLVersion.minor >= minMinor),
                                     "[{0}] SomeEngine requires at least {1} version {2}.{3}",
                                     tag,
                                     glName,
                                     minMajor,
                                     minMinor);
        }

        // Default gl settings
        glEnable(GL_ALPHA_TEST);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void GLContextBase::Shutdown() {}

    void GLContextBase::SwapBuffers() { g_EngineContext->WindowSys->SwapBuffers(); }

    void GLContextBase::SetVSync(bool vsyncEnabled) { glfwSwapInterval(vsyncEnabled); }
} // namespace SnowLeopardEngine