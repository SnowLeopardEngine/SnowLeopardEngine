-- add requirements
add_requires("zlib v1.3.1", "assimp")

-- target defination, name: Assimp_Test
target("Assimp_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add packages
    add_packages("zlib v1.3.1", "assimp")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/Assimp_Test")

    -- copy fbx files
    after_build(function(target)
        os.cp("$(scriptdir)/assets", target:targetdir())
    end)