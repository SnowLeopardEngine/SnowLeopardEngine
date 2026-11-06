-- set project name
set_project("SnowLeopardEngine")

-- set project version
set_version("0.1.0")

-- set language version: C++ 20
set_languages("cxx20")

-- global options
option("examples") -- build examples?
    set_default(true)
option_end()

-- if build on windows
if is_plat("windows") then
    add_cxxflags("/Zc:__cplusplus", {tools = {"msvc", "cl"}}) -- fix __cplusplus == 199711L error
    add_cxxflags("/bigobj") -- avoid big obj
    add_cxxflags("-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING")
    add_cxxflags("/EHsc")
    add_cxflags("/utf-8", {tools = {"msvc", "cl"}}) -- required by fmt >= 11 (and spdlog through it)
    if is_mode("debug") then
        set_runtimes("MDd")
        add_links("ucrtd")
    else
        set_runtimes("MD")
    end
else
    add_cxxflags("-fexceptions")
end

-- global rules
rule("copy_assets")
    after_build(function(target)
        print("Install C# bindings for target: " .. target:name())
        os.cp("$(builddir)/assembly/SnowLeopardEngine/*", target:targetdir())

        -- shared packages of the engine (SDL3, the Slang compiler) have to sit next to every executable
        local engine = target:dep("SnowLeopardEngine")
        for _, name in ipairs({"libsdl3", "slang-prebuilt"}) do
            local pkg = engine and engine:pkg(name)
            if pkg then
                if is_plat("windows", "mingw") then
                    os.cp(path.join(pkg:installdir(), "bin", "*.dll"), target:targetdir())
                elseif is_plat("macosx") then
                    os.cp(path.join(pkg:installdir(), "lib", "*.dylib"), target:targetdir())
                else
                    os.cp(path.join(pkg:installdir(), "lib", "*.so*"), target:targetdir(), {symlink = true})
                end
            end
        end
    end)
    on_load(function(target)
        if not is_plat("windows", "mingw") then
            target:add("rpathdirs", "@loader_path", "$ORIGIN") -- find the copied shared libraries next to the binary
        end
        -- https://xmake.io/#/zh-cn/manual/builtin_modules?id=oscp
        -- copy the folder itself: "Assets/*" with rootdir flattened sub-folders on a target's first build
        os.cp("$(projectdir)/Assets", target:targetdir())
        os.cp("$(projectdir)/Tools/$(plat)/$(arch)/Coral/Coral**", target:targetdir())
    end)
rule_end()

add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

-- add our own xmake-repo here
add_repositories("snow-leopard-engine-xmake-repo https://github.com/SnowLeopardEngine/xmake-repo.git dev-next")
-- in-tree recipes not yet published to the package repo (VRI as libvultra builds it)
add_repositories("snow-leopard-engine-local xmake")

-- include 3rdParty libraries
includes("3rdParty")

-- include source
includes("Source")

-- include tests
includes("Tests")

-- if build examples, then include examples
if has_config("examples") then
    includes("Examples")
end
