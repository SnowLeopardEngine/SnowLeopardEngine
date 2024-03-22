#include "SnowLeopardEditor/Panels/ResourcePanel.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include "IconsMaterialDesignIcons.h"
#include "imgui_internal.h"

namespace SnowLeopardEngine::Editor
{
    std::string GlobalizePath(std::string_view path)
    {
        std::string pathCopy = std::string(path);
        if (pathCopy.find("res://") == 0)
        {
            if (!FileSystem::GetExecutableDirectory().string().empty())
            {
                // Replace "res:/" with project_path_
                size_t pos = pathCopy.find("res:/");
                if (pos != std::string::npos)
                {
                    pathCopy.replace(pos, 5, FileSystem::GetExecutableDirectory().string());
                }
            }
            else
            {
                // Remove "res://" from path
                pathCopy.erase(0, 6);
            }
        }
        return pathCopy;
    }

    DirectoryInformation::DirectoryInformation(const std::string& thisPath, DirectoryInformation* parent) :
        Parent(parent)
    {
        ThisPath = thisPath;
        if (Parent)
        {
            LocalPath  = PathJoin(Parent->LocalPath, ThisPath);
            GlobalPath = GlobalizePath(LocalPath);
        }
        else
        {
            LocalPath  = ThisPath;
            GlobalPath = GlobalizePath(LocalPath);
        }
        IsDirectory = std::filesystem::is_directory(GlobalPath);
    }

    void ResourcePanel::Init()
    {
        m_ShowHiddenFiles      = false;
        m_IsDragging           = false;
        m_IsInListView         = true;
        m_UpdateNavigationPath = true;
        m_Filter               = new ImGuiTextFilter();
        m_ProjectPath          = FileSystem::GetExecutableDirectory().string();
        m_BaseProjectDir       = ProcessDirectory("res://", nullptr);
        ChangeDirectory(m_BaseProjectDir);
    }

    void ResourcePanel::ChangeDirectory(DirectoryInformation* directory)
    {
        if (!directory)
            return;
        m_PreviousDirectory    = m_CurrentDir;
        m_CurrentDir           = directory;
        m_UpdateNavigationPath = true;
    }

    bool ResourcePanel::MoveFFile(const std::string& filePath, const std::string& movePath)
    {
        auto movedPath = PathJoin(filePath, movePath);
        return std::filesystem::exists(movedPath);
    }

    void ResourcePanel::OnTick(float /*deltaTime*/)
    {
        if (ImGui::Begin(ICON_MDI_FOLDER_STAR "Resources"))
        {
            auto windowSize = ImGui::GetWindowSize();
            bool vertical   = windowSize.y > windowSize.x;
            {
                if (!vertical)
                {
                    ImGui::BeginColumns("ResourcePanelColumns", 2, ImGuiOldColumnFlags_NoResize);
                    ImGui::SetColumnWidth(0, ImGui::GetWindowContentRegionMax().x / 3.0f);
                    ImGui::BeginChild("##folders_common");
                }
                else
                    ImGui::BeginChild("##folders_common", ImVec2(0, ImGui::GetWindowHeight() / 3.0f));
                {
                    ImGui::BeginChild("##folders");
                    {
                        DrawFolder(m_BaseProjectDir, true);
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
            }
            if (ImGui::BeginDragDropTarget())
            {
                const auto* data =
                    ImGui::AcceptDragDropPayload("selectable", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
                if (data)
                {
                    std::string file = static_cast<char*>(data->Data);
                    if (MoveFFile(file, m_MovePath))
                    {
                        SNOW_LEOPARD_INFO("Moved File: {0} to {1}", file, m_MovePath);
                    }
                    m_IsDragging = false;
                }
                ImGui::EndDragDropTarget();
            }
            float offset = 0.0f;
            if (!vertical)
            {
                ImGui::NextColumn();
            }
            else
            {
                offset = ImGui::GetWindowHeight() / 3.0f + 6.0f;
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
            }
            ImGui::BeginChild("##directory_structure",
                              ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing() * 2.6f - offset));
            {
                ImGui::BeginChild("##directory_breadcrumbs",
                                  ImVec2(ImGui::GetColumnWidth(), ImGui::GetFrameHeightWithSpacing()));
                if (ImGui::Button(ICON_MDI_ARROW_LEFT))
                {
                    if (m_CurrentDir != m_BaseProjectDir)
                    {
                        m_PreviousDirectory    = m_CurrentDir;
                        m_CurrentDir           = m_CurrentDir->Parent;
                        m_UpdateNavigationPath = true;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button(ICON_MDI_ARROW_RIGHT))
                {
                    // ...
                }
                ImGui::SameLine();
                if (m_UpdateNavigationPath)
                {
                    m_BreadCrumbData.clear();
                    auto* current = m_CurrentDir;
                    while (current)
                    {
                        if (current->Parent != nullptr)
                        {
                            m_BreadCrumbData.push_back(current);
                            current = current->Parent;
                        }
                        else
                        {
                            m_BreadCrumbData.push_back(m_BaseProjectDir);
                            current = nullptr;
                        }
                    }
                    std::reverse(m_BreadCrumbData.begin(), m_BreadCrumbData.end());
                    m_UpdateNavigationPath = false;
                }
                if (m_IsInListView)
                {
                    if (ImGui::Button(ICON_MDI_VIEW_GRID))
                    {
                        m_IsInListView = !m_IsInListView;
                    }
                    ImGui::SameLine();
                }
                else
                {
                    if (ImGui::Button(ICON_MDI_VIEW_LIST))
                    {
                        m_IsInListView = !m_IsInListView;
                    }
                    ImGui::SameLine();
                }
                ImGui::TextUnformatted(ICON_MDI_MAGNIFY);
                ImGui::SameLine();
                m_Filter->Draw("##Filter", ImGui::GetContentRegionAvail().x - ImGui::GetStyle().IndentSpacing);
                ImGui::EndChild();
                {
                    ImGui::BeginChild("##Scrolling");
                    int   shownIndex = 0;
                    float xAvail     = ImGui::GetContentRegionAvail().x;
                    m_GridItemsPerRow =
                        static_cast<int>(floor(xAvail / (m_GridSize + ImGui::GetStyle().ItemSpacing.x)));
                    m_GridItemsPerRow = std::max(1, m_GridItemsPerRow);
                    if (m_IsInListView)
                    {
                        for (int i = 0; i < m_CurrentDir->Children.size(); i++)
                        {
                            if (!m_CurrentDir->Children.empty())
                            {
                                if (m_Filter->IsActive())
                                {
                                    if (!m_Filter->PassFilter(m_CurrentDir->Children[i]->GlobalPath.c_str()))
                                    {
                                        continue;
                                    }
                                }
                                bool doubleClicked =
                                    RenderFile(i, m_CurrentDir->Children[i]->IsDirectory, shownIndex, !m_IsInListView);
                                if (doubleClicked)
                                    break;
                                shownIndex++;
                            }
                        }
                    }
                    else
                    {
                        for (int i = 0; i < m_CurrentDir->Children.size(); i++)
                        {
                            if (m_Filter->IsActive())
                            {
                                if (!m_Filter->PassFilter(m_CurrentDir->Children[i]->GlobalPath.c_str()))
                                {
                                    continue;
                                }
                            }
                            bool doubleClicked =
                                RenderFile(i, m_CurrentDir->Children[i]->IsDirectory, shownIndex, !m_IsInListView);
                            if (doubleClicked)
                                break;
                            shownIndex++;
                        }
                    }
                    if (ImGui::BeginPopupContextWindow())
                    {
                        if (ImGui::Selectable("Import New"))
                        {
                            // ...
                        }
                        if (ImGui::Selectable("Refresh"))
                        {
                            Refresh();
                        }
                        if (ImGui::Selectable("New folder"))
                        {
                            // ...
                            Refresh();
                        }
                        ImGui::EndPopup();
                    }
                    ImGui::EndChild();
                }
                ImGui::EndChild();
                ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
                RenderBottom();
            }
            if (ImGui::BeginDragDropTarget())
            {
                const auto* data =
                    ImGui::AcceptDragDropPayload("selectable", ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
                if (data)
                {
                    std::string a = static_cast<char*>(data->Data);
                    if (MoveFFile(a, m_MovePath))
                    {
                        SNOW_LEOPARD_INFO("Moved File: {0} to {1}", a, m_MovePath);
                    }
                    m_IsDragging = false;
                }
                ImGui::EndDragDropTarget();
            }
        }
        ImGui::End();
    }

    void ResourcePanel::Shutdown() { delete m_Filter; }

    bool ResourcePanel::RenderFile(int dirIndex, bool folder, int shownIndex, bool gridView)
    {
        bool doubleClicked = false;
        if (gridView)
        {
            ImGui::BeginGroup();
            if (ImGui::Button(folder ? ICON_MDI_FOLDER : ICON_MDI_FILE))
            {
            }
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                doubleClicked = true;
            }
            auto newFname = SimplifyPath(m_CurrentDir->Children[dirIndex]->ThisPath);
            ImGui::TextUnformatted(newFname.c_str());
            ImGui::EndGroup();
            if ((shownIndex + 1) % m_GridItemsPerRow != 0)
                ImGui::SameLine();
        }
        else
        {
            ImGui::TextUnformatted(folder ? ICON_MDI_FOLDER : ICON_MDI_FILE);
            ImGui::SameLine();
            if (ImGui::Selectable(
                    m_CurrentDir->Children[dirIndex]->ThisPath.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    doubleClicked = true;
                }
            }
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(m_CurrentDir->Children[dirIndex]->GlobalPath.c_str());
            ImGui::EndTooltip();
        }
        ImGui::PopStyleVar();
        if (doubleClicked)
        {
            if (folder)
            {
                m_PreviousDirectory    = m_CurrentDir;
                m_CurrentDir           = m_CurrentDir->Children[dirIndex];
                m_UpdateNavigationPath = true;
            }
            else
            {
                //            m_Editor->FileOpenCallback(m_BasePath + "/" +
                //            m_CurrentDir->Children[dirIndex]->FilePath.string());
            }
        }
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
        {
            ImGui::TextUnformatted(ICON_MDI_FILE);
            ImGui::SameLine();
            m_MovePath = m_ProjectPath + "/" + m_CurrentDir->Children[dirIndex]->GlobalPath;
            ImGui::TextUnformatted(m_MovePath.c_str());
            size_t size = sizeof(const char*) + strlen(m_MovePath.c_str());
            ImGui::SetDragDropPayload("AssetFile", m_MovePath.c_str(), size);
            m_IsDragging = true;
            ImGui::EndDragDropSource();
        }
        return doubleClicked;
    }

    void ResourcePanel::RenderBottom()
    {
        ImGui::BeginChild("##nav",
                          ImVec2(ImGui::GetColumnWidth(), ImGui::GetFontSize() * 1.8f),
                          true,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.2f, 0.7f, 0.0f));
            for (auto& directory : m_BreadCrumbData)
            {
                const std::string& directoryName = directory->ThisPath;
                if (ImGui::SmallButton(directoryName.c_str()))
                    ChangeDirectory(directory);
                ImGui::SameLine();
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }
        if (!m_IsInListView)
        {
            ImGui::SliderFloat("##GridSize", &m_GridSize, 40.0f, 400.0f);
        }
        ImGui::EndChild();
    }

    DirectoryInformation* ResourcePanel::ProcessDirectory(const std::string&    directoryPath,
                                                          DirectoryInformation* parent)
    {
        auto it = m_Directories.find(directoryPath);
        if (it != m_Directories.end())
            return it->second.get();
        auto directoryInfo = std::make_unique<DirectoryInformation>(directoryPath, parent);
        if (directoryInfo->IsDirectory)
        {
            for (const auto& fileInfo : std::filesystem::directory_iterator {directoryInfo->GlobalPath})
            {
                auto* subdir = ProcessDirectory(FileSystem::GetFileName(fileInfo.path().string()), directoryInfo.get());
                directoryInfo->Children.push_back(subdir);
            }
        }
        auto* res                                = directoryInfo.get();
        m_Directories[directoryInfo->GlobalPath] = std::move(directoryInfo);
        return res;
    }

    void ResourcePanel::DrawFolder(DirectoryInformation* dirInfo, bool defaultOpen)
    {
        ImGuiTreeNodeFlags nodeFlags = ((dirInfo == m_CurrentDir) ? ImGuiTreeNodeFlags_Selected : 0);
        nodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
        if (dirInfo->Parent == nullptr)
            nodeFlags |= ImGuiTreeNodeFlags_Framed;
        const ImColor treeLineColor = ImColor(128, 128, 128, 128);
        const float   smallOffsetX  = 6.0f * ImGui::GetWindowDpiScale();
        ImDrawList*   drawList      = ImGui::GetWindowDrawList();
        if (dirInfo->IsDirectory)
        {
            bool containsFolder = false;
            for (auto& file : dirInfo->Children)
            {
                if (file->IsDirectory)
                {
                    containsFolder = true;
                    break;
                }
            }
            if (!containsFolder)
                nodeFlags |= ImGuiTreeNodeFlags_Leaf;
            if (defaultOpen)
                nodeFlags |= ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf;
            nodeFlags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_AllowItemOverlap |
                         ImGuiTreeNodeFlags_SpanAvailWidth;
            bool        isOpen = ImGui::TreeNodeEx(reinterpret_cast<void*>(dirInfo), nodeFlags, "");
            const char* folderIcon =
                ((isOpen && containsFolder) || m_CurrentDir == dirInfo) ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.48f, 0.48f, 0.48f, 0.93f));
            ImGui::Text("%s ", folderIcon);
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::TextUnformatted(static_cast<const char*>(dirInfo->ThisPath.c_str()));
            ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
            if (ImGui::IsItemClicked())
            {
                m_PreviousDirectory    = m_CurrentDir;
                m_CurrentDir           = dirInfo;
                m_UpdateNavigationPath = true;
            }
            if (isOpen && containsFolder)
            {
                verticalLineStart.x += smallOffsetX; // to nicely line up with the arrow symbol
                ImVec2 verticalLineEnd = verticalLineStart;
                for (auto* i : dirInfo->Children)
                {
                    if (i->IsDirectory)
                    {
                        auto currentPos = ImGui::GetCursorScreenPos();
                        ImGui::Indent(10.0f);
                        bool containsFolderTemp = false;
                        for (auto& file : i->Children)
                        {
                            if (file->IsDirectory)
                            {
                                containsFolderTemp = true;
                                break;
                            }
                        }
                        float horizontalTreeLineSize = 16.0f * ImGui::GetWindowDpiScale(); // chosen arbitrarily
                        if (containsFolderTemp)
                            horizontalTreeLineSize *= 0.5f;
                        DrawFolder(i);
                        const ImVec2 sizeToAdd(0.0f, ImGui::GetFontSize());
                        const ImRect childRect =
                            ImRect(currentPos.x, currentPos.y, currentPos.x + sizeToAdd.x, currentPos.y + sizeToAdd.y);
                        const float midpoint = (childRect.Min.y + childRect.Max.y) * 0.5f;
                        drawList->AddLine(ImVec2(verticalLineStart.x, midpoint),
                                          ImVec2(verticalLineStart.x + horizontalTreeLineSize, midpoint),
                                          treeLineColor);
                        verticalLineEnd.y = midpoint;
                        ImGui::Unindent(10.0f);
                    }
                }
                drawList->AddLine(verticalLineStart, verticalLineEnd, treeLineColor);
                ImGui::TreePop();
            }
            if (isOpen && !containsFolder)
                ImGui::TreePop();
        }
        if (m_IsDragging && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        {
            m_MovePath = dirInfo->GlobalPath;
        }
    }

    void ResourcePanel::Refresh()
    {
        m_ProjectPath          = FileSystem::GetExecutableDirectory().string();
        auto currentPath       = m_CurrentDir->GlobalPath;
        m_UpdateNavigationPath = true;
        m_Directories.clear();
        m_BaseProjectDir    = ProcessDirectory("res://", nullptr);
        m_PreviousDirectory = nullptr;
        m_CurrentDir        = nullptr;
        if (m_Directories.find(currentPath) != m_Directories.end())
            m_CurrentDir = m_Directories[currentPath].get();
        else
            ChangeDirectory(m_BaseProjectDir);
    }

} // namespace SnowLeopardEngine::Editor