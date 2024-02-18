#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsContext.h"

namespace SnowLeopardEngine
{
    class GLContextBase : public GraphicsContext
    {
    public:
        virtual void Init();
        virtual void Shutdown();

        virtual void SwapBuffers();
        virtual void SetVSync(bool vsyncEnabled);

        virtual int                LoadGL()      = 0;
        virtual int                GetMinMajor() = 0;
        virtual int                GetMinMinor() = 0;
        virtual const std::string& GetName()     = 0;
    };
} // namespace SnowLeopardEngine