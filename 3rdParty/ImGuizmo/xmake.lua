-- imgui is required once, in Source/Editor/xmake.lua (a second add_requires here used to win and pinned the
-- editor to an older imgui)

-- define target
target("ImGuizmo")
    set_kind("static")
    add_headerfiles("ImGuizmo.h")
    add_files("ImGuizmo.cpp")
    add_packages("imgui", { public = true })
    add_includedirs(".", {public = true}) -- public: let other targets to auto include