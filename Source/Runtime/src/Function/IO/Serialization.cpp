#include "SnowLeopardEngine/Function/IO/Serialization.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Project/Project.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

namespace SnowLeopardEngine
{
    namespace IO
    {
        bool Serialize(LogicScene* scene, const std::filesystem::path& dstPath)
        {
            std::ofstream os(dstPath.generic_string());
            if (!os.is_open())
            {
                SNOW_LEOPARD_CORE_ERROR("[LogicSceneSerializer] Failed to open output stream at {0}!",
                                        dstPath.generic_string());
                return false;
            }

            cereal::JSONOutputArchive output(os);
            entt::snapshot {scene->GetRegistry()}.entities(output).component<ALL_SERIALIZABLE_COMPONENT_TYPES>(output);

            return true;
        }

        bool Deserialize(LogicScene* scene, const std::filesystem::path& srcPath)
        {
            std::ifstream is(srcPath.generic_string());

            if (!is.is_open())
            {
                SNOW_LEOPARD_CORE_ERROR("[LogicSceneSerializer] Failed to open input stream at {0}!",
                                        srcPath.generic_string());
                return false;
            }

            cereal::JSONInputArchive input(is);
            entt::snapshot_loader {scene->GetRegistry()}
                .entities(input)
                .component<ALL_SERIALIZABLE_COMPONENT_TYPES>(input)
                .orphans();

            return true;
        }

        bool Serialize(Project* project, const std::filesystem::path& dstPath)
        {
            std::ofstream os(dstPath.generic_string());
            if (!os.is_open())
            {
                SNOW_LEOPARD_CORE_ERROR("[ProjectSerializer] Failed to open output stream at {0}!",
                                        dstPath.generic_string());
                return false;
            }

            cereal::JSONOutputArchive output(os);

            output(project->GetInfo());

            return true;
        }

        bool Deserialize(Project* project, const std::filesystem::path& srcPath)
        {
            std::ifstream is(srcPath.generic_string());

            if (!is.is_open())
            {
                SNOW_LEOPARD_CORE_ERROR("[ProjectSerializer] Failed to open input stream at {0}!",
                                        srcPath.generic_string());
                return false;
            }

            cereal::JSONInputArchive input(is);

            DzProject dzProject;
            input(dzProject);

            project->SetInfo(dzProject);

            return true;
        }
    } // namespace IO
} // namespace SnowLeopardEngine