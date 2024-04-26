#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"

namespace SnowLeopardEngine
{
    class GLTexture2D : public Texture2D
    {
    public:
        // FrameGraph API
        explicit GLTexture2D(const TextureDesc& desc);

        // Old API
        GLTexture2D(const TextureDesc& desc, Buffer* data);

        virtual ~GLTexture2D();

        virtual void Bind(uint32_t slot) const override;
        virtual void Unbind() const override;

        virtual void UploadBuffer(Buffer* data) const override;

        virtual const TextureDesc& GetDesc() const override { return m_Desc; }

        virtual void* GetName() const override { return reinterpret_cast<void*>(static_cast<intptr_t>(m_Name)); }

        virtual bool operator==(const Texture& other) const override
        {
            return m_Name == static_cast<uint32_t>(reinterpret_cast<intptr_t>(other.GetName()));
        }

    private:
        TextureDesc m_Desc;
        uint32_t    m_Name;
        uint32_t    m_InternalFormat, m_DataFormat;
    };

    class GLCubemap : public Cubemap
    {
    public:
        // FrameGraph API
        explicit GLCubemap(const TextureDesc& desc);

        // Old API
        GLCubemap(const TextureDesc& desc, const std::vector<Buffer*>& dataList);

        virtual ~GLCubemap();

        virtual void Bind(uint32_t slot) const override;
        virtual void Unbind() const override;

        virtual void UploadBufferList(std::vector<Buffer*> dataList) const override;

        virtual const TextureDesc& GetDesc() const override { return m_Desc; }

        virtual void* GetName() const override { return reinterpret_cast<void*>(static_cast<intptr_t>(m_Name)); }

        virtual bool operator==(const Texture& other) const override
        {
            return m_Name == static_cast<uint32_t>(reinterpret_cast<intptr_t>(other.GetName()));
        }

    private:
        TextureDesc m_Desc;
        uint32_t    m_Name;
        uint32_t    m_InternalFormat, m_DataFormat;
    };
} // namespace SnowLeopardEngine