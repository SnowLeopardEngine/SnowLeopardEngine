// GPU-driven scene renderer regression test.
//
// Renders a small scene (floor, spheres, sun, environment) offscreen through its own SceneRenderer and render graph
// and reads the result back: the sky, a lit object, the sun's cascaded shadow on the floor, GPU frustum culling
// (draw counts per view), an alpha-blended transparent, CameraClearFlags::Color, a textured (bindless) material,
// a point light's attenuation, alpha-tested foliage (one shared mesh, its own culling views) and the in-game UI
// batch (painter's order, SDF text). Exit code = number of failures.

#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderGraph/RenderGraph.h"
#include "SnowLeopardEngine/Function/Rendering/Renderer/SceneRenderer.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

#include <cmath>
#include <cstdio>
#include <string>

using namespace SnowLeopardEngine;

namespace
{
    int g_Failures = 0;

    void Check(bool condition, const std::string& name, const std::string& detail = "")
    {
        std::printf("[%s] %-58s %s\n", condition ? "PASS" : "FAIL", name.c_str(), detail.c_str());
        if (!condition)
        {
            ++g_Failures;
        }
    }

    constexpr RHI::Extent kExtent {320, 180};

    struct Image
    {
        std::vector<uint8_t> Pixels;

        glm::ivec3 At(int x, int y) const
        {
            x = std::clamp(x, 0, int(kExtent.Width) - 1);
            y = std::clamp(y, 0, int(kExtent.Height) - 1);
            const auto* p = &Pixels[(size_t(y) * kExtent.Width + x) * 4];
            return {p[0], p[1], p[2]};
        }
        // Mean over a (2 r + 1)^2 block (robust to edges and PCF noise).
        glm::vec3 Mean(glm::ivec2 pixel, int r = 2) const
        {
            glm::vec3 sum(0);
            for (int dy = -r; dy <= r; ++dy)
                for (int dx = -r; dx <= r; ++dx)
                    sum += glm::vec3(At(pixel.x + dx, pixel.y + dy));
            return sum / float((2 * r + 1) * (2 * r + 1));
        }
    };

    std::string Text(glm::vec3 c)
    {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "(%.0f, %.0f, %.0f)", c.r, c.g, c.b);
        return buffer;
    }
    float Luma(glm::vec3 c) { return 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b; }

    Image Render(SceneRenderer& renderer, LogicScene& scene)
    {
        auto& device = g_EngineContext->RenderSys->GetDevice();
        Image image;
        if (!renderer.Prepare(&scene, kExtent))
        {
            return image;
        }
        RenderGraph graph(device);
        const auto  target = graph.CreateTexture("Output", RHI::ColorTextureDesc(kExtent, VriFormat_RGBA8_UNORM));
        renderer.AddPasses(graph, target, VriFormat_RGBA8_UNORM);
        graph.ExportResource(target);
        graph.Compile();

        RHI::Frame frame(device);
        auto*      cmd = frame.Begin();
        graph.Execute(cmd);
        auto&                output = graph.GetTexture(target);
        RHI::TextureReadback readback(device, output);
        readback.Record(cmd, output);
        frame.SubmitAndWait();
        image.Pixels = readback.Pixels();
        return image;
    }

    // World point -> pixel of the test camera (same matrices as the renderer).
    glm::ivec2 ToPixel(Entity camera, glm::vec3 world)
    {
        auto& component = camera.GetComponent<CameraComponent>();
        auto  view      = g_EngineContext->CameraSys->GetViewMatrix(camera.GetComponent<TransformComponent>());
        auto  clip      = g_EngineContext->CameraSys->GetProjectionMatrix(component) * view * glm::vec4(world, 1);
        glm::vec2 ndc   = glm::vec2(clip) / clip.w;
        return {int((ndc.x * 0.5f + 0.5f) * kExtent.Width), int((0.5f - ndc.y * 0.5f) * kExtent.Height)};
    }

    Entity AddPrimitive(LogicScene& scene, MeshPrimitiveType type, glm::vec3 position, glm::vec3 scale, const std::string& material)
    {
        Entity entity                                         = scene.CreateEntity("Object");
        entity.GetComponent<TransformComponent>().Position    = position;
        entity.GetComponent<TransformComponent>().Scale       = scale;
        entity.AddComponent<MeshFilterComponent>().PrimitiveType = type;
        entity.AddComponent<MeshRendererComponent>().MaterialFilePath = material;
        return entity;
    }
} // namespace

int main() TRY
{
    std::setvbuf(stdout, nullptr, _IONBF, 0); // results survive a crash
    EngineInitInfo initInfo {};
    initInfo.Window.Title = "Test - SceneRendering";
    initInfo.Window.Offline = true; // GPU work only: no visible window, no swapchain

    Engine engine;
    if (!engine.Init(initInfo))
    {
        std::fprintf(stderr, "Failed to init engine!\n");
        return 1;
    }
    auto& device = g_EngineContext->RenderSys->GetDevice();

    // The scene: a camera looking down -Z, a floor (top at y = 0), a sphere above it, the default sun.
    auto   scene  = g_EngineContext->SceneMngr->CreateScene("SceneRendering", true);
    Entity camera = scene->CreateEntity("Camera");
    camera.GetComponent<TransformComponent>().Position = {0, 4, 14};
    camera.GetComponent<TransformComponent>().SetRotationEuler({-15, 0, 0});
    auto& cameraComponent          = camera.AddComponent<CameraComponent>();
    cameraComponent.ClearFlags     = CameraClearFlags::Skybox;
    cameraComponent.ViewportWidth  = float(kExtent.Width);
    cameraComponent.ViewportHeight = float(kExtent.Height);

    AddPrimitive(*scene, MeshPrimitiveType::Cube, {0, -0.5f, 0}, {60, 1, 60}, "Assets/Materials/Next/White.dzmaterial");
    const glm::vec3 spherePosition {0, 3, 0};
    AddPrimitive(*scene, MeshPrimitiveType::Sphere, spherePosition, {2, 2, 2}, "Assets/Materials/Next/White.dzmaterial");

    engine.PostInit();
    engine.TickOneFrame(1.0f / 60.0f); // loads meshes and materials

    // The renderer is destroyed before the engine (and its device) shuts down.
    [&] {
    SceneRenderer renderer(device);

    // ---- no camera: nothing to render
    {
        auto empty = g_EngineContext->SceneMngr->CreateScene("Empty", false);
        Check(!renderer.Prepare(empty.get(), kExtent), "a scene without a camera is not rendered");
    }

    // ---- sky, lit sphere, sun shadow
    const auto sunDirection = glm::normalize(glm::vec3(-0.6f, -1.0f, -1.2f)); // DirectionalLightComponent default
    auto       image        = Render(renderer, *scene);
    Check(!image.Pixels.empty(), "scene renders", std::to_string(image.Pixels.size() / 4) + " pixels");
    if (image.Pixels.empty())
    {
        return;
    }
    const auto sky = image.Mean({int(kExtent.Width / 2), 6});
    Check(Luma(sky) > 120, "sky shows the environment map", Text(sky));

    const auto sphere = image.Mean(ToPixel(camera, spherePosition));
    Check(Luma(sphere) > 60 && glm::length(sphere - sky) > 10, "sphere is drawn and lit", Text(sphere));

    // Where the sphere's centre projects onto the floor along the sun, against a floor point mirrored across the
    // sun's plane of incidence (same distance, unshadowed).
    const auto shadowPoint = spherePosition + sunDirection * (spherePosition.y / -sunDirection.y);
    const auto litPoint    = glm::vec3(-shadowPoint.x + 6, 0, shadowPoint.z);
    const auto shadowed    = image.Mean(ToPixel(camera, shadowPoint));
    const auto lit         = image.Mean(ToPixel(camera, litPoint));
    // The sky (IBL) fills shadows, so the sun shadow is a clear but partial darkening.
    Check(Luma(lit) - Luma(shadowed) > 15, "sphere casts a cascaded shadow on the floor", Text(shadowed) + " vs " + Text(lit));

    // ---- GPU culling: draw counts per view
    auto counts = renderer.ReadDrawCounts();
    Check(counts[0] == 2, "camera view draws the 2 visible objects", std::to_string(counts[0]));
    Check(counts[1] >= 1, "first cascade draws the shadow casters", std::to_string(counts[1]));

    Entity behind = AddPrimitive(*scene, MeshPrimitiveType::Cube, {0, 2, 40}, {1, 1, 1}, "Assets/Materials/Next/White.dzmaterial");
    engine.TickOneFrame(1.0f / 60.0f);
    image  = Render(renderer, *scene);
    counts = renderer.ReadDrawCounts();
    Check(renderer.GetStatistics().Instances == 3 && counts[0] == 2, "object behind the camera is culled on the GPU",
          std::to_string(renderer.GetStatistics().Instances) + " instances, " + std::to_string(counts[0]) + " drawn");
    behind.GetComponent<EntityStatusComponent>().IsEnabled = false;

    // ---- transparent: alpha blended over the floor, kept in the in-place view
    const glm::vec3 glassPosition {-4, 1.5f, 6};
    AddPrimitive(*scene, MeshPrimitiveType::Sphere, glassPosition, {2, 2, 2}, "Assets/Materials/Next/RedTransparent.dzmaterial");
    const auto beforeGlass = image.Mean(ToPixel(camera, glassPosition));
    engine.TickOneFrame(1.0f / 60.0f);
    image        = Render(renderer, *scene);
    counts       = renderer.ReadDrawCounts();
    const auto glass = image.Mean(ToPixel(camera, glassPosition));
    Check(counts[5] == 1, "transparent view has the glass sphere", std::to_string(counts[5]));
    Check(glass.r > glass.g + 25 && glass.g > 0.25f * beforeGlass.g, "transparent sphere blends red over the scene",
          Text(glass) + " over " + Text(beforeGlass));

    // ---- textured material through the bindless table
    const glm::vec3 ironPosition {4, 1.5f, 6};
    AddPrimitive(*scene, MeshPrimitiveType::Sphere, ironPosition, {2, 2, 2}, "Assets/Materials/Next/RustedIronPBRDeferred.dzmaterial");
    engine.TickOneFrame(1.0f / 60.0f);
    image          = Render(renderer, *scene);
    const auto iron = image.Mean(ToPixel(camera, ironPosition));
    Check(renderer.GetStatistics().Textures >= 4, "material maps are in the bindless table",
          std::to_string(renderer.GetStatistics().Textures) + " textures");
    Check(iron.r > iron.b + 10, "rusted iron sphere samples its albedo map", Text(iron));

    // ---- point light: brightens the floor near it, falling off with distance
    const auto floorNear = ToPixel(camera, {6, 0, 9});
    const auto floorFar  = ToPixel(camera, {-6, 0, 9});
    const auto nearBefore = image.Mean(floorNear), farBefore = image.Mean(floorFar);
    Entity light = scene->CreateEntity("Point Light");
    light.GetComponent<TransformComponent>().Position = {6, 1, 9};
    auto& pointLight     = light.AddComponent<PointLightComponent>();
    pointLight.Color     = {0, 0, 1};
    pointLight.Intensity = 20;
    image                = Render(renderer, *scene);
    const auto nearAfter = image.Mean(floorNear), farAfter = image.Mean(floorFar);
    Check(nearAfter.b - nearBefore.b > 2 * std::max(farAfter.b - farBefore.b, 1.0f),
          "point light attenuates with distance",
          "near +" + std::to_string(int(nearAfter.b - nearBefore.b)) + ", far +" + std::to_string(int(farAfter.b - farBefore.b)));

    // ---- alpha-tested grass: 40 entities, each with its own quad mesh, share one arena range
    const auto uniqueBefore = renderer.GetStatistics().UniqueMeshes;
    const auto patch        = ToPixel(camera, {10, 1, -7}); // clear of the spheres
    auto greenPixels = [&](const Image& img) {
        int count = 0;
        for (int dy = -12; dy <= 12; ++dy)
            for (int dx = -12; dx <= 12; ++dx)
            {
                const auto c = img.At(patch.x + dx, patch.y + dy);
                count += c.g > c.r + 25 && c.g > c.b + 25;
            }
        return count;
    };
    const int bareGreen = greenPixels(image);
    for (int i = 0; i < 40; ++i)
    {
        Entity blade = AddPrimitive(*scene, MeshPrimitiveType::Quad, {8.0f + float(i % 8) * 0.6f, 1, -8.0f + float(i / 8) * 0.6f}, {1, 2, 1},
                                    "Assets/Materials/Next/Grass.dzmaterial");
        blade.GetComponent<TransformComponent>().SetRotationEuler({-90, float(i * 37 % 90), 0});
    }
    engine.TickOneFrame(1.0f / 60.0f);
    image  = Render(renderer, *scene);
    counts = renderer.ReadDrawCounts();
    Check(renderer.GetStatistics().AlphaTest == 40, "grass is in the alpha-test queue", std::to_string(renderer.GetStatistics().AlphaTest));
    Check(renderer.GetStatistics().UniqueMeshes == uniqueBefore + 1, "40 identical quads share one mesh",
          std::to_string(renderer.GetStatistics().UniqueMeshes - uniqueBefore) + " new");
    Check(counts[SceneRenderer::kViewAlphaTest] > 0 && counts[SceneRenderer::kViewAlphaTest] <= 40,
          "alpha-test view is culled on the GPU", std::to_string(counts[SceneRenderer::kViewAlphaTest]) + " drawn");
    const int grassGreen = greenPixels(image);
    Check(bareGreen == 0 && grassGreen > 30, "grass is drawn (cut out, lit)",
          std::to_string(grassGreen) + " green pixels (bare floor " + std::to_string(bareGreen) + ")");

    // ---- in-game UI: one batch, painter's order by Pos.z, SDF text
    auto addImage = [&](glm::vec3 pos, glm::vec4 color) {
        Entity entity = scene->CreateEntity("Image");
        auto&  rect   = entity.AddComponent<UI::RectTransformComponent>();
        rect.Pos      = pos;
        rect.Size     = {40, 30};
        entity.AddComponent<UI::ImageComponent>().Color = color;
    };
    addImage({60, 150, 0.5f}, {0, 1, 0, 1}); // in front
    addImage({60, 150, 0.0f}, {1, 0, 0, 1}); // behind, created later
    Entity label = scene->CreateEntity("Label");
    label.AddComponent<UI::RectTransformComponent>().Pos = {200, 140, 0};
    auto& text       = label.AddComponent<UI::TextComponent>();
    text.TextContent = "UI";
    text.FontSize    = 32;
    text.Color       = {0, 0, 1, 1};
    image            = Render(renderer, *scene);
    const auto panel = image.At(60, int(kExtent.Height) - 150);
    Check(panel == glm::ivec3(0, 255, 0), "UI quads are drawn in Pos.z order", Text(glm::vec3(panel)));
    int blue = 0;
    for (int y = int(kExtent.Height) - 170; y < int(kExtent.Height) - 135; ++y)
        for (int x = 195; x < 250; ++x)
        {
            const auto c = image.At(x, y);
            blue += c.b > 200 && c.r < 60 && c.g < 60;
        }
    Check(renderer.GetStatistics().UIQuads == 4 && blue > 40, "text is drawn from the SDF atlas",
          std::to_string(renderer.GetStatistics().UIQuads) + " quads, " + std::to_string(blue) + " glyph pixels");

    // ---- CameraClearFlags::Color replaces the sky
    cameraComponent.ClearFlags = CameraClearFlags::Color;
    cameraComponent.ClearColor = {1, 0, 1, 1};
    image                      = Render(renderer, *scene);
    const auto cleared         = image.Mean({int(kExtent.Width / 2), 6});
    Check(cleared.r > cleared.g + 60 && cleared.b > cleared.g + 60, "ClearFlags Color shows the clear colour", Text(cleared));
    }();

    engine.Shutdown();
    std::printf("%d failure(s)\n", g_Failures);
    return g_Failures;
}
CATCH
