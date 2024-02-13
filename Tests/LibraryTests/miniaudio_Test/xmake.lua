-- add requirements
add_requires("miniaudio")

-- target defination, name: miniaudio_Test
target("miniaudio_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add packages
    add_packages("miniaudio")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/miniaudio_Test")

    -- copy sounds
    after_build(function(target)
        os.cp("$(scriptdir)/sounds", target:targetdir())
    end)