-- target defination, name: PhysX_Test
target("PhysX_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add rules
    add_rules("install_physx")

    -- link PhysX
    link_physx()

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/PhysX_Test")