#include "SnowLeopardEngine/Function/Audio/AudioSystem.h"
#include "SnowLeopardEngine/Core/Log/Log.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace SnowLeopardEngine
{
    AudioSystem::AudioSystem()
    {
        m_AudioEngine = new ma_engine();
        auto result   = ma_engine_init(NULL, m_AudioEngine);
        SNOW_LEOPARD_CORE_ASSERT(result == MA_SUCCESS, "[AudioSystem] Failed to init miniaudio engine.");
        SNOW_LEOPARD_CORE_INFO("[AudioSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    AudioSystem::~AudioSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[AudioSystem] Shutting Down...");
        ma_engine_uninit(m_AudioEngine);
        delete m_AudioEngine;
        m_AudioEngine = nullptr;
        m_State       = SystemState::ShutdownOk;
    }

    void AudioSystem::Play(const std::string& rawAudioFilePath)
    {
        ma_engine_play_sound(m_AudioEngine, rawAudioFilePath.c_str(), NULL);
    }
} // namespace SnowLeopardEngine
