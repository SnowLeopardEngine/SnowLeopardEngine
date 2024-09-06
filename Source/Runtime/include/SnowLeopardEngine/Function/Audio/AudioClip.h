#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"

#include <miniaudio.h>

namespace SnowLeopardEngine
{
    enum class AttenuationDistanceModel
    {
        None = 0,
        Inverse,
        Linear,
        Exponential
    };

    class AudioClip
    {
        friend class AudioSystem;

    public:
        AudioClip() = delete;
        explicit AudioClip(const std::string& path);

        bool Play();
        bool Stop();

        bool IsPlaying();

        void SetVolume(float volume);

        void SetIsLoop(bool isLoop);
        void SetIsSpatial(bool isSpatial);

        void SetPosition(const glm::vec3& position);
        void SetDirection(const glm::vec3& direction);

        void SetDistanceMinMax(const glm::vec2& minMaxDistance);
        void SetDistanceModel(AttenuationDistanceModel model);

        void SetCone(float innerAngleRadians, float outerAngleRadians, float outerGain);

        void SetRollOff(float rollOff);
        void SetGainMinMax(const glm::vec2& minMaxGain);

    private:
        bool Init();
        void Release();

    private:
        ma_sound* m_InternalSound = nullptr;

        std::string m_Path;
    };
} // namespace SnowLeopardEngine