#include "SnowLeopardEngine/Function/Audio/AudioSystem.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace SnowLeopardEngine
{
    AudioSystem::AudioSystem()
    {
        m_AudioEngine = new ma_engine();
        auto result   = ma_engine_init(NULL, m_AudioEngine);
        SNOW_LEOPARD_CORE_ASSERT(result == MA_SUCCESS, "[AudioSystem] Failed to init miniaudio engine.");
    }

    AudioSystem::~AudioSystem()
    {
        ma_engine_uninit(m_AudioEngine);
        delete m_AudioEngine;
        m_AudioEngine = nullptr;
    }

    void AudioSystem::Play(const std::string& rawAudioFilePath)
    {
        ma_engine_play_sound(m_AudioEngine, rawAudioFilePath.c_str(), NULL);
    }
} // namespace SnowLeopardEngine
