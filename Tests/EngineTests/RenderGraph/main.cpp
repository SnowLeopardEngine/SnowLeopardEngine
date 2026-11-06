// Render graph regression test (VRI device of the engine's render system).
//
// Plan checks: culling of passes that feed nothing, transient aliasing only for disjoint lifetimes, and the
// validation errors (read before write, missing usage flags, a resource used twice in one pass).
// GPU checks: passes really run in order with the barriers the graph derives - clear -> copy into an export,
// CaptureAfterPass keeps an intermediate value that a later pass overwrites - verified by reading pixels back.
// Exit code = number of failures.

#include "SnowLeopardEngine/Engine/Debug.h"
#include "SnowLeopardEngine/Engine/Engine.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderGraph/RenderGraph.h"

#include <cstdio>
#include <stdexcept>
#include <array>
#include <string>

using namespace SnowLeopardEngine;

namespace
{
    int g_Failures = 0;

    void Check(bool condition, const std::string& name, const std::string& detail = "")
    {
        std::printf("[%s] %-56s %s\n", condition ? "PASS" : "FAIL", name.c_str(), detail.c_str());
        if (!condition)
        {
            ++g_Failures;
        }
    }

    template<typename F>
    bool Throws(F&& f)
    {
        try
        {
            f();
        }
        catch (const std::exception&)
        {
            return true;
        }
        return false;
    }

    constexpr RHI::Extent kExtent {8, 8};

    VriTextureDesc Target() { return RHI::ColorTextureDesc(kExtent, VriFormat_RGBA8_UNORM); }

    RenderGraph::ExecutePass ClearTo(RenderGraph::Resource target, float r, float g, float b)
    {
        return [=](VriCommandBuffer* cmd, RenderGraph& graph) {
            auto&       texture = graph.GetTexture(target);
            const float color[4] {r, g, b, 1.0f};
            RHI::BeginColorPass(graph.GetDevice(), cmd, texture.View(), texture.GetExtent(), color);
            graph.GetDevice().Core.CmdEndRendering(cmd);
        };
    }

    RenderGraph::ExecutePass CopyTexture(RenderGraph::Resource source, RenderGraph::Resource destination)
    {
        return [=](VriCommandBuffer* cmd, RenderGraph& graph) {
            VriTextureCopyDesc region {};
            region.src.layerNum = 1;
            region.dst.layerNum = 1;
            region.src.aspect   = VriImageAspect_Color;
            region.dst.aspect   = VriImageAspect_Color;
            graph.GetDevice().Core.CmdCopyTexture(
                cmd, graph.GetTexture(destination).Handle, graph.GetTexture(source).Handle, &region);
        };
    }

    // Executes the graph and returns the RGBA8 value of one texel of each texture.
    std::vector<std::array<uint8_t, 4>> RunAndRead(RenderGraph& graph, std::initializer_list<RenderGraph::Resource> outputs)
    {
        auto&                                    device = graph.GetDevice();
        std::vector<Scope<RHI::TextureReadback>> readbacks;
        RHI::Frame                               frame(device);
        auto*                                    cmd = frame.Begin();
        graph.Execute(cmd);
        for (auto output : outputs)
        {
            auto& texture = graph.GetTexture(output);
            readbacks.push_back(CreateScope<RHI::TextureReadback>(device, texture));
            readbacks.back()->Record(cmd, texture);
        }
        frame.SubmitAndWait();
        std::vector<std::array<uint8_t, 4>> result;
        for (auto& readback : readbacks)
        {
            const auto pixels = readback->Pixels();
            const auto center = (size_t(kExtent.Height / 2) * kExtent.Width + kExtent.Width / 2) * 4;
            result.push_back({pixels[center], pixels[center + 1], pixels[center + 2], pixels[center + 3]});
        }
        return result;
    }

    std::string Rgba(const std::array<uint8_t, 4>& c)
    {
        return "(" + std::to_string(c[0]) + ", " + std::to_string(c[1]) + ", " + std::to_string(c[2]) + ", " +
               std::to_string(c[3]) + ")";
    }

    bool Is(const std::array<uint8_t, 4>& c, uint8_t r, uint8_t g, uint8_t b)
    {
        return c[0] == r && c[1] == g && c[2] == b && c[3] == 255;
    }
} // namespace

int main() TRY
{
    EngineInitInfo initInfo {};
    initInfo.Window.Title = "Test - RenderGraph";

    Engine engine;
    if (!engine.Init(initInfo))
    {
        std::fprintf(stderr, "Failed to init engine!\n");
        return 1;
    }
    auto& device = g_EngineContext->RenderSys->GetDevice();

    // ---- culling: only producers of exports / side effects stay live
    {
        RenderGraph graph(device);
        auto        used   = graph.CreateTexture("Used", Target());
        auto        unused = graph.CreateTexture("Unused", Target());
        auto        output = graph.CreateTexture("Output", Target());
        graph.AddPass("Write used", {{used, GraphUsage::ColorWrite, true}}, ClearTo(used, 1, 0, 0));
        graph.AddPass("Write unused", {{unused, GraphUsage::ColorWrite, true}}, ClearTo(unused, 0, 1, 0));
        graph.AddPass("Copy", {{used, GraphUsage::CopySource}, {output, GraphUsage::CopyDestination, true}}, CopyTexture(used, output));
        graph.ExportResource(output);
        graph.Compile();
        const auto passes = graph.GetActivePasses();
        Check(passes == std::vector<std::string> {"Write used", "Copy"}, "pass feeding nothing is culled",
              std::to_string(passes.size()) + " live");
        Check(!graph.GetResourceInfo(unused).Active, "culled pass's texture is inactive");

        const auto pixel = RunAndRead(graph, {output});
        Check(Is(pixel[0], 255, 0, 0), "clear then copy reaches the export", Rgba(pixel[0]));
    }

    // ---- a discarding write does not keep the previous writer alive
    {
        RenderGraph graph(device);
        auto        target = graph.CreateTexture("Target", Target());
        graph.AddPass("First", {{target, GraphUsage::ColorWrite, true}}, ClearTo(target, 1, 0, 0));
        graph.AddPass("Second", {{target, GraphUsage::ColorWrite, true}}, ClearTo(target, 0, 0, 1));
        graph.ExportResource(target);
        graph.Compile();
        Check(graph.GetActivePasses() == std::vector<std::string> {"Second"}, "overwritten producer is culled");
        const auto pixel = RunAndRead(graph, {target});
        Check(Is(pixel[0], 0, 0, 255), "last write wins", Rgba(pixel[0]));
    }

    // ---- CaptureAfterPass keeps an intermediate value
    {
        RenderGraph graph(device);
        auto        target = graph.CreateTexture("Target", Target());
        graph.AddPass("Red", {{target, GraphUsage::ColorWrite, true}}, ClearTo(target, 1, 0, 0));
        graph.AddPass("Green", {{target, GraphUsage::ColorWrite, true}}, ClearTo(target, 0, 1, 0));
        auto captured = graph.CaptureAfterPass("Red", target, "After red");
        graph.ExportResource(target);
        graph.Compile();
        const auto pixels = RunAndRead(graph, {captured, target});
        Check(Is(pixels[0], 255, 0, 0), "capture sees the value after its pass", Rgba(pixels[0]));
        Check(Is(pixels[1], 0, 255, 0), "later write still lands", Rgba(pixels[1]));
    }

    // ---- aliasing: identical transients with disjoint lifetimes share memory
    {
        RenderGraph graph(device);
        auto        a      = graph.CreateTexture("A", Target());
        auto        b      = graph.CreateTexture("B", Target());
        auto        c      = graph.CreateTexture("C", Target());
        auto        out1   = graph.CreateTexture("Out1", Target());
        auto        out2   = graph.CreateTexture("Out2", Target());
        graph.AddPass("Write A", {{a, GraphUsage::ColorWrite, true}}, ClearTo(a, 1, 0, 0));
        graph.AddPass("A to Out1", {{a, GraphUsage::CopySource}, {out1, GraphUsage::CopyDestination, true}}, CopyTexture(a, out1));
        // B starts after A's last use: it may reuse A. C overlaps B: it may not.
        graph.AddPass("Write B", {{b, GraphUsage::ColorWrite, true}}, ClearTo(b, 0, 1, 0));
        graph.AddPass("Write C", {{c, GraphUsage::ColorWrite, true}}, ClearTo(c, 0, 0, 1));
        graph.AddPass("B + C to Out2",
                      {{b, GraphUsage::CopySource}, {c, GraphUsage::Sampled}, {out2, GraphUsage::CopyDestination, true}},
                      CopyTexture(b, out2));
        graph.ExportResource(out1);
        graph.ExportResource(out2);
        graph.Compile(true);
        const auto infoA = graph.GetResourceInfo(a);
        const auto infoB = graph.GetResourceInfo(b);
        const auto infoC = graph.GetResourceInfo(c);
        Check(infoB.Allocation == infoA.Allocation, "disjoint transient reuses memory");
        Check(infoC.Allocation != infoB.Allocation, "overlapping transients get separate memory");
        Check(graph.GetResourceInfo(out1).Allocation != graph.GetResourceInfo(out2).Allocation, "exports never alias");
        Check(Throws([&] { graph.CreateTexture("Late", Target()); }), "aliased plan rejects edits");
        const auto pixels = RunAndRead(graph, {out1, out2});
        Check(Is(pixels[0], 255, 0, 0) && Is(pixels[1], 0, 255, 0), "aliased resources keep their values",
              Rgba(pixels[0]) + " " + Rgba(pixels[1]));
    }

    // ---- imported texture, rebound to another of the same shape
    {
        RHI::Texture first(device, Target());
        RHI::Texture second(device, Target());
        RenderGraph  graph(device);
        auto         target = graph.ImportResource("Imported", first, false);
        graph.AddPass("Fill", {{target, GraphUsage::ColorWrite, true}}, ClearTo(target, 1, 1, 0));
        graph.AddPass("Keep", {{target, GraphUsage::Sampled}}, [](VriCommandBuffer*, RenderGraph&) {}, true);
        graph.Compile();
        auto pixel = RunAndRead(graph, {target});
        Check(Is(pixel[0], 255, 255, 0), "side-effect pass keeps the import's producer", Rgba(pixel[0]));
        graph.Bind(target, second);
        Check(&graph.GetTexture(target) == &second, "Bind swaps the imported texture");
        pixel = RunAndRead(graph, {target});
        Check(Is(pixel[0], 255, 255, 0), "rebound import is written", Rgba(pixel[0]));
        RHI::Texture other(device, RHI::ColorTextureDesc({4, 4}));
        Check(Throws([&] { graph.Bind(target, other); }), "Bind rejects another extent");
    }

    // ---- validation
    {
        RenderGraph graph(device);
        auto        never = graph.CreateTexture("Never written", Target());
        auto        out   = graph.CreateTexture("Out", Target());
        graph.AddPass("Reads too early", {{never, GraphUsage::Sampled}, {out, GraphUsage::ColorWrite, true}}, ClearTo(out, 0, 0, 0));
        graph.ExportResource(out);
        Check(Throws([&] { graph.Compile(); }), "read before the first write is rejected");
    }
    {
        RenderGraph graph(device);
        auto        desc = Target();
        desc.usage       = VriTextureUsage_ShaderResource; // no colour attachment
        auto target      = graph.CreateTexture("Sample only", desc);
        graph.AddPass("Draws", {{target, GraphUsage::ColorWrite, true}}, ClearTo(target, 0, 0, 0));
        graph.ExportResource(target);
        Check(Throws([&] { graph.Compile(); }), "missing usage flags are rejected");
    }
    {
        RenderGraph graph(device);
        auto        target = graph.CreateTexture("Twice", Target());
        graph.AddPass("Twice", {{target, GraphUsage::ColorWrite, true}, {target, GraphUsage::Sampled}}, ClearTo(target, 0, 0, 0));
        graph.ExportResource(target);
        Check(Throws([&] { graph.Compile(); }), "a resource used twice in one pass is rejected");
    }
    {
        RenderGraph graph(device);
        auto        target = graph.CreateTexture("Target", Target());
        Check(Throws([&] { graph.AddPass("Read discard", {{target, GraphUsage::Sampled, true}}, ClearTo(target, 0, 0, 0)); graph.Compile(); }),
              "DiscardPrevious on a read is rejected");
    }

    std::printf("\n%d failure(s)\n", g_Failures);
    engine.Shutdown();
    return g_Failures;
}
CATCH
