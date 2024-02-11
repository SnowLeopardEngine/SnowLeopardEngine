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
add_rules("mode.debug", "mode.release")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

-- add our own xmake-repo here
add_repositories("snow-leopard-engine-xmake-repo https://github.com/SnowLeopardEngine/xmake-repo.git dev")

-- include source
includes("Source")

-- if build examples, then include examples
if has_config("examples") then
    includes("Examples")
end