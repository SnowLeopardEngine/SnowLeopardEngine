-- set project name
set_project("SnowLeopardEngine")

-- set project version
set_version("0.1.0")

-- set language version: C++ 17
set_languages("cxx17")

-- global options
option("examples") -- build examples?
    set_default(true)
option_end()

-- if build on windows
if is_host("windows") then
    add_cxxflags("/Zc:__cplusplus", {tools = {"msvc", "cl"}}) -- fix __cplusplus == 199711L error
    add_cxxflags("/bigobj") -- avoid big obj
    set_runtimes("MD")
end

-- global rules
rule("install_physx")
    before_build(function(target)
        print("Installing PhysX SDK 5.1.2...")
        if is_plat("windows") then
            os.cd("$(projectdir)/Deps/SPhysX-Cross")
            os.run("powershell.exe .\\download_prebuilt_sdk_windows.ps1")
        elseif is_plat("macosx") then
            os.cd("$(projectdir)/Deps/SPhysX-Cross")
            os.run("./download_prebuilt_sdk_macosx.sh")
        end
    end)

    after_build(function(target)
        if is_plat("windows") then
            -- copy dll
            print("Copying PhysX DLLs...")
            os.cp("$(projectdir)/Deps/SPhysX-Cross/Prebuilt/Libraries/windows/$(arch)/$(mode)/dll/*", target:targetdir())
        end
    end)
rule_end()

add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

-- add our own xmake-repo here
add_repositories("snow-leopard-engine-xmake-repo https://github.com/SnowLeopardEngine/xmake-repo.git dev")

function link_physx()
    -- add include dir
    add_includedirs("$(projectdir)/Deps/SPhysX-Cross/PhysX-CMake/physx/include", {public = true}) -- public: let other targets to auto include

    -- links
    add_linkdirs("$(projectdir)/Deps/SPhysX-Cross/Prebuilt/Libraries/$(plat)/$(arch)/$(mode)/")
    add_links("PhysX_static")
    add_links("PhysXCharacterKinematic_static")
    add_links("PhysXCommon_static")
    add_links("PhysXCooking_static")
    add_links("PhysXExtensions_static")
    add_links("PhysXFoundation_static")
    add_links("PhysXPvdSDK_static")
    add_links("PhysXVehicle_static")
end

-- include source
includes("Source")

-- include tests
includes("Tests")

-- if build examples, then include examples
if has_config("examples") then
    includes("Examples")
end