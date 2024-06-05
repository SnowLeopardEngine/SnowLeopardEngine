#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"
#include "SnowLeopardEngine/Function/Animation/AnimatorController.h"
#include "ozz/animation/runtime/blending_job.h"
#include "ozz/base/maths/math_ex.h"
#include "ozz/base/maths/soa_float4x4.h"

namespace SnowLeopardEngine
{
    class Animator
    {
    public:
        explicit Animator(const Ref<AnimationClip>& clip);
        explicit Animator();
        void InitAnimators();
        void UpdateAnimator(float deltaTime);

        void                    SetTrigger(const std::string& triggerName);
        void                    SetFloat(const std::string& floatName, float value);
        void                    SetBoolean(const std::string& booleanName, bool value);
        void                    SetController(const Ref<AnimatorController>& controller);
        Ref<AnimatorController> GetController() const { return m_Controller; }

    private:
        void Update(float dt);
        void Reset() { m_CurrentTime = 0; }
        void Play(const Ref<AnimationClip>& clip);
        void CheckParameters();
        void Blending(const Ref<AnimationClip>& sourceAnimationClip,
                      const Ref<AnimationClip>& targetAnimationClip,
                      float                     duration,
                      float                     dt);

        friend class AnimatorManager;

    private:
        Ref<AnimationClip>      m_CurrentClip = nullptr;
        Ref<AnimatorController> m_Controller  = nullptr;

        float m_CurrentTime = 0;
        float m_DeltaTime = 0;

        bool m_NeedBlending = false;
        bool m_IsBlending = false;

        Ref<AnimationClip> m_SourceAnimationClip = nullptr;
        Ref<AnimationClip> m_TargetAnimationClip = nullptr;
        int                m_Duration;
    };
} // namespace SnowLeopardEngine