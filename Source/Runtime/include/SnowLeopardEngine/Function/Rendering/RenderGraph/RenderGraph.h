#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace SnowLeopardEngine
{
    // How a pass uses a graph resource. The graph derives every barrier from these.
    enum class GraphUsage
    {
        ColorWrite,
        ColorReadWrite,
        DepthWrite,
        DepthRead, // keeps depth values; the attachment store still needs write synchronization
        DepthReadWrite,
        Sampled,
        StorageRead,
        StorageWrite,
        StorageReadWrite,
        CopySource,
        CopyDestination,
        Present
    };

    const char* GraphUsageName(GraphUsage usage);

    // Render graph ported from libvultra (MIT): passes declare the resources they use, Compile culls passes that
    // contribute nothing to an export / side effect / history and allocates transient resources (optionally
    // aliasing those with disjoint lifetimes), Execute records each live pass after the barriers its uses need.
    //
    // Passes run in declaration order, which must be a valid order (a pass may only read what an earlier pass wrote
    // or what was imported initialized). Graph textures are single-mip, single-layer 2D textures.
    class RenderGraph
    {
    public:
        struct Resource
        {
            const RenderGraph* Graph = nullptr;
            uint32_t           Index = UINT32_MAX;
        };

        struct Use
        {
            Resource   Target;
            GraphUsage Usage;
            // The write replaces the whole resource: the pass must not load or keep its old content.
            bool DiscardPrevious = false;
        };

        struct ResourceInfo
        {
            std::string    Name;
            bool           Imported  = false;
            bool           Exported  = false;
            bool           Active    = false;
            bool           IsTexture = true;
            VriTextureDesc TextureDesc {};
            VriBufferDesc  BufferDesc {};
            bool           History     = false;
            uint32_t       FirstUse    = UINT32_MAX;
            uint32_t       LastUse     = UINT32_MAX;
            uint32_t       Allocation  = UINT32_MAX; // equal values: the same physical resource
            uint64_t       MemoryBytes = 0;          // VRI allocator measurement, including alignment / tiling
            bool           MemoryKnown = false;
        };

        struct PassUseInfo
        {
            uint32_t   ResourceIndex;
            GraphUsage Usage;
            bool       DiscardPrevious = false;
        };

        struct PassInfo
        {
            std::string              Name;
            bool                     Active = false;
            std::vector<size_t>      Dependencies;
            std::vector<PassUseInfo> Uses;
            // Producers whose data this pass needs (ordering-only write-after-read / write edges excluded).
            std::vector<size_t> ContentDependencies;
        };

        struct Snapshot
        {
            std::vector<ResourceInfo> Resources;
            std::vector<PassInfo>     Passes;
        };

        using ExecutePass = std::function<void(VriCommandBuffer*, RenderGraph&)>;

        explicit RenderGraph(RHI::Device& device) : m_Device(device) {}

        RenderGraph(const RenderGraph&)            = delete;
        RenderGraph& operator=(const RenderGraph&) = delete;

        Resource ImportResource(std::string name, RHI::Texture& texture, bool initialized = true);
        Resource ImportResource(std::string name, RHI::Buffer& buffer, bool initialized = true);
        Resource CreateTexture(std::string name, const VriTextureDesc& desc);
        Resource CreateBuffer(std::string name, const VriBufferDesc& desc);
        // Owned by the graph and kept across frames; restored to `initial` before its first use after a reset.
        Resource CreateHistoryTexture(std::string name, const VriTextureDesc& desc, VriClearColor initial = {});
        void     ResetHistory(); // between completed frames; allocations and descriptors stay
        uint64_t GetHistoryEpoch() const { return m_HistoryEpoch; }

        void AddPass(std::string name, std::initializer_list<Use> uses, ExecutePass execute, bool sideEffect = false);
        void AddPass(std::string name, std::span<const Use> uses, ExecutePass execute, bool sideEffect = false);

        void     ExportResource(Resource resource);
        void     NameResource(Resource resource, std::string name);
        // Inserts (and exports) a copy of `source` right after the pass that writes it, before later writes.
        Resource CaptureAfterPass(const std::string& passName, Resource source, std::string name);
        Resource FindResource(std::string_view name) const;

        // Aliasing shares a physical resource between identical transients with disjoint lifetimes; an aliased
        // plan cannot be edited any more.
        void Compile(bool aliasTransients = false);
        bool AliasesTransients() const { return m_Aliased; }
        void Execute(VriCommandBuffer* cmd);

        // Rebinds an imported texture of the same format and extent (e.g. this frame's swapchain image).
        void Bind(Resource resource, RHI::Texture& texture);

        RHI::Texture&            GetTexture(Resource resource);
        RHI::Buffer&             GetBuffer(Resource resource);
        RHI::Device&             GetDevice() const { return m_Device; }
        ResourceInfo             GetResourceInfo(Resource resource) const;
        std::vector<std::string> GetActivePasses() const;
        Snapshot                 GetSnapshot() const; // the compiled plan; no GPU readback

    private:
        struct Entry
        {
            std::string         Name;
            bool                IsTexture   = true;
            bool                Imported    = false;
            bool                Initialized = false;
            VriTextureDesc      TextureDesc {};
            VriBufferDesc       BufferDesc {};
            RHI::Texture*       Texture = nullptr;
            RHI::Buffer*        Buffer  = nullptr;
            Scope<RHI::Texture> OwnedTexture;
            Scope<RHI::Buffer>  OwnedBuffer;
            bool                History = false;
            VriClearColor       Initial {};
            uint32_t            FirstUse   = UINT32_MAX;
            uint32_t            LastUse    = UINT32_MAX;
            uint32_t            Allocation = UINT32_MAX;
        };

        struct Pass
        {
            std::string         Name;
            std::vector<Use>    Uses;
            ExecutePass         Execute;
            bool                SideEffect = false;
            bool                Live       = false;
            std::vector<size_t> Dependencies;
            std::vector<size_t> ContentDependencies;
        };

        Entry&       Lookup(Resource resource);
        const Entry& Lookup(Resource resource) const;
        void         Edit() const;
        bool         IsExported(size_t index) const;

        RHI::Device&          m_Device;
        std::vector<Entry>    m_Resources;
        std::vector<Pass>     m_Passes;
        std::vector<Resource> m_Exports;
        bool                  m_Compiled     = false;
        bool                  m_Aliased      = false;
        bool                  m_ResetHistory = true;
        uint64_t              m_HistoryEpoch = 1;
    };
} // namespace SnowLeopardEngine
