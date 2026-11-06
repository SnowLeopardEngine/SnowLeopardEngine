-- add requirements
add_requires("joltphysics v5.5.0", { configs = { debug = is_mode("debug") }})

-- target defination, name: JoltPhysics_Test
target("JoltPhysics_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add packages
    add_packages("joltphysics")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/JoltPhysics_Test")
