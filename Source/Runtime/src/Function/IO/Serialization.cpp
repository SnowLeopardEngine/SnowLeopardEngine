#include "SnowLeopardEngine/Function/IO/Serialization.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Project/Project.h"
#include "SnowLeopardEngine/Function/Rendering/Material.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

namespace SnowLeopardEngine
{
    namespace IO
    {
        namespace
        {
            // entt >= 3.13: one get<T>() per storage, entities first.
            template<typename... Component>
            void SaveSnapshot(const entt::registry& registry, cereal::JSONOutputArchive& output)
            {
                entt::snapshot snapshot {registry};
                snapshot.get<entt::entity>(output);
                (snapshot.get<Component>(output), ...);
            }

            template<typename... Component>
            void LoadSnapshot(entt::registry& registry, cereal::JSONInputArchive& input)
            {
                entt::snapshot_loader loader {registry};
                loader.get<entt::entity>(input);
                (loader.get<Component>(input), ...);
                loader.orphans();
            }
        } // namespace

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
            SaveSnapshot<ALL_SERIALIZABLE_COMPONENT_TYPES>(scene->GetRegistry(), output);

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
            LoadSnapshot<ALL_SERIALIZABLE_COMPONENT_TYPES>(scene->GetRegistry(), input);

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

        bool Serialize(Material* material, const std::filesystem::path& dstPath)
        {
            std::ofstream os(dstPath.generic_string());
            if (!os.is_open())
            {
                SNOW_LEOPARD_CORE_ERROR("[MaterialSerializer] Failed to open output stream at {0}!",
                                        dstPath.generic_string());
                return false;
            }

            cereal::JSONOutputArchive output(os);

            output(material->GetDefine());

            return true;
        }

        bool Deserialize(Material* project, const std::filesystem::path& srcPath)
        {
            std::ifstream is(srcPath.generic_string());

            if (!is.is_open())
            {
                SNOW_LEOPARD_CORE_ERROR("[MaterialSerializer] Failed to open input stream at {0}!",
                                        srcPath.generic_string());
                return false;
            }

            cereal::JSONInputArchive input(is);

            DzMaterial dzMaterial;
            input(dzMaterial);

            project->SetDefine(dzMaterial);

            return true;
        }
    } // namespace IO
} // namespace SnowLeopardEngine