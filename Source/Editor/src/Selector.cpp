#include "SnowLeopardEditor/Selector.h"

namespace SnowLeopardEngine::Editor
{
    std::unordered_map<SelectionCategory, std::vector<CoreUUID>> Selector::s_SelectionMap;

    void Selector::Select(SelectionCategory category, CoreUUID selectionId)
    {
        auto& selections = s_SelectionMap[category];
        if (std::find(selections.begin(), selections.end(), selectionId) != selections.end())
        {
            return;
        }

        selections.emplace_back(selectionId);
    }

    void Selector::Unselect(SelectionCategory category, CoreUUID selectionId)
    {
        auto& selections = s_SelectionMap[category];
        auto  it         = std::find(selections.begin(), selections.end(), selectionId);
        if (it == selections.end())
        {
            return;
        }

        selections.erase(it);
    }

    void Selector::Unselect(CoreUUID selectionId)
    {
        for (auto& [category, selections] : s_SelectionMap)
        {
            auto it = std::find(selections.begin(), selections.end(), selectionId);
            if (it == selections.end())
            {
                continue;
            }

            selections.erase(it);
            break;
        }
    }

    void Selector::UnselectAll()
    {
        for (auto& [category, selections] : s_SelectionMap)
        {
            selections.clear();
        }
    }

    void Selector::UnselectAll(SelectionCategory category) { s_SelectionMap[category].clear(); }

    CoreUUID Selector::GetSelection(SelectionCategory category, size_t index)
    {
        return s_SelectionMap[category][index];
    }

    size_t Selector::GetSelectionCount(SelectionCategory category) { return s_SelectionMap[category].size(); }

    std::optional<CoreUUID> Selector::GetLastSelection(SelectionCategory category)
    {
        std::optional<CoreUUID> result;

        auto selections = GetSelections(category);
        if (!selections.empty())
        {
            result = selections[selections.size() - 1];
        }

        return result;
    }

    bool Selector::IsSelected(SelectionCategory category, CoreUUID selectionId)
    {
        const auto& selections = s_SelectionMap[category];
        return std::find(selections.begin(), selections.end(), selectionId) != selections.end();
    }
} // namespace SnowLeopardEngine::Editor