#pragma once

#include "SnowLeopardEngine/Function/Rendering/VertexAttributes.h"

namespace SnowLeopardEngine
{
    // NOTE: Make sure that the following locations match with the
    // Assets/Shaders/Geometry.vert
    enum class AttributeLocation : int32_t
    {
        Position = 0,
        Normal,
        TexCoords,
        EntityID
    };

    class VertexFormat
    {
    public:
        VertexFormat()                        = delete;
        VertexFormat(const VertexFormat&)     = delete;
        VertexFormat(VertexFormat&&) noexcept = default;
        ~VertexFormat()                       = default;

        VertexFormat& operator=(const VertexFormat&)     = delete;
        VertexFormat& operator=(VertexFormat&&) noexcept = delete;

        std::size_t GetHash() const;

        const VertexAttributes& GetAttributes() const;
        bool                    Contains(AttributeLocation) const;
        bool                    Contains(std::initializer_list<AttributeLocation>) const;

        uint32_t GetStride() const;

        class Builder final
        {
        public:
            Builder()                   = default;
            Builder(const Builder&)     = delete;
            Builder(Builder&&) noexcept = delete;
            ~Builder()                  = default;

            Builder& operator=(const Builder&)     = delete;
            Builder& operator=(Builder&&) noexcept = delete;

            Builder& SetAttribute(AttributeLocation, const VertexAttribute&);

            Ref<VertexFormat> Build();
            Ref<VertexFormat> BuildDefault();

        private:
            VertexAttributes m_Attributes;

            using Cache = std::unordered_map<std::size_t, std::weak_ptr<VertexFormat>>;
            inline static Cache s_Cache;
        };

    private:
        VertexFormat(std::size_t hash, VertexAttributes&&, uint32_t stride);

    private:
        const std::size_t      m_Hash {0u};
        const VertexAttributes m_Attributes;
        const uint32_t         m_Stride {0};
    };

    int32_t getSize(VertexAttribute::Type);
} // namespace SnowLeopardEngine