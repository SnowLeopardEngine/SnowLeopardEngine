#pragma once

#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"

namespace SnowLeopardEngine::Editor
{
    enum class SelectionCategory
    {
        None = 0,
        Viewport,
        // TODO: More categories
    };

    class Selector
    {
    public:
        static void                                Select(SelectionCategory category, CoreUUID selectionId);
        static void                                Unselect(SelectionCategory category, CoreUUID selectionId);
        static void                                Unselect(CoreUUID selectionId);
        static void                                UnselectAll();
        static void                                UnselectAll(SelectionCategory category);
        static CoreUUID                            GetSelection(SelectionCategory category, size_t index);
        static size_t                              GetSelectionCount(SelectionCategory category);
        static std::optional<CoreUUID>             GetLastSelection(SelectionCategory category);
        inline static const std::vector<CoreUUID>& GetSelections(SelectionCategory category)
        {
            return s_SelectionMap[category];
        }

        static bool IsSelected(SelectionCategory category, CoreUUID selectionId);

    private:
        static std::unordered_map<SelectionCategory, std::vector<CoreUUID>> s_SelectionMap;
    };
} // namespace SnowLeopardEngine::Editor