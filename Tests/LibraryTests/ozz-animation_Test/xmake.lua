-- add requirements
add_requires("ozz-animation", { configs = { shared = true }})
add_requires("raylib")

-- target defination, name: ozz-animation_Test
target("ozz-animation_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add packages
    add_packages("ozz-animation")
    add_packages("raylib")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/ozz-animation_Test")

    -- copy ozz files
    on_load(function(target)
        os.cp("$(scriptdir)/assets", target:targetdir())
    end)