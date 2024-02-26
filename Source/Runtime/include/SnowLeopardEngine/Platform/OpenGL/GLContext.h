#pragma once

#include "SnowLeopardEngine/Platform/OpenGL/GLContextBase.h"
#include "SnowLeopardEngine/Platform/Platform.h"

namespace SnowLeopardEngine
{
    class GLContext final : public GLContextBase
    {
    public:
        virtual int                LoadGL() override final;
        inline virtual int         GetMinMajor() override final { return SNOW_LEOPARD_RENDER_API_OPENGL_MIN_MAJOR; }
        inline virtual int         GetMinMinor() override final { return SNOW_LEOPARD_RENDER_API_OPENGL_MIN_MINOR; }
        virtual const std::string& GetName() override final { return m_Name; }

    private:
        const std::string m_Name = "OpenGL";
    };
} // namespace SnowLeopardEngine