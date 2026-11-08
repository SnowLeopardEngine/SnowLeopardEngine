#pragma once

#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

#include <string>

namespace SnowLeopardEngine::Editor
{
    // The entities of the active scene as a tree (TreeNodeComponent): click to select (Ctrl / Shift: add or remove),
    // drag onto an entity to parent it (onto the empty area: to the root, the world transform is kept), double-click
    // or F2 to rename, right-click for create / duplicate / delete, a filter by name.
    class HierarchyPanel final : public PanelBase
    {
    public:
        virtual void OnTick(float deltaTime) override final;

        // The "create" items shared with the main menu bar's Entity menu.
        static void DrawCreateMenu(Entity parent);

    private:
        void DrawEntity(Entity entity);
        bool MatchesFilter(Entity entity);

        char     m_Filter[128]     = {};
        CoreUUID m_Renaming        = {};
        char     m_RenameBuffer[128] = {};
        bool     m_FocusRename     = false;
        Entity   m_ContextEntity   = {};
    };
} // namespace SnowLeopardEngine::Editor
