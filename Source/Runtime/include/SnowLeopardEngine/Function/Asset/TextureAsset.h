#pragma once

#include "SnowLeopardEngine/Function/Asset/Asset.h"
#include "SnowLeopardEngine/Function/IO/TextureLoader.h"

namespace SnowLeopardEngine
{
    class Texture2DAsset : public Asset
    {
    public:
        Texture2DAsset(const std::filesystem::path& path,
                       const CoreUUID&              uuid,
                       const TextureLoadingOutput&  loadOutput) :
            Asset(path, uuid),
            m_LoadOutput(loadOutput)
        {}

        Ref<Texture2D> GetHandle()
        {
            if (m_Handle == nullptr)
            {
                m_Handle = Texture2D::Create(m_LoadOutput.GetDesc(), &m_LoadOutput.OutBuffer);
            }

            return m_Handle;
        }

        Buffer      GetBuffer() const { return m_LoadOutput.OutBuffer; }
        TextureDesc GetDesc() const { return m_LoadOutput.GetDesc(); }

        static bool Load(const std::filesystem::path& path, Ref<Texture2DAsset>& outAsset, bool flip);

    private:
        TextureLoadingOutput m_LoadOutput;
        Ref<Texture2D>       m_Handle = nullptr;
    };
} // namespace SnowLeopardEngine