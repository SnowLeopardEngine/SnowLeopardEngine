-- SAssetPipeline: SnowLeopardEngine's asset importer (`sap`) and the cooked-asset runtime the engine links.
-- Kept in-tree until it is published to SnowLeopardEngine/xmake-repo. While developing both, the engine builds a local
-- checkout instead (`xmake f --sap_dir=../SAssetPipeline`).
package("sassetpipeline")
    set_homepage("https://github.com/SnowLeopardEngine/SAssetPipeline")
    set_description("The asset pipeline of SnowLeopardEngine: import-time cooking of textures, meshes and materials.")
    set_license("MIT")

    add_urls("https://github.com/SnowLeopardEngine/SAssetPipeline.git")
    add_versions("0.2.0", "4cd828e22a709cf32d11190b67ad0ac029cbf17d")

    -- The engine links only the runtime (cooked formats, registry) and runs the `sap` tool; tools that import in
    -- process (an editor reimporting on file changes) also link the importer.
    add_configs("importer", {description = "Link the importer libraries (Assimp, stb, BC7 encoder)", default = false, type = "boolean"})

    add_deps("nlohmann_json", "zstd", "assimp", "stb", "xxhash")

    on_load(function (package)
        package:addenv("PATH", "bin") -- the `sap` tool
        if package:config("importer") then
            -- the importer needs the vendored BC7 encoder after it, the runtime last
            package:add("links", "SAssetPipelineImport", "SAssetPipeline_bc7enc", "SAssetPipeline")
        else
            package:add("links", "SAssetPipeline")
        end
    end)

    on_install("windows", "linux", "macosx", function (package)
        import("package.tools.xmake").install(package, {sap_tests = false})
    end)

    on_test(function (package)
        assert(package:check_cxxsnippets({test = [[
            #include <SAssetPipeline/AssetRegistry.h>
            void test() {
                SnowLeopardEngine::AssetPipeline::AssetRegistry registry("Assets");
                registry.Load();
            }
        ]]}, {configs = {languages = "c++20"}}))
    end)
package_end()
