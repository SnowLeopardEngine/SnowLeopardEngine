-- add requirements
add_requires("freetype", "glfw", "glm")

-- target defination, name: FreeType_Test
target("FreeType_Test")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- add packages
    add_packages("freetype", "glfw", "glm")
    add_deps("glad")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/LibraryTests/FreeType_Test")

    -- copy fonts
    after_build(function(target)
        os.cp("$(scriptdir)/assets", target:targetdir())
    end)