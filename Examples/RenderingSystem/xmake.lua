-- target defination, name: RenderingSystem
target("RenderingSystem")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add deps
    add_deps("SnowLeopardEngine")

    -- add rules
    add_rules("copy_assets")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Examples/RenderingSystem")