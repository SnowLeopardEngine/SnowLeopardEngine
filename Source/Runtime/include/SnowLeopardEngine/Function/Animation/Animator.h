#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"

namespace SnowLeopardEngine
{
    class Animator
    {
    public:
        explicit Animator(const Ref<AnimationClip>& clip);

        void SetLoop(bool loop) { m_Loop = loop; }

    private:
        void Update(float dt);
        void Reset() { m_CurrentTime = 0; }
        void Play(const Ref<AnimationClip>& clip);

        friend class AnimatorController;

    private:
        Ref<AnimationClip> m_CurrentClip;
        float              m_CurrentTime;
        float              m_DeltaTime;
        bool               m_Loop = true;
    };
} // namespace SnowLeopardEngine