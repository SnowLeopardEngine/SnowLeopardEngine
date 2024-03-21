#pragma once

#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEngine/Core/Base/Base.h"

#include "imgui.h"

namespace SnowLeopardEngine::Editor
{
    struct DirectoryInformation
    {
        DirectoryInformation*              Parent {nullptr};
        std::vector<DirectoryInformation*> Children;
        std::string                        LocalPath;
        std::string                        GlobalPath;
        std::string                        ThisPath;
        bool                               IsDirectory {};

    public:
        DirectoryInformation(const std::string& thisPath, DirectoryInformation* parent);
    };

    static std::string PathJoin(std::string_view base, std::string_view file)
    {
        if (base.empty())
        {
            return std::string(file);
        }
        if (!base.empty() && base.back() == '/' || (!file.empty() && file.front() == '/'))
        {
            return std::string(base) + std::string(file);
        }
        return std::string(base) + "/" + std::string(file);
    }

    static std::string SimplifyPath(std::string_view path)
    {
        std::string s = std::string(path);
        std::string drive;
        auto        p     = s.find("://");
        bool        found = false;
        if (p > 0)
        {
            bool onlyChars = true;
            for (int i = 0; i < p; i++)
            {
                if (!isalpha(s[i]))
                {
                    onlyChars = false;
                    break;
                }
            }
            if (onlyChars)
            {
                found = true;
                drive = s.substr(0, p + 3);
                s     = s.substr(p + 3);
            }
        }
        if (!found)
        {
            if (s.substr(0, 2) == "//" || s.substr(0, 2) == "\\\\")
            {
                drive = s.substr(0, 2);
                s     = s.substr(2);
            }
            else if (s[0] == '/' || s[0] == '\\')
            {
                drive = s.substr(0, 1);
                s     = s.substr(1);
            }
            else
            {
                p = s.find(":/");
                if (p == -1)
                {
                    p = s.find(":\\");
                }
                if (p != -1 && p < s.find('/'))
                {
                    drive = s.substr(0, p + 2);
                    s     = s.substr(p + 2);
                }
            }
        }
        std::replace(s.begin(), s.end(), '\\', '/');
        while (true)
        {
            auto compare = std::search_n(s.begin(), s.end(), 2, '/');
            if (compare == s.end())
            {
                break;
            }
            else
            {
                s.erase(compare, compare + 1);
            }
        }
        std::vector<std::string> dirs;
        std::string              token;
        size_t                   posStart = 0, posEnd;
        while ((posEnd = s.find('/', posStart)) != std::string::npos)
        {
            token    = s.substr(posStart, posEnd - posStart);
            posStart = posEnd + 1;
            if (!token.empty())
            {
                if (token == ".")
                {
                    continue;
                }
                else if (token == "..")
                {
                    if (!dirs.empty())
                    {
                        dirs.pop_back();
                    }
                }
                else
                {
                    dirs.push_back(token);
                }
            }
        }
        token = s.substr(posStart);
        if (!token.empty() && token != ".")
        {
            if (token == "..")
            {
                if (!dirs.empty())
                {
                    dirs.pop_back();
                }
            }
            else
            {
                dirs.push_back(token);
            }
        }
        s = "";
        for (const auto& dir : dirs)
        {
            s += "/" + dir;
        }
        if (s.empty())
        {
            s = "/";
        }
        return drive + s;
    }

    class ResourcePanel : public PanelBase
    {
    public:
        virtual void Init() override final;
        virtual void OnTick(float deltaTime) override final;
        virtual void Shutdown() override final;

        static bool           MoveFFile(const std::string& filePath, const std::string& movePath);
        void                  ChangeDirectory(DirectoryInformation* directory);
        bool                  RenderFile(int dirIndex, bool folder, int shownIndex, bool gridView);
        void                  RenderBottom();
        void                  DrawFolder(DirectoryInformation* dirInfo, bool defaultOpen = false);
        DirectoryInformation* ProcessDirectory(const std::string& directoryPath, DirectoryInformation* parent);
        void                  Refresh();

    private:
        std::string                                                            m_ProjectPath;
        std::string                                                            m_MovePath;
        std::string                                                            m_LastNavPath;
        bool                                                                   m_IsDragging;
        bool                                                                   m_IsInListView;
        bool                                                                   m_ShowHiddenFiles;
        int                                                                    m_GridItemsPerRow;
        float                                                                  m_GridSize = 50.0f;
        ImGuiTextFilter*                                                       m_Filter {nullptr};
        bool                                                                   m_UpdateNavigationPath;
        DirectoryInformation*                                                  m_CurrentDir {nullptr};
        DirectoryInformation*                                                  m_BaseProjectDir {nullptr};
        DirectoryInformation*                                                  m_NextDirectory {nullptr};
        DirectoryInformation*                                                  m_PreviousDirectory {nullptr};
        std::vector<DirectoryInformation*>                                     m_BreadCrumbData;
        std::unordered_map<std::string, std::unique_ptr<DirectoryInformation>> m_Directories;
    };

} // namespace SnowLeopardEngine::Editor