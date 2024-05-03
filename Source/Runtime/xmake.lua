-- add requirements
add_requires("spdlog", "fmt 9.1.0", "glfw", "glm", "stduuid", "entt v3.10.3")
add_requires("miniaudio", "stb", "tracy", "magic_enum", "cereal", "shaderc", "spirv-cross", "scoral", "cryptopp", "fg", "freetype")
add_requires("sphysx-cross", { configs = { debug = is_mode("debug") }})
add_requires("ozz-animation 0.14.2", { configs = { debug = is_mode("debug") }})

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
    add_packages("fmt", { public = true })
    add_packages("glfw", { public = true })
    add_packages("glm", { public = true })
    add_packages("stduuid", { public = true })
    add_packages("entt", { public = true })
    add_packages("miniaudio", { public = true })
    add_packages("stb", { public = true })
    add_packages("tracy", { public = true })
    add_packages("magic_enum", { public = true })
    add_packages("cereal", { public = true })
    add_packages("shaderc", { public = true })
    add_packages("spirv-cross", { public = true })
    add_packages("scoral", { public = true })
    add_packages("cryptopp", { public = true })
    add_packages("fg", { public = true })
    add_packages("freetype", { public = true })
    add_packages("ozz-animation", { public = true })
    add_packages("sphysx-cross", { public = true })

    -- add dependencies (inside project)
    add_deps("glad")
    add_deps("FastNoiseLite")

    -- defines
    add_defines("PX_PHYSX_STATIC_LIB") -- force static lib https://github.com/NVIDIAGameWorks/PhysX/issues/260
    if is_mode("debug") then
        add_defines("_DEBUG")
    else
        add_defines("NDEBUG")
    end

    -- enable tracy profiler
    add_defines("TRACY_ENABLE")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/SnowLeopardEngine")