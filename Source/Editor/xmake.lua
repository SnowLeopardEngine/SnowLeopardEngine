-- add requirements
add_requires("imgui v1.90-docking", {configs = {glfw = true, opengl3 = true}})

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

    -- add packages
    add_packages("imgui", { public = true })

    -- add dependencies
    add_deps("SnowLeopardEngine")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/SnowLeopardEditor")

    -- copy config
    after_build(function(target)
        os.cp("$(scriptdir)/config/*", target:targetdir())
    end)