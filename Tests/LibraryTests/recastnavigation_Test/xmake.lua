add_requires("tinyobjloader", { configs = { debug = is_mode("debug") }})
add_requires("recastnavigation", { configs = { debug = is_mode("debug") }})

-- target defination, name: recastnavigation_Test
target("recastnavigation_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    add_packages("tinyobjloader")
    add_packages("recastnavigation")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/recastnavigation_Test")

    -- copy assets
    on_load(function(target)
        os.cp("$(scriptdir)/assets", target:targetdir())
    end)