#include "SnowLeopardEngine/Function/Audio/AudioClip.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    AudioClip::AudioClip(const std::string& path) : m_Path(path) { m_InternalSound = new ma_sound(); }

    bool AudioClip::Init()
    {
        ma_result result = ma_sound_init_from_file(
            g_EngineContext->AudioSys->m_AudioEngine, m_Path.c_str(), 0, nullptr, nullptr, m_InternalSound);

        return result == MA_SUCCESS;
    }

    void AudioClip::Release()
    {
        ma_sound_uninit(m_InternalSound);
        delete m_InternalSound;
    }

    bool AudioClip::Play()
    {
        ma_result result = ma_sound_start(m_InternalSound);
        return result == MA_SUCCESS;
    }

    bool AudioClip::Stop()
    {
        ma_result result = ma_sound_stop(m_InternalSound);
        ma_sound_seek_to_pcm_frame(m_InternalSound, 0);
        return result == MA_SUCCESS;
    }

    bool AudioClip::IsPlaying() { return ma_sound_is_playing(m_InternalSound); }

    void AudioClip::SetVolume(float volume) { ma_sound_set_volume(m_InternalSound, volume); }

    void AudioClip::SetIsLoop(bool isLoop) { ma_sound_set_looping(m_InternalSound, isLoop); }

    void AudioClip::SetIsSpatial(bool isSpatial) { ma_sound_set_spatialization_enabled(m_InternalSound, isSpatial); }

    void AudioClip::SetPosition(const glm::vec3& position)
    {
        ma_sound_set_position(m_InternalSound, position.x, position.y, position.z);
    }

    void AudioClip::SetDirection(const glm::vec3& direction)
    {
        ma_sound_set_direction(m_InternalSound, direction.x, direction.y, direction.z);
    }

    void AudioClip::SetDistanceMinMax(const glm::vec2& minMaxDistance)
    {
        ma_sound_set_min_distance(m_InternalSound, minMaxDistance.x);
        ma_sound_set_max_distance(m_InternalSound, minMaxDistance.y);
    }

    void AudioClip::SetDistanceModel(AttenuationDistanceModel model)
    {
        ma_sound_set_attenuation_model(m_InternalSound, static_cast<ma_attenuation_model>(model));
    }

    void AudioClip::SetCone(float innerAngleRadians, float outerAngleRadians, float outerGain)
    {
        ma_sound_set_cone(m_InternalSound, innerAngleRadians, outerAngleRadians, outerGain);
    }

    void AudioClip::SetRollOff(float rollOff) { ma_sound_set_rolloff(m_InternalSound, rollOff); }

    void AudioClip::SetGainMinMax(const glm::vec2& minMaxGain)
    {
        ma_sound_set_min_gain(m_InternalSound, minMaxGain.x);
        ma_sound_set_max_gain(m_InternalSound, minMaxGain.y);
    }
} // namespace SnowLeopardEngine