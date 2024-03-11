-- add requirements
add_requires("imgui v1.90.1-docking", {configs = {glfw = true, opengl3 = true, wchar32 = true}})

-- define target
target("ImGuizmo")
    set_kind("static")
    add_headerfiles("ImGuizmo.h")
    add_files("ImGuizmo.cpp")
    add_packages("imgui", { public = true })
    add_includedirs(".", {public = true}) -- public: let other targets to auto include