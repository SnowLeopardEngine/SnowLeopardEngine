#include "SnowLeopardEngine/Function/Rendering/FrameGraph/TransientResources.h"
#include "SnowLeopardEngine/Core/Base/Hash.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include <fmt/format.h>

namespace std
{
    template<>
    struct hash<SnowLeopardEngine::FrameGraphTexture::Desc>
    {
        std::size_t operator()(const SnowLeopardEngine::FrameGraphTexture::Desc& desc) const noexcept
        {
            std::size_t h {0};
            SnowLeopardEngine::hashCombine(h,
                                           desc.Extent.Width,
                                           desc.Extent.Height,
                                           desc.Depth,
                                           desc.NumMipLevels,
                                           desc.Layers,
                                           desc.Format,
                                           desc.ShadowSampler,
                                           desc.Wrap,
                                           desc.Filter);
            return h;
        }
    };
    template<>
    struct hash<SnowLeopardEngine::FrameGraphBuffer::Desc>
    {
        std::size_t operator()(const SnowLeopardEngine::FrameGraphBuffer::Desc& desc) const noexcept
        {
            std::size_t h {0};
            SnowLeopardEngine::hashCombine(h, desc.Size);
            return h;
        }
    };

} // namespace std

namespace SnowLeopardEngine
{
    void heartbeat(auto& objects, auto& pools, float dt, auto&& deleter)
    {
        constexpr auto kMaxIdleTime = 1.0f; // in seconds

        auto poolIt = pools.begin();
        while (poolIt != pools.end())
        {
            auto& [_, pool] = *poolIt;
            if (pool.empty())
            {
                poolIt = pools.erase(poolIt);
            }
            else
            {
                auto objectIt = pool.begin();
                while (objectIt != pool.cend())
                {
                    auto& [object, idleTime] = *objectIt;
                    idleTime += dt;
                    if (idleTime >= kMaxIdleTime)
                    {
                        deleter(*object);
                        SNOW_LEOPARD_CORE_INFO("[TransientResources] Released resource: {0}", fmt::ptr(object));
                        objectIt = pool.erase(objectIt);
                    }
                    else
                    {
                        ++objectIt;
                    }
                }
                ++poolIt;
            }
        }
        objects.erase(std::remove_if(objects.begin(), objects.end(), [](auto& object) { return !(*object); }),
                      objects.end());
    }

    TransientResources::TransientResources(RenderContext& rc) : m_RenderContext {rc} {}
    TransientResources::~TransientResources()
    {
        for (auto& texture : m_Textures)
            m_RenderContext.Destroy(*texture);
        for (auto& buffer : m_Buffers)
            m_RenderContext.Destroy(*buffer);
    }

    void TransientResources::Update(float dt)
    {
        const auto deleter = [&](auto& object) { m_RenderContext.Destroy(object); };
        heartbeat(m_Textures, m_TexturePools, dt, deleter);
        heartbeat(m_Buffers, m_BufferPools, dt, deleter);
    }

    Texture* TransientResources::AcquireTexture(const FrameGraphTexture::Desc& desc)
    {
        const auto h    = std::hash<FrameGraphTexture::Desc> {}(desc);
        auto&      pool = m_TexturePools[h];
        if (pool.empty())
        {
            Texture texture;
            if (desc.Depth > 0)
            {
                texture = m_RenderContext.CreateTexture3D(desc.Extent, desc.Depth, desc.Format);
            }
            else
            {
                texture = m_RenderContext.CreateTexture2D(desc.Extent, desc.Format, desc.NumMipLevels, desc.Layers);
            }

            glm::vec4 borderColor {0.0f};
            auto      addressMode = SamplerAddressMode::ClampToEdge;
            switch (desc.Wrap)
            {
                case WrapMode::ClampToEdge:
                    addressMode = SamplerAddressMode::ClampToEdge;
                    break;
                case WrapMode::ClampToOpaqueBlack:
                    addressMode = SamplerAddressMode::ClampToBorder;
                    borderColor = glm::vec4 {0.0f, 0.0f, 0.0f, 1.0f};
                    break;
                case WrapMode::ClampToOpaqueWhite:
                    addressMode = SamplerAddressMode::ClampToBorder;
                    borderColor = glm::vec4 {1.0f};
                    break;
            }
            SamplerInfo samplerInfo {
                .MinFilter    = desc.Filter,
                .MipmapMode   = desc.NumMipLevels > 1 ? MipmapMode::Nearest : MipmapMode::None,
                .MagFilter    = desc.Filter,
                .AddressModeS = addressMode,
                .AddressModeT = addressMode,
                .AddressModeR = addressMode,
                .BorderColor  = borderColor,
            };
            if (desc.ShadowSampler)
                samplerInfo.CompareOperator = CompareOp::LessOrEqual;
            m_RenderContext.SetupSampler(texture, samplerInfo);

            m_Textures.push_back(std::make_unique<Texture>(std::move(texture)));
            auto* ptr = m_Textures.back().get();
            SNOW_LEOPARD_CORE_INFO("[TransientResources] Created texture: {0}", fmt::ptr(ptr));
            return ptr;
        }
        else
        {
            auto* texture = pool.back().Resource;
            pool.pop_back();
            return texture;
        }
    }
    void TransientResources::ReleaseTexture(const FrameGraphTexture::Desc& desc, Texture* texture)
    {
        const auto h = std::hash<FrameGraphTexture::Desc> {}(desc);
        m_TexturePools[h].push_back({texture, 0.0f});
    }

    Buffer* TransientResources::AcquireBuffer(const FrameGraphBuffer::Desc& desc)
    {
        const auto h    = std::hash<FrameGraphBuffer::Desc> {}(desc);
        auto&      pool = m_BufferPools[h];
        if (pool.empty())
        {
            auto buffer = m_RenderContext.CreateBuffer(desc.Size);
            m_Buffers.push_back(std::make_unique<Buffer>(std::move(buffer)));
            auto* ptr = m_Buffers.back().get();
            SNOW_LEOPARD_CORE_INFO("[TransientResources] Created buffer: {0}", fmt::ptr(ptr));
            return ptr;
        }
        else
        {
            auto* buffer = pool.back().Resource;
            pool.pop_back();
            return buffer;
        }
    }
    void TransientResources::ReleaseBuffer(const FrameGraphBuffer::Desc& desc, Buffer* buffer)
    {
        const auto h = std::hash<FrameGraphBuffer::Desc> {}(desc);
        m_BufferPools[h].push_back({std::move(buffer), 0.0f});
    }
} // namespace SnowLeopardEngine