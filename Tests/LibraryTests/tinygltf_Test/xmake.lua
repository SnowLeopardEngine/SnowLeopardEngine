-- add requirements
add_requires("tinygltf")

-- target defination, name: tinygltf_Test
target("tinygltf_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add packages
    add_packages("tinygltf")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/tinygltf_Test")

    -- copy gltf files
    on_load(function(target)
        os.cp("$(scriptdir)/assets", target:targetdir())
    end)