-- target defination, name: AnimationPlayback
-- Plays two ozz clips on the Vampire model and checks skinning, transitions and blending.
target("AnimationPlayback")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- defines
    if is_mode("debug") then
        add_defines("_DEBUG")
    else
        add_defines("NDEBUG")
    end

    -- add deps
    add_deps("SnowLeopardEngine")

    -- add rules
    add_rules("copy_assets")

    -- set target directory
    set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)/Tests/EngineTests/AnimationPlayback")
