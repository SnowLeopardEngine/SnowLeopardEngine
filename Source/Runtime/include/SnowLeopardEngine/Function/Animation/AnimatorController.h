#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"
#include "SnowLeopardEngine/Function/Animation/Transition.h"
#include <vector>


namespace SnowLeopardEngine
{
    class AnimatorController
    {
    public:
        explicit AnimatorController();
        void RegisterAnimationClip(const Ref<AnimationClip>& animationClip);
        void DeleteAnimationClip(const Ref<AnimationClip>& animationClip);
        void SetEntryAnimationClip(const Ref<AnimationClip>& animationClip);

        Ref<Transition> RegisterTransition(const Ref<AnimationClip>& sourceAnimationClip, const Ref<AnimationClip>& targetAnimationClip, float duration);
        void DeleteTransition(const Ref<AnimationClip>& sourceAnimationClip, const Ref<AnimationClip>& targetAnimationClip);

        void RegisterParameters(const std::string& parameterName, const std::variant<float, bool>& defaultValue);
        void RegisterParameters(const std::string& parameterName);
        void DeleteParameters(const std::string& parameterName);

        inline Ref<std::map<std::string, std::variant<float, bool, std::monostate>>> GetParameters() { return m_Parameters;}
        inline std::map<Ref<AnimationClip>, std::vector<Ref<Transition>>> GetTransitions() { return m_TransitionsInfoMap;}
        inline Ref<AnimationClip> GetAnimationClip() { return m_CurrentAnimationClip;}
        inline void SetAnimationClip(const Ref<AnimationClip>& animatioClip) { m_CurrentAnimationClip = animatioClip;}

    private:

    private: 
        Ref<AnimationClip>                                                         m_CurrentAnimationClip;
        std::vector<Ref<AnimationClip>>                                            m_AnimationClips;
        std::map<Ref<AnimationClip>, std::vector<Ref<Transition>>>                 m_TransitionsInfoMap;
        Ref<std::map<std::string, std::variant<float, bool, std::monostate>>>      m_Parameters;
    };
} // namespace SnowLeopardEngine