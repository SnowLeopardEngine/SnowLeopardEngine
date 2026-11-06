-- target defination, name: PhysicsScenarios
-- Engine-level physics regression test: steps fixed ticks deterministically and checks body positions.
target("PhysicsScenarios")
    -- set target kind: executable
    set_kind("binary")

    -- add source files
    add_files("main.cpp")

    -- defines
    if is_mode("debug") then
        add_defines("_DEBUG")
    else
        add_defines("NDEBUG")
    end

    -- add deps
    add_deps("SnowLeopardEngine")

    -- add rules
    add_rules("copy_assets")

    -- set target directory
    set_targetdir("$(buildir)/$(plat)/$(arch)/$(mode)/Tests/EngineTests/PhysicsScenarios")
