#include "SnowLeopardEngine/Platform/OpenGL/GLContextBase.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

namespace SnowLeopardEngine
{
    static void GLMessageCallback(GLenum        source,
                                  GLenum        type,
                                  GLuint        id,
                                  GLenum        severity,
                                  GLsizei       length,
                                  GLchar const* message,
                                  void const*   userParam)
    {
        const auto* const srcStr = [source]() {
            switch (source)
            {
                case GL_DEBUG_SOURCE_API:
                    return "API";
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                    return "WINDOW SYSTEM";
                case GL_DEBUG_SOURCE_SHADER_COMPILER:
                    return "SHADER COMPILER";
                case GL_DEBUG_SOURCE_THIRD_PARTY:
                    return "THIRD PARTY";
                case GL_DEBUG_SOURCE_APPLICATION:
                    return "APPLICATION";
                case GL_DEBUG_SOURCE_OTHER:
                    return "OTHER";

                default:
                    return "UNKNOWN_SOURCE";
            }
        }();

        const auto* const typeStr = [type]() {
            switch (type)
            {
                case GL_DEBUG_TYPE_ERROR:
                    return "ERROR";
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                    return "DEPRECATED_BEHAVIOR";
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                    return "UNDEFINED_BEHAVIOR";
                case GL_DEBUG_TYPE_PORTABILITY:
                    return "PORTABILITY";
                case GL_DEBUG_TYPE_PERFORMANCE:
                    return "PERFORMANCE";
                case GL_DEBUG_TYPE_MARKER:
                    return "MARKER";
                case GL_DEBUG_TYPE_OTHER:
                    return "OTHER";

                default:
                    return "UNKNOWN_TYPE";
            }
        }();

        const auto* const severityStr = [severity]() {
            switch (severity)
            {
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                    return "NOTIFICATION";
                case GL_DEBUG_SEVERITY_LOW:
                    return "LOW";
                case GL_DEBUG_SEVERITY_MEDIUM:
                    return "MEDIUM";
                case GL_DEBUG_SEVERITY_HIGH:
                    return "HIGH";

                default:
                    return "UNKNOWN_SEVERITY";
            }
        }();
        SNOW_LEOPARD_CORE_INFO("[OpenGLMessage] Source: {0}, Type: {1}, Severity: {2}, Id: {3}, Message: {4}",
                               srcStr,
                               typeStr,
                               severityStr,
                               id,
                               message);
    }

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

#ifndef NDEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLMessageCallback, nullptr);
#endif
    }

    void GLContextBase::Shutdown() {}

    void GLContextBase::SwapBuffers() { g_EngineContext->WindowSys->SwapBuffers(); }

    void GLContextBase::SetVSync(bool vsyncEnabled) { glfwSwapInterval(vsyncEnabled); }
} // namespace SnowLeopardEngine