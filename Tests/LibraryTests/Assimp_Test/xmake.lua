-- add requirements
add_requireconfs("assimp.zlib", {system = false}) -- https://github.com/xmake-io/xmake-repo/issues/1855
add_requires("assimp", { configs = { debug = is_mode("debug"), shared = true }})

-- target defination, name: Assimp_Test
target("Assimp_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add packages
    add_packages("assimp")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/Assimp_Test")

    -- copy fbx files
    after_build(function(target)
        os.cp("$(scriptdir)/assets", target:targetdir())
    end)