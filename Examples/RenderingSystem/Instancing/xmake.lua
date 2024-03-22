-- target defination, name: RenderingSystem-Instancing
target("RenderingSystem-Instancing")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- defines
    add_defines("PX_PHYSX_STATIC_LIB") -- force static lib https://github.com/NVIDIAGameWorks/PhysX/issues/260
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
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Examples/RenderingSystem/Instancing")