-- add requirements
add_requires("openfbx")

-- target defination, name: OpenFBX_Test
target("OpenFBX_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add packages
    add_packages("openfbx")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/OpenFBX_Test")

    -- copy fbx files
    after_build(function(target)
        os.cp("$(scriptdir)/assets", target:targetdir())
    end)