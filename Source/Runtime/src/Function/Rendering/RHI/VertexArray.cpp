#include "SnowLeopardEngine/Function/Rendering/RHI/VertexArray.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLVertexArray.h"

namespace SnowLeopardEngine
{
    Ref<VertexArray> VertexArray::Create() { return CreateRef<GLVertexArray>(); }
} // namespace SnowLeopardEngine