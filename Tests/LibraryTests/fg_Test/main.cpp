#include "fg/Blackboard.hpp"
#include "fg/FrameGraph.hpp"
#include "fg/FrameGraphResource.hpp"

#include <fstream>
#include <iostream>
#include <span>
#include <vector>

class FrameGraphTexture
{
public:
    struct Desc
    {
        uint32_t Width;
        uint32_t Height;
    };

    void create(const Desc& desc, void* allocator) { std::cout << "FrameGraphTexture Created!" << std::endl; }
    void destroy(const Desc& desc, void* allocator) { std::cout << "FrameGraphTexture Destroyed!" << std::endl; }
};

class RenderContext
{
public:
    RenderContext& bindTextures(const std::vector<FrameGraphTexture>& textures)
    {
        std::cout << "RenderContext bind textures!" << std::endl;
        return *this;
    }

    RenderContext& drawFullScreenQuad()
    {
        std::cout << "RenderContext draw full screen quad!" << std::endl;
        return *this;
    }
};

class Mesh
{};

class Renderable
{
public:
    const Mesh& mesh;

    explicit Renderable(const Mesh& mesh) : mesh(mesh) {}

    void draw(const RenderContext& rc) const { std::cout << "Draw mesh!" << std::endl; }
};

struct GBufferData
{
    FrameGraphResource depth;
    FrameGraphResource normal;
    FrameGraphResource albedo;
};

class GBufferPass
{
public:
    void addToFrameGraph(FrameGraph& fg, FrameGraphBlackboard& blackboard, std::span<const Renderable> renderables)
    {
        blackboard.add<GBufferData>() = fg.addCallbackPass<GBufferData>(
            "GBuffer Pass",
            [&](FrameGraph::Builder& builder, GBufferData& data) {
                data.depth = builder.create<FrameGraphTexture>("SceneDepth", {/* extent, pixelFormat ... */});
                data.depth = builder.write(data.depth);

                data.normal = builder.create<FrameGraphTexture>("Normal", {});
                data.normal = builder.write(data.normal);

                data.albedo = builder.create<FrameGraphTexture>("Albedo", {});
                data.albedo = builder.write(data.albedo);
            },
            [=](const GBufferData& data, FrameGraphPassResources& resources, void* ctx) {
                auto& rc = *static_cast<RenderContext*>(ctx);
                for (const auto& renderable : renderables)
                    renderable.draw(rc);
            });
    }
};

struct LightingData
{
    FrameGraphResource out;
};

class DeferredLightingPass
{
public:
    void addToFrameGraph(FrameGraph& fg, FrameGraphBlackboard& blackboard, FrameGraphResource backbufferId)
    {
        const auto& gBuffer = blackboard.get<GBufferData>();

        blackboard.add<LightingData>() = fg.addCallbackPass<LightingData>(
            "Deferred Lighting Pass",
            [&](FrameGraph::Builder& builder, LightingData& data) {
                builder.read(gBuffer.depth);
                builder.read(gBuffer.normal);
                builder.read(gBuffer.albedo);
                data.out = builder.write(backbufferId);
            },
            [=](const LightingData& data, FrameGraphPassResources& resources, void* ctx) {
                auto& rc = *static_cast<RenderContext*>(ctx);
                rc.bindTextures({
                                    resources.get<FrameGraphTexture>(gBuffer.depth),
                                    resources.get<FrameGraphTexture>(gBuffer.normal),
                                    resources.get<FrameGraphTexture>(gBuffer.albedo),
                                })
                    .drawFullScreenQuad();
            });
    }
};

void renderFrame(RenderContext* rc, std::span<const Renderable> renderables)
{
    FrameGraph           fg;
    FrameGraphBlackboard blackboard;

    auto backbufferId = fg.import("Backbuffer", {1280, 720}, FrameGraphTexture {});

    GBufferPass gBufferPass;
    gBufferPass.addToFrameGraph(fg, blackboard, renderables);

    DeferredLightingPass deferredLightingPass;
    deferredLightingPass.addToFrameGraph(fg, blackboard, backbufferId);

    fg.compile();
    fg.execute(rc);

    // Built in graphviz writer.
    std::ofstream {"fg.dot"} << fg;
}

int main()
{
    auto* rc = new RenderContext();

    std::vector<Renderable> renderables;

    Mesh       mesh0;
    Renderable r0(mesh0);
    renderables.emplace_back(r0);

    Mesh       mesh1;
    Renderable r1(mesh1);
    renderables.emplace_back(r1);

    Mesh       mesh2;
    Renderable r2(mesh2);
    renderables.emplace_back(r2);

    renderFrame(rc, renderables);

    delete rc;

    return 0;
}