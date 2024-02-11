-- add requirements
add_requires("spdlog", "glfw", "glm")

-- target defination, name: SnowLeopardEngine
target("SnowLeopardEngine")
    -- set target kind: static library
    set_kind("static")

    -- add include dir
    add_includedirs("include", {public = true}) -- public: let other targets to auto include

    -- set precompiled header file (PCH)
    set_pcxxheader("include/SnowLeopardEngine/PrecompiledHeader.h")

    -- add header files
    add_headerfiles("include/(SnowLeopardEngine/**.h)")

    -- add source files
    add_files("src/**.cpp")

    -- add packages
    add_packages("spdlog", { public = true })
    add_packages("glfw", { public = true })
    add_packages("glm", { public = true })

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/SnowLeopardEngine")