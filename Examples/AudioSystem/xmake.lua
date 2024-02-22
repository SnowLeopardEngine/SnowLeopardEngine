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

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Examples/AudioSystem")

    -- copy sounds
    after_build(function(target)
        os.cp("$(scriptdir)/sounds", target:targetdir())
    end)