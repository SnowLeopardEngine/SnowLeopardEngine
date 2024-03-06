#include "SnowLeopardEngine/Function/Rendering/GraphicsContext.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLContext.h"

namespace SnowLeopardEngine
{
    Ref<GraphicsContext> GraphicsContext::Create() { return CreateRef<GLContext>(); }
} // namespace SnowLeopardEngine