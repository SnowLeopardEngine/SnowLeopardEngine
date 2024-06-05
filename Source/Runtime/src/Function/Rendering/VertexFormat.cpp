#include "SnowLeopardEngine/Function/Rendering/VertexFormat.h"
#include "SnowLeopardEngine/Core/Base/Hash.h"
#include "SnowLeopardEngine/Function/Rendering/VertexAttributes.h"

namespace SnowLeopardEngine
{
    int32_t getSize(VertexAttribute::Type type)
    {
        switch (type)
        {
            using enum VertexAttribute::Type;
            case Float:
                return sizeof(float);
            case Float2:
                return sizeof(float) * 2;
            case Float3:
                return sizeof(float) * 3;
            case Float4:
                return sizeof(float) * 4;

            case VertexAttribute::Type::Int:
                return sizeof(int32_t);
            case Int4:
                return sizeof(int32_t) * 4;

            case UByte4_Norm:
                return sizeof(uint8_t) * 4;
        }
        return 0;
    }

    std::size_t VertexFormat::GetHash() const { return m_Hash; }

    const VertexAttributes& VertexFormat::GetAttributes() const { return m_Attributes; }
    bool                    VertexFormat::Contains(AttributeLocation location) const
    {
        return m_Attributes.contains(static_cast<int32_t>(location));
    }
    bool VertexFormat::Contains(std::initializer_list<AttributeLocation> locations) const
    {
        return std::all_of(std::cbegin(locations), std::cend(locations), [&](auto location) {
            return std::any_of(m_Attributes.cbegin(),
                               m_Attributes.cend(),
                               [v = static_cast<int32_t>(location)](const auto& p) { return p.first == v; });
        });
    }

    uint32_t VertexFormat::GetStride() const { return m_Stride; }

    VertexFormat::VertexFormat(std::size_t hash, VertexAttributes&& attributes, uint32_t stride) :
        m_Hash {hash}, m_Attributes {attributes}, m_Stride {stride}
    {}

    using Builder = VertexFormat::Builder;

    Builder& Builder::SetAttribute(AttributeLocation location, const VertexAttribute& attribute)
    {
        m_Attributes.insert_or_assign(static_cast<int32_t>(location), attribute);
        return *this;
    }

    Ref<VertexFormat> Builder::Build()
    {
        uint32_t    stride {0};
        std::size_t hash {0};
        for (const auto& [location, attribute] : m_Attributes)
        {
            stride += getSize(attribute.VertType);
            hashCombine(hash, location, attribute);
        }

        if (const auto it = s_Cache.find(hash); it != s_Cache.cend())
            if (auto vertexFormat = it->second.lock(); vertexFormat)
                return vertexFormat;

        auto vertexFormat = CreateRef<VertexFormat>(VertexFormat {hash, std::move(m_Attributes), stride});
        s_Cache.insert_or_assign(hash, vertexFormat);
        return vertexFormat;
    }

    Ref<VertexFormat> Builder::BuildDefault()
    {
        m_Attributes.clear();

        SetAttribute(AttributeLocation::Position, {.VertType = VertexAttribute::Type::Float3, .Offset = 0});
        SetAttribute(AttributeLocation::Normal, {.VertType = VertexAttribute::Type::Float3, .Offset = 12});
        SetAttribute(AttributeLocation::TexCoords, {.VertType = VertexAttribute::Type::Float2, .Offset = 24});
        SetAttribute(AttributeLocation::EntityID, {.VertType = VertexAttribute::Type::Int, .Offset = 32});

        return Build();
    }
} // namespace SnowLeopardEngine