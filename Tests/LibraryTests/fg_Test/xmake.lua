-- add requirements
add_requires("fg")

-- target defination, name: fg_Test
target("fg_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add packages
    add_packages("fg")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/fg_Test")