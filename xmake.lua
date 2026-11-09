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

-- SAssetPipeline (asset importer + cooked asset runtime): the package, or a local checkout while developing both
-- (`xmake f --sap_dir=../SAssetPipeline`).
option("sap_dir")
    set_default("")
    set_showmenu(true)
    set_description("Build SAssetPipeline from this checkout instead of using the package")
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
        -- the cooked assets of this build (imported after the load-time copy of Assets/)
        if os.isdir("$(projectdir)/Assets/.imported") then
            os.cp("$(projectdir)/Assets/.imported", path.join(target:targetdir(), "Assets"))
        end

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
        -- cooked assets are imported before the apps build (ImportAssets); the load-time copy may predate them
        if target:name() ~= "ImportAssets" then
            target:add("deps", "ImportAssets")
        end
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

-- SAssetPipeline
if get_config("sap_dir") and get_config("sap_dir") ~= "" then
    includes(path.join(get_config("sap_dir"), "Source"))
else
    add_requires("sassetpipeline")
end

-- Imports Assets/ (incremental: only new or changed sources are cooked into Assets/.imported). Every app depends on it.
target("ImportAssets")
    set_kind("phony")
    if get_config("sap_dir") and get_config("sap_dir") ~= "" then
        add_deps("sap")
    else
        add_packages("sassetpipeline")
    end
    on_build(function (target)
        local sap
        local tool = target:dep("sap")
        if tool then
            sap = tool:targetfile()
        else
            sap = path.join(target:pkg("sassetpipeline"):installdir(), "bin", is_host("windows") and "sap.exe" or "sap")
        end
        os.execv(sap, {"import", path.join(os.projectdir(), "Assets"), "--quiet"})
    end)
target_end()

-- include source
includes("Source")

-- include tests
includes("Tests")

-- if build examples, then include examples
if has_config("examples") then
    includes("Examples")
end
