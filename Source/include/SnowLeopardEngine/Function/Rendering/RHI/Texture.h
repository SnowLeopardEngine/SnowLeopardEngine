#pragma once

#include "SnowLeopardEngine/Core/Containers/Buffer.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    class Texture
    {
    public:
        virtual ~Texture() = default;

        virtual void Bind(uint32_t slot) const = 0;

        virtual const TextureDesc& GetDesc() const = 0;

        virtual void* GetName() const = 0;

        virtual bool operator==(const Texture& other) const = 0;
    };

    class Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(const TextureDesc& desc, Buffer* data);
    };
} // namespace SnowLeopardEngine