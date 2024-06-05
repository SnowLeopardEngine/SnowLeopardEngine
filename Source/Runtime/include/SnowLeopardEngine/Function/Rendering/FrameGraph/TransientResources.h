#pragma once

#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"

namespace SnowLeopardEngine
{
    class RenderContext;

    class TransientResources
    {
    public:
        TransientResources() = delete;
        explicit TransientResources(RenderContext&);
        TransientResources(const TransientResources&)     = delete;
        TransientResources(TransientResources&&) noexcept = delete;
        ~TransientResources();

        TransientResources& operator=(const TransientResources&)     = delete;
        TransientResources& operator=(TransientResources&&) noexcept = delete;

        void Update(float dt);

        Texture* AcquireTexture(const FrameGraphTexture::Desc&);
        void     ReleaseTexture(const FrameGraphTexture::Desc&, Texture*);

        Buffer* AcquireBuffer(const FrameGraphBuffer::Desc&);
        void    ReleaseBuffer(const FrameGraphBuffer::Desc&, Buffer*);

    private:
        RenderContext& m_RenderContext;

        std::vector<Scope<Texture>> m_Textures;
        std::vector<Scope<Buffer>>  m_Buffers;

        template<typename T>
        struct ResourceEntry
        {
            T     Resource;
            float Life;
        };
        template<typename T>
        using ResourcePool = std::vector<ResourceEntry<T>>;

        std::unordered_map<std::size_t, ResourcePool<Texture*>> m_TexturePools;
        std::unordered_map<std::size_t, ResourcePool<Buffer*>>  m_BufferPools;
    };
} // namespace SnowLeopardEngine