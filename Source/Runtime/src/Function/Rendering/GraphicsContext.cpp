#include "SnowLeopardEngine/Function/Rendering/GraphicsContext.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Platform/Platform.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

namespace SnowLeopardEngine
{
    static void GLMessageCallback(GLenum source,
                                  GLenum type,
                                  GLuint id,
                                  GLenum severity,
                                  GLsizei /*length*/,
                                  GLchar const* message,
                                  void const* /*userParam*/)
    {
        if (id == 131185 || id == 131218)
        {
            return;
        }

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

    void GraphicsContext::Init()
    {
        g_EngineContext->WindowSys->MakeCurrentContext();
        int version = LoadGL();

        SNOW_LEOPARD_CORE_ASSERT(version, "[OpenGLContext] Failed to initialize Glad!");
        if (version)
        {
            int minMajor = GetMinMajor();
            int minMinor = GetMinMinor();
            SNOW_LEOPARD_CORE_INFO("[OpenGLContext] Loaded {0}.{1}", GLVersion.major, GLVersion.minor);

            std::stringstream ss;
            ss << "    Vendor:       " << glGetString(GL_VENDOR) << std::endl;
            ss << "    Version:      " << glGetString(GL_VERSION) << std::endl;
            ss << "    Renderer:     " << glGetString(GL_RENDERER) << std::endl;
            ss << "    GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
            SNOW_LEOPARD_CORE_INFO("[OpenGLContext] GL Context Info:\n{0}", ss.str());

            SNOW_LEOPARD_CORE_ASSERT(GLVersion.major > minMajor ||
                                         (GLVersion.major == minMajor && GLVersion.minor >= minMinor),
                                     "[OpenGLContext] SomeEngine requires at least OpenGL version {0}.{1}",
                                     minMajor,
                                     minMinor);
        }

        m_SupportDSA = GLAD_GL_VERSION_4_5 || GLAD_GL_VERSION_4_6;

#ifndef NDEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLMessageCallback, nullptr);
#endif
    }

    void GraphicsContext::Shutdown() {}

    void GraphicsContext::SwapBuffers() { g_EngineContext->WindowSys->SwapBuffers(); }

    void GraphicsContext::SetVSync(bool vsyncEnabled) { glfwSwapInterval(vsyncEnabled); }

    int GraphicsContext::LoadGL() { return gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)); }

    int GraphicsContext::GetMinMajor() { return SNOW_LEOPARD_RENDER_API_OPENGL_MIN_MAJOR; }

    int GraphicsContext::GetMinMinor() { return SNOW_LEOPARD_RENDER_API_OPENGL_MIN_MINOR; }
} // namespace SnowLeopardEngine