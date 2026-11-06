#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    class GraphicsContext
    {
    public:
        void Init();
        void Shutdown();

        void SwapBuffers();
        void SetVSync(bool vsyncEnabled);

        // Writes the current back buffer (the frame about to be presented) to a PNG file.
        bool CaptureBackBuffer(const std::filesystem::path& pngPath);

        bool IsSupportDSA() const { return m_SupportDSA; }

    private:
        int  LoadGL();
        int  GetMinMajor();
        int  GetMinMinor();

    protected:
        bool m_SupportDSA;
    };
} // namespace SnowLeopardEngine