#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Function/Project/Project.h"

namespace SnowLeopardEngine
{
    class ProjectManager final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(ProjectManager)

        void         SetActiveProject(const Ref<Project>& activeProject) { m_ActiveProject = activeProject; }
        Ref<Project> GetActiveProject() const { return m_ActiveProject; }

        static Ref<Project> CreateProject() { return CreateRef<Project>(); }

    private:
        Ref<Project> m_ActiveProject = nullptr;
    };
} // namespace SnowLeopardEngine