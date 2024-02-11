#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"

namespace SnowLeopardEngine
{
    template<class T>
    class SingletonWrapper
    {
    public:
        SNOW_LEOPARD_NO_COPY_MOVE_CONSTRUCTOR(SingletonWrapper)
        SNOW_LEOPARD_DEFAULT_CONSTRUCTOR(SingletonWrapper)

        ~SingletonWrapper() { Shutdown(); }

        template<typename... Args>
        void Init(Args&&... args)
        {
            if (m_Instance == nullptr)
            {
                m_Instance = new T(std::forward<Args>(args)...);
            }
            else
            {
                throw std::runtime_error("Init function is called twice.");
            }
        }

        void Shutdown()
        {
            if (m_Instance != nullptr)
            {
                delete m_Instance;
                m_Instance = nullptr;
            }
        }

        T* GetInstance() noexcept { return m_Instance; }

        const T* GetInstance() const noexcept { return m_Instance; }

        T* operator->() noexcept { return m_Instance; }

        const T* operator->() const noexcept { return m_Instance; }

        T& operator()() noexcept { return *m_Instance; }

        const T& operator()() const noexcept { return *m_Instance; }

    private:
        T* m_Instance = nullptr;
    };
} // namespace SnowLeopardEngine
