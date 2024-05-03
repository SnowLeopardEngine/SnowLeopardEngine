#include "SnowLeopardEngine/Function/Rendering/GraphicsContext.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Character.h"
#include "SnowLeopardEngine/Platform/Platform.h"

#include <ft2build.h>

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace SnowLeopardEngine
{
    std::map<GLchar, Character> g_Characters;

    static void GLMessageCallback(GLenum source,
                                  GLenum type,
                                  GLuint id,
                                  GLenum severity,
                                  GLsizei /*length*/,
                                  GLchar const* message,
                                  void const* /*userParam*/)
    {
        if (id == 131185 || id == 131218 || id == 0)
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

        bool loadFreeType = LoadFreeType();
        SNOW_LEOPARD_CORE_ASSERT(loadFreeType, "[OpenGLContext] Failed to initialize FreeType!");

        SNOW_LEOPARD_PROFILE_GL_INIT_CONTEXT;
    }

    void GraphicsContext::Shutdown() {}

    void GraphicsContext::SwapBuffers() { g_EngineContext->WindowSys->SwapBuffers(); }

    void GraphicsContext::SetVSync(bool vsyncEnabled) { glfwSwapInterval(vsyncEnabled); }

    int GraphicsContext::LoadGL() { return gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)); }

    int GraphicsContext::GetMinMajor() { return SNOW_LEOPARD_RENDER_API_OPENGL_MIN_MAJOR; }

    int GraphicsContext::GetMinMinor() { return SNOW_LEOPARD_RENDER_API_OPENGL_MIN_MINOR; }

    bool GraphicsContext::LoadFreeType()
    {
        FT_Library ft;
        if (FT_Init_FreeType(&ft))
        {
            SNOW_LEOPARD_CORE_ERROR("[OpenGLContext] Could not init FreeType Library");
            return false;
        }

        FT_Face face;
        if (FT_New_Face(ft, "Assets/Fonts/Jaro-Regular.ttf", 0, &face))
        {
            SNOW_LEOPARD_CORE_ERROR("[OpenGLContext] Failed to load font Jaro-Regular");
            return false;
        }

        FT_Set_Pixel_Sizes(face, 0, 48);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

        g_Characters.clear();
        for (unsigned char c = 0; c < 128; ++c)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                SNOW_LEOPARD_CORE_ERROR("[OpenGLContext] Failed to load Glyph");
                continue;
            }

            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         GL_RED,
                         face->glyph->bitmap.width,
                         face->glyph->bitmap.rows,
                         0,
                         GL_RED,
                         GL_UNSIGNED_BYTE,
                         face->glyph->bitmap.buffer);

            glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Character character = {texture,
                                   glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                                   glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                                   static_cast<GLuint>(face->glyph->advance.x)};
            g_Characters.insert(std::pair<GLchar, Character>(c, character));
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        return true;
    }
} // namespace SnowLeopardEngine