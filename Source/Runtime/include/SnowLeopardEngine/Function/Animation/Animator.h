#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"
#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "ozz/animation/runtime/blending_job.h"

namespace SnowLeopardEngine
{
    class Animator
    {
    public:
        explicit Animator(const Ref<AnimationClip>& clip);
        Animator();
        void                    Update(float dt);
        void                    SetTrigger(const std::string& triggerName);
        void                    SetFloat(const std::string& floatName, float value);
        void                    SetBoolean(const std::string& booleanName, bool value);
        void                    SetController(const Ref<AnimatorController>& controller);
        Ref<AnimatorController> GetController() const { return m_Controller; }

    private:
        void Reset() { m_CurrentTime = 0; }
        void Play(const Ref<AnimationClip>& clip);

        void Blending(const Ref<AnimationClip>& sourceAnimationClip,
                      const Ref<AnimationClip>& targetAnimationClip,
                      int                       duration,
                      float                     dt);

        friend class AnimatorController;

    private:
        void CheckParameters();

        Ref<AnimatorController> m_Controller;
        Ref<AnimationClip>      m_CurrentClip;
        float                   m_CurrentTime;
        float                   m_DeltaTime;

        bool               m_NeedBlending;
        Ref<AnimationClip> m_SourceAnimationClip;
        Ref<AnimationClip> m_TargetAnimationClip;
        int                m_Duration;
    };
} // namespace SnowLeopardEngine