-- add requirements
add_requires("fmt", { configs = { header_only = true }})
add_requires("spdlog", { configs = { fmt_external = true }}) -- share one fmt with the engine instead of the bundled copy
add_requires("glm", "stduuid", "entt v3.16.0", "assimp")
add_requires("miniaudio", "stb", "tracy", "magic_enum", "cereal", "scoral", "cryptopp", "freetype")
-- one shared SDL for the engine DLL and the editor's imgui backend (two static copies would not share state)
add_requires("libsdl3 3.4.0", { configs = { shared = true }})
add_requires("joltphysics v5.5.0", { configs = { debug = is_mode("debug") }})
add_requires("ozz-animation 0.14.2", { configs = { debug = is_mode("debug") }})
-- rendering: VRI (Vulkan backend) is the only graphics API
add_requires("vri-vultra v0.2.0", {alias = "vri"})

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
    add_packages("libsdl3") -- private: SDL only appears in Platform/SDL3/SDL3Window.cpp
    add_packages("glm", { public = true })
    add_packages("stduuid", { public = true })
    add_packages("entt", { public = true })
    add_packages("assimp", { public = true })
    add_packages("miniaudio", { public = true })
    add_packages("stb", { public = true })
    add_packages("tracy", { public = true })
    add_packages("magic_enum", { public = true })
    add_packages("cereal", { public = true })
    add_packages("scoral", { public = true })
    add_packages("cryptopp", { public = true })
    add_packages("freetype", { public = true })
    add_packages("ozz-animation", { public = true })
    add_packages("joltphysics", { public = true })
    add_packages("vri", { public = true }) -- public: the editor draws ImGui through VRI

    -- add dependencies (inside project)
    add_deps("FastNoiseLite")

    -- defines
    if is_mode("debug") then
        add_defines("_DEBUG")
    else
        add_defines("NDEBUG")
    end

    -- enable tracy profiler
    add_defines("TRACY_ENABLE")

    -- set target directory
    set_targetdir("$(builddir)/$(plat)/$(arch)/$(mode)/SnowLeopardEngine")