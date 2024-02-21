#include "SnowLeopardEngine/Platform/OpenGL/GLContext.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

namespace SnowLeopardEngine
{
    int GLContext::LoadGL() { return gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)); }
} // namespace SnowLeopardEngine