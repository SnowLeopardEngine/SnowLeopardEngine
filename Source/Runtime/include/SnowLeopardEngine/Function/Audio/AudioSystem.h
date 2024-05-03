#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"

struct ma_engine;

namespace SnowLeopardEngine
{
    class AudioSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(AudioSystem)

        void Play(const std::string& rawAudioFilePath);
        void Stop();

    private:
        ma_engine* m_AudioEngine;
    };
} // namespace SnowLeopardEngine
