#include "SnowLeopardEngine/Function/Audio/AudioSystem.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

namespace SnowLeopardEngine
{
    AudioSystem::AudioSystem()
    {
        m_AudioEngine                 = new ma_engine();
        ma_engine_config engineConfig = ma_engine_config_init();
        engineConfig.listenerCount    = 1;

        auto result = ma_engine_init(&engineConfig, m_AudioEngine);
        SNOW_LEOPARD_CORE_ASSERT(result == MA_SUCCESS, "[AudioSystem] Failed to init miniaudio engine.");
        SNOW_LEOPARD_CORE_INFO("[AudioSystem] Initialized");

        m_State = SystemState::InitOk;
    }

    AudioSystem::~AudioSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[AudioSystem] Shutting Down...");

        for (const auto& clip : m_Clips)
        {
            clip->Release();
        }

        ma_engine_uninit(m_AudioEngine);
        delete m_AudioEngine;
        m_AudioEngine = nullptr;
        m_State       = SystemState::ShutdownOk;
    }

    void AudioSystem::Play(const std::string& rawAudioFilePath)
    {
        auto realPath = FileSystem::GetExecutableDirectory() / rawAudioFilePath;
        ma_engine_play_sound(m_AudioEngine, realPath.generic_string().c_str(), nullptr);
    }

    void AudioSystem::Stop() { ma_engine_stop(m_AudioEngine); }

    Ref<AudioClip> AudioSystem::CreateAudioClip(const std::string& rawAudioFilePath)
    {
        auto clip = CreateRef<AudioClip>(rawAudioFilePath);

        bool loadSuccess = clip->Init();
        if (!loadSuccess)
        {
            SNOW_LEOPARD_CORE_ERROR("[AudioSystem] AudioClip {0} unable to load", rawAudioFilePath);
        }

        m_Clips.emplace_back(clip);

        return clip;
    }

    void AudioSystem::SetListenerPosition(const glm::vec3& pos)
    {
        ma_engine_listener_set_position(m_AudioEngine, 0, pos.x, pos.y, pos.z);
    }

    void AudioSystem::SetListenerDirection(const glm::vec3& direction)
    {
        ma_engine_listener_set_direction(m_AudioEngine, 0, direction.x, direction.y, direction.z);
    }

    void AudioSystem::SetListenerCone(float innerAngleRadians, float outerAngleRadians, float outerGain)
    {
        ma_engine_listener_set_cone(m_AudioEngine, 0, innerAngleRadians, outerAngleRadians, outerGain);
    }
} // namespace SnowLeopardEngine
