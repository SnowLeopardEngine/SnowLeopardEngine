-- target defination, name: NativeScripting
target("NativeScripting")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add deps
    add_deps("SnowLeopardEngine")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Examples/NativeScripting")