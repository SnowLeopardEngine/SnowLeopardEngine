-- add requirements
add_requires("imgui v1.92.5-docking", {configs = {sdl3 = true, wchar32 = true}})
add_requireconfs("imgui.libsdl3", {version = "3.4.0", configs = {shared = true}})
add_requires("argparse")

-- target defination, name: SnowLeopardEditor
target("SnowLeopardEditor")
    -- set target kind: executable
    set_kind("binary")

    -- add include dir
    add_includedirs("include", {public = true}) -- public: let other targets to auto include

    -- add header files
    add_headerfiles("include/(SnowLeopardEditor/**.h)")

    -- add rules
    add_rules("copy_assets")

    -- add source files
    add_files("src/**.cpp")

    -- defines
    if is_mode("debug") then
        add_defines("_DEBUG")
    else
        add_defines("NDEBUG")
    end

    -- add packages
    add_packages("imgui", { public = true })
    add_packages("libsdl3")
    add_packages("argparse", { public = true })

    -- add dependencies
    add_deps("SnowLeopardEngine", "ImGuizmo", "IconFontCppHeaders")

    -- set target directory
    set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)/SnowLeopardEditor")

    -- default arguments for `xmake run SnowLeopardEditor`; arguments given on the command line replace them
    set_runargs("--project", path.join(os.scriptdir(), "../../ExampleProject.dzproj"))
