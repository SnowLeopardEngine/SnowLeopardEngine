-- target defination, name: AudioSystem
target("AudioSystem")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add deps
    add_deps("SnowLeopardEngine")

    -- add rules
    add_rules("copy_assets")

    -- defines
    add_defines("PX_PHYSX_STATIC_LIB") -- force static lib https://github.com/NVIDIAGameWorks/PhysX/issues/260
    if is_mode("debug") then
        add_defines("_DEBUG")
    else
        add_defines("NDEBUG")
    end

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Examples/AudioSystem")

    -- copy sounds
    after_build(function(target)
        os.cp("$(scriptdir)/sounds", target:targetdir())
    end)