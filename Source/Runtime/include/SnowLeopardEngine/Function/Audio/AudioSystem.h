#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Audio/AudioClip.h"

#include <miniaudio.h>

namespace SnowLeopardEngine
{
    class AudioSystem final : public EngineSubSystem
    {
        friend class AudioClip;

    public:
        DECLARE_SUBSYSTEM(AudioSystem)

        // Simple API. Just Play Once or Stop current

        void Play(const std::string& rawAudioFilePath);
        void Stop();

        Ref<AudioClip> CreateAudioClip(const std::string& rawAudioFilePath);

        void SetListenerPosition(const glm::vec3& pos);
        void SetListenerDirection(const glm::vec3& direction);
        void SetListenerCone(float innerAngleRadians, float outerAngleRadians, float outerGain);

    private:
        ma_engine* m_AudioEngine;

        std::vector<Ref<AudioClip>> m_Clips;
    };
} // namespace SnowLeopardEngine
