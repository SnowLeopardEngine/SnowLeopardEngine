#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLAPI.h"

namespace SnowLeopardEngine
{
    Ref<GraphicsAPI> GraphicsAPI::Create(GraphicsBackend backend)
    {
        if (backend == GraphicsBackend::OpenGL)
        {
            return CreateRef<OpenGLAPI>();
        }

        SNOW_LEOPARD_CORE_ASSERT(false, "[GraphicsAPI] Unsuppored API!");
        return nullptr;
    }
} // namespace SnowLeopardEngine