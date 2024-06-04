#include "SnowLeopardEngine/Function/Animation/AnimatorManager.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Animation/Transition.h"

namespace SnowLeopardEngine
{
    void AnimatorManager::Init() { InitAnimators(); }

    void AnimatorManager::Update(float deltaTime)
    {
        UpdateAnimator(deltaTime);
    }

    void AnimatorManager::RegisterAnimator(const Ref<Animator>& animator) { m_Animators.push_back(animator); }

    void AnimatorManager::DeleteAnimator(const Ref<Animator>& animator)
    {
        auto it = std::find(m_Animators.begin(), m_Animators.end(), animator);
        if (it != m_Animators.end())
        {
            m_Animators.erase(it);
        }
    }

    void AnimatorManager::InitAnimators()
    {
        for (const auto& animator : m_Animators)
        {
            animator->Reset();
            animator->Update(0);
        }
    }

    void AnimatorManager::UpdateAnimator(float deltaTime) 
    {
        for(const auto& animator : m_Animators)
        {
            animator->Update(deltaTime); 
        }
    }

} // namespace SnowLeopardEngine