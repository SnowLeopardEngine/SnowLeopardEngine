#include "SnowLeopardEngine/Function/Project/Project.h"

namespace SnowLeopardEngine
{
    void Project::LoadAssets()
    {
        Resources::OnProjectLoad(this);
        Resources::ImportAssets(m_Path.parent_path());
        m_Info.AssetMetaDatas = Resources::g_AssetMetaDatas;
    }
} // namespace SnowLeopardEngine