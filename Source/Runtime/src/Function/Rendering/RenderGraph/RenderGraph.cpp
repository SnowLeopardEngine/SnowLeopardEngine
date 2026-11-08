#include "SnowLeopardEngine/Function/Rendering/RenderGraph/RenderGraph.h"

#include <algorithm>
#include <cstring>
#include <optional>
#include <stdexcept>

namespace SnowLeopardEngine
{
    namespace
    {
        void ValidateTexture(const VriTextureDesc& desc)
        {
            if (desc.layerNum != 1 || desc.mipNum != 1 || desc.type != VriTextureType_2D || desc.depth != 1)
            {
                throw std::invalid_argument("Graph textures are single-mip, single-layer 2D textures");
            }
            if ((desc.usage & VriTextureUsage_DepthStencilAttachment) && desc.format != VriFormat_D32_SFLOAT)
            {
                throw std::invalid_argument("Graph depth textures use D32_SFLOAT");
            }
            if (desc.format == VriFormat_D32_SFLOAT &&
                (desc.usage & (VriTextureUsage_ColorAttachment | VriTextureUsage_ShaderResourceStorage)))
            {
                throw std::invalid_argument("Depth textures cannot be colour or storage attachments");
            }
        }

        uint32_t RequiredUsage(GraphUsage usage, bool texture)
        {
            switch (usage)
            {
                case GraphUsage::CopySource:
                    return texture ? uint32_t(VriTextureUsage_TransferSrc) : uint32_t(VriBufferUsage_TransferSrc);
                case GraphUsage::CopyDestination:
                    return texture ? uint32_t(VriTextureUsage_TransferDst) : uint32_t(VriBufferUsage_TransferDst);
                case GraphUsage::StorageRead:
                case GraphUsage::StorageWrite:
                case GraphUsage::StorageReadWrite:
                    return texture ? uint32_t(VriTextureUsage_ShaderResourceStorage) :
                                     uint32_t(VriBufferUsage_StorageBuffer);
                case GraphUsage::ColorWrite:
                case GraphUsage::ColorReadWrite:
                    if (texture)
                    {
                        return VriTextureUsage_ColorAttachment;
                    }
                    break;
                case GraphUsage::DepthWrite:
                case GraphUsage::DepthRead:
                case GraphUsage::DepthReadWrite:
                    if (texture)
                    {
                        return VriTextureUsage_DepthStencilAttachment;
                    }
                    break;
                case GraphUsage::Sampled:
                    if (texture)
                    {
                        return VriTextureUsage_ShaderResource;
                    }
                    break;
                case GraphUsage::Present:
                    if (texture)
                    {
                        return 0;
                    }
                    break;
                case GraphUsage::IndirectRead:
                    if (!texture)
                    {
                        return VriBufferUsage_IndirectBuffer;
                    }
                    break;
            }
            throw std::invalid_argument("Unsupported graph usage for this resource type");
        }

        bool Reads(GraphUsage u)
        {
            return u != GraphUsage::ColorWrite && u != GraphUsage::DepthWrite && u != GraphUsage::StorageWrite &&
                   u != GraphUsage::CopyDestination;
        }

        bool Writes(GraphUsage u)
        {
            return u == GraphUsage::ColorWrite || u == GraphUsage::ColorReadWrite || u == GraphUsage::DepthWrite ||
                   u == GraphUsage::DepthReadWrite || u == GraphUsage::StorageWrite ||
                   u == GraphUsage::StorageReadWrite || u == GraphUsage::CopyDestination;
        }

        VriAccessLayoutStage StateForUsage(GraphUsage u)
        {
            switch (u)
            {
                case GraphUsage::ColorWrite:
                    return {VriAccess_ColorAttachmentWrite, VriLayout_ColorAttachment, VriPipelineStage_ColorAttachmentOutput};
                case GraphUsage::ColorReadWrite:
                    return {VriAccess_ColorAttachmentRead | VriAccess_ColorAttachmentWrite,
                            VriLayout_ColorAttachment,
                            VriPipelineStage_ColorAttachmentOutput};
                case GraphUsage::Sampled:
                    return {VriAccess_ShaderResourceRead, VriLayout_ShaderResource, VriPipelineStage_AllCommands};
                case GraphUsage::DepthWrite:
                    return {VriAccess_DepthStencilAttachmentWrite,
                            VriLayout_DepthStencilAttachment,
                            VriPipelineStage_EarlyFragmentTests | VriPipelineStage_LateFragmentTests};
                case GraphUsage::DepthRead:
                    // VRI stores a bound depth attachment even with depth writes off: keep read-only dataflow for
                    // culling, but synchronize the store.
                case GraphUsage::DepthReadWrite:
                    return {VriAccess_DepthStencilAttachmentRead | VriAccess_DepthStencilAttachmentWrite,
                            VriLayout_DepthStencilAttachment,
                            VriPipelineStage_EarlyFragmentTests | VriPipelineStage_LateFragmentTests};
                case GraphUsage::StorageRead:
                    return {VriAccess_ShaderResourceStorageRead, VriLayout_General, VriPipelineStage_AllCommands};
                case GraphUsage::StorageWrite:
                    return {VriAccess_ShaderResourceStorageWrite, VriLayout_General, VriPipelineStage_AllCommands};
                case GraphUsage::StorageReadWrite:
                    return {VriAccess_ShaderResourceStorageRead | VriAccess_ShaderResourceStorageWrite,
                            VriLayout_General,
                            VriPipelineStage_AllCommands};
                case GraphUsage::CopySource:
                    return {VriAccess_CopySourceRead, VriLayout_CopySource, VriPipelineStage_Transfer};
                case GraphUsage::CopyDestination:
                    return {VriAccess_CopyDestinationWrite, VriLayout_CopyDestination, VriPipelineStage_Transfer};
                case GraphUsage::Present:
                    return {VriAccess_None, VriLayout_Present, VriPipelineStage_AllCommands};
                case GraphUsage::IndirectRead:
                    return {VriAccess_IndirectBufferRead, VriLayout_Undefined, VriPipelineStage_DrawIndirect};
            }
            throw std::logic_error("Unknown graph usage");
        }

        bool SameTexture(const VriTextureDesc& a, const VriTextureDesc& b)
        {
            // Field by field (not struct padding); the clear hint belongs to the physical texture too.
            return a.type == b.type && a.format == b.format && a.width == b.width && a.height == b.height &&
                   a.depth == b.depth && a.mipNum == b.mipNum && a.layerNum == b.layerNum &&
                   a.sampleNum == b.sampleNum && a.usage == b.usage && a.memoryLocation == b.memoryLocation &&
                   std::memcmp(&a.clearValue, &b.clearValue, sizeof(VriClearValue)) == 0;
        }

        bool SameBuffer(const VriBufferDesc& a, const VriBufferDesc& b)
        {
            return a.size == b.size && a.structureStride == b.structureStride && a.usage == b.usage &&
                   a.memoryLocation == b.memoryLocation;
        }
    } // namespace

    const char* GraphUsageName(GraphUsage usage)
    {
        switch (usage)
        {
            case GraphUsage::ColorWrite:
                return "color write";
            case GraphUsage::ColorReadWrite:
                return "color read/write";
            case GraphUsage::DepthWrite:
                return "depth write";
            case GraphUsage::DepthRead:
                return "depth read";
            case GraphUsage::DepthReadWrite:
                return "depth read/write";
            case GraphUsage::Sampled:
                return "sampled";
            case GraphUsage::StorageRead:
                return "storage read";
            case GraphUsage::StorageWrite:
                return "storage write";
            case GraphUsage::StorageReadWrite:
                return "storage read/write";
            case GraphUsage::CopySource:
                return "copy source";
            case GraphUsage::CopyDestination:
                return "copy destination";
            case GraphUsage::Present:
                return "present";
            case GraphUsage::IndirectRead:
                return "indirect read";
        }
        throw std::logic_error("Unknown graph usage");
    }

    void RenderGraph::Edit() const
    {
        if (m_Aliased)
        {
            throw std::logic_error("Rebuild the graph to change an aliased resource plan");
        }
    }

    RenderGraph::Entry& RenderGraph::Lookup(Resource resource)
    {
        if (resource.Graph != this || resource.Index >= m_Resources.size())
        {
            throw std::invalid_argument("Resource belongs to another graph");
        }
        return m_Resources[resource.Index];
    }

    const RenderGraph::Entry& RenderGraph::Lookup(Resource resource) const
    {
        if (resource.Graph != this || resource.Index >= m_Resources.size())
        {
            throw std::invalid_argument("Resource belongs to another graph");
        }
        return m_Resources[resource.Index];
    }

    bool RenderGraph::IsExported(size_t index) const
    {
        return std::any_of(m_Exports.begin(), m_Exports.end(), [index](Resource r) { return r.Index == index; });
    }

    RenderGraph::Resource RenderGraph::ImportResource(std::string name, RHI::Texture& texture, bool initialized)
    {
        Edit();
        ValidateTexture(texture.Desc);
        for (const auto& entry : m_Resources)
        {
            if (entry.Texture != nullptr && entry.Texture->Handle == texture.Handle)
            {
                throw std::invalid_argument("Import each texture only once");
            }
        }
        Entry entry;
        entry.Name        = std::move(name);
        entry.Imported    = true;
        entry.Initialized = initialized;
        entry.Texture     = &texture;
        entry.TextureDesc = texture.Desc;
        m_Device.Core.SetDebugName(texture.Handle, entry.Name.c_str());
        m_Resources.push_back(std::move(entry));
        m_Compiled = false;
        return {this, uint32_t(m_Resources.size() - 1)};
    }

    RenderGraph::Resource RenderGraph::ImportResource(std::string name, RHI::Buffer& buffer, bool initialized)
    {
        Edit();
        for (const auto& entry : m_Resources)
        {
            if (entry.Buffer != nullptr && entry.Buffer->Handle == buffer.Handle)
            {
                throw std::invalid_argument("Import each buffer only once");
            }
        }
        Entry entry;
        entry.Name        = std::move(name);
        entry.IsTexture   = false;
        entry.Imported    = true;
        entry.Initialized = initialized;
        entry.Buffer      = &buffer;
        entry.BufferDesc  = buffer.Desc;
        m_Device.Core.SetDebugName(buffer.Handle, entry.Name.c_str());
        m_Resources.push_back(std::move(entry));
        m_Compiled = false;
        return {this, uint32_t(m_Resources.size() - 1)};
    }

    RenderGraph::Resource RenderGraph::CreateTexture(std::string name, const VriTextureDesc& desc)
    {
        Edit();
        ValidateTexture(desc);
        Entry entry;
        entry.Name        = std::move(name);
        entry.TextureDesc = desc;
        m_Resources.push_back(std::move(entry));
        m_Compiled = false;
        return {this, uint32_t(m_Resources.size() - 1)};
    }

    RenderGraph::Resource RenderGraph::CreateBuffer(std::string name, const VriBufferDesc& desc)
    {
        Edit();
        Entry entry;
        entry.Name       = std::move(name);
        entry.IsTexture  = false;
        entry.BufferDesc = desc;
        m_Resources.push_back(std::move(entry));
        m_Compiled = false;
        return {this, uint32_t(m_Resources.size() - 1)};
    }

    RenderGraph::Resource RenderGraph::CreateHistoryTexture(std::string name, const VriTextureDesc& desc, VriClearColor initial)
    {
        if (!(desc.usage & VriTextureUsage_TransferDst) || !(desc.usage & VriTextureUsage_ShaderResourceStorage) ||
            desc.sampleNum != 1 || !m_Device.Core.GetDeviceDesc(m_Device.Handle)->hasClearStorageTexture)
        {
            throw std::invalid_argument("History textures need storage + transfer-destination usage and VRI clear support");
        }
        const auto resource = CreateTexture(std::move(name), desc);
        auto&      entry    = Lookup(resource);
        entry.History       = true;
        entry.Initialized   = true;
        entry.Initial       = initial;
        m_ResetHistory      = true;
        return resource;
    }

    void RenderGraph::ResetHistory()
    {
        if (m_HistoryEpoch == UINT64_MAX)
        {
            throw std::overflow_error("Graph history epoch exhausted");
        }
        ++m_HistoryEpoch;
        m_ResetHistory = true;
    }

    void RenderGraph::AddPass(std::string name, std::initializer_list<Use> uses, ExecutePass execute, bool sideEffect)
    {
        AddPass(std::move(name), std::span(uses.begin(), uses.size()), std::move(execute), sideEffect);
    }

    void RenderGraph::AddPass(std::string name, std::span<const Use> uses, ExecutePass execute, bool sideEffect)
    {
        Edit();
        if (!execute)
        {
            throw std::invalid_argument("Pass needs an execute function");
        }
        for (const auto& use : uses)
        {
            Lookup(use.Target);
        }
        Pass pass;
        pass.Name       = std::move(name);
        pass.Uses       = {uses.begin(), uses.end()};
        pass.Execute    = std::move(execute);
        pass.SideEffect = sideEffect;
        m_Passes.push_back(std::move(pass));
        m_Compiled = false;
    }

    void RenderGraph::NameResource(Resource resource, std::string name)
    {
        Edit();
        if (name.empty())
        {
            throw std::invalid_argument("Render graph resource name is empty");
        }
        Lookup(resource).Name = std::move(name);
    }

    void RenderGraph::ExportResource(Resource resource)
    {
        Edit();
        Lookup(resource);
        m_Exports.push_back(resource);
        m_Compiled = false;
    }

    RenderGraph::Resource RenderGraph::FindResource(std::string_view name) const
    {
        uint32_t found = UINT32_MAX;
        for (uint32_t i = 0; i < m_Resources.size(); ++i)
        {
            if (m_Resources[i].Name == name)
            {
                if (found != UINT32_MAX)
                {
                    throw std::invalid_argument("Ambiguous graph resource: " + std::string(name));
                }
                found = i;
            }
        }
        if (found == UINT32_MAX)
        {
            throw std::invalid_argument("Unknown graph resource: " + std::string(name));
        }
        return {this, found};
    }

    RenderGraph::Resource RenderGraph::CaptureAfterPass(const std::string& passName, Resource source, std::string name)
    {
        Edit();
        const auto& entry = Lookup(source);
        if (!entry.IsTexture || !(entry.TextureDesc.usage & VriTextureUsage_TransferSrc))
        {
            throw std::invalid_argument("Capture source must be a TransferSrc texture");
        }
        const auto writesSource = [source](const Pass& pass) {
            return std::any_of(pass.Uses.begin(), pass.Uses.end(), [source](const Use& use) {
                return use.Target.Index == source.Index && Writes(use.Usage);
            });
        };
        auto pass = m_Passes.end();
        if (passName.empty())
        {
            for (auto candidate = m_Passes.begin(); candidate != m_Passes.end(); ++candidate)
            {
                if (writesSource(*candidate))
                {
                    pass = candidate; // the last writer
                }
            }
            if (pass == m_Passes.end())
            {
                throw std::invalid_argument("Captured texture has no writer: " + entry.Name);
            }
        }
        else
        {
            const auto matches = [&](const Pass& candidate) { return candidate.Name == passName; };
            if (std::count_if(m_Passes.begin(), m_Passes.end(), matches) != 1)
            {
                throw std::invalid_argument("Capture requires exactly one pass named " + passName);
            }
            pass = std::find_if(m_Passes.begin(), m_Passes.end(), matches);
        }
        if (!writesSource(*pass))
        {
            throw std::invalid_argument(pass->Name + " does not write the captured texture");
        }
        const auto writer     = pass->Name;
        const auto passOffset = pass - m_Passes.begin();

        auto desc = entry.TextureDesc;
        desc.usage |= VriTextureUsage_TransferDst;
        const auto captured = CreateTexture(std::move(name), desc);

        Pass copy;
        copy.Name    = "Capture after " + writer;
        copy.Uses    = {{source, GraphUsage::CopySource}, {captured, GraphUsage::CopyDestination, true}};
        copy.Execute = [source, captured](VriCommandBuffer* cmd, RenderGraph& graph) {
            VriTextureCopyDesc region {};
            region.src.layerNum = 1;
            region.dst.layerNum = 1;
            region.src.aspect   = graph.GetTexture(source).Desc.format == VriFormat_D32_SFLOAT ? VriImageAspect_Depth :
                                                                                                 VriImageAspect_Color;
            region.dst.aspect   = region.src.aspect;
            graph.GetDevice().Core.CmdCopyTexture(
                cmd, graph.GetTexture(captured).Handle, graph.GetTexture(source).Handle, &region);
        };
        m_Passes.insert(m_Passes.begin() + passOffset + 1, std::move(copy));
        ExportResource(captured);
        return captured;
    }

    void RenderGraph::Compile(bool aliasTransients)
    {
        if (m_Aliased)
        {
            if (!aliasTransients)
            {
                throw std::logic_error("An aliased graph must keep its allocation plan");
            }
            return;
        }
        if (aliasTransients && std::any_of(m_Resources.begin(), m_Resources.end(), [](const Entry& entry) {
                return entry.OwnedTexture || entry.OwnedBuffer;
            }))
        {
            throw std::logic_error("Choose transient aliasing before the first compilation");
        }
        m_Compiled = false;

        // Dependencies: a pass depends on the last writer of each resource it uses, and a write additionally
        // waits for the readers since that writer. Content dependencies skip writes that discard the old value.
        std::vector<std::optional<size_t>> writer(m_Resources.size());
        std::vector<std::vector<size_t>>   readers(m_Resources.size());
        for (size_t p = 0; p < m_Passes.size(); ++p)
        {
            auto& pass = m_Passes[p];
            pass.Dependencies.clear();
            pass.ContentDependencies.clear();
            pass.Live = false;
            std::vector<bool> used(m_Resources.size());
            for (const auto& use : pass.Uses)
            {
                auto&      entry = Lookup(use.Target);
                const auto r     = use.Target.Index;
                if (used[r])
                {
                    throw std::invalid_argument(pass.Name + ": resource used twice; use a ReadWrite usage");
                }
                used[r] = true;
                if (use.DiscardPrevious && (!Writes(use.Usage) || Reads(use.Usage)))
                {
                    throw std::invalid_argument(pass.Name + ": DiscardPrevious needs a write-only usage: " + entry.Name);
                }
                const auto required = RequiredUsage(use.Usage, entry.IsTexture);
                const auto declared = entry.IsTexture ? entry.TextureDesc.usage : entry.BufferDesc.usage;
                if ((declared & required) != required)
                {
                    throw std::invalid_argument(pass.Name + ": resource lacks the usage flags for " +
                                                GraphUsageName(use.Usage) + ": " + entry.Name);
                }
                if (Reads(use.Usage) && !writer[r] && !entry.Initialized)
                {
                    throw std::invalid_argument(pass.Name + ": read before the first write: " + entry.Name);
                }
                if (writer[r])
                {
                    pass.Dependencies.push_back(*writer[r]);
                    if (Reads(use.Usage) || !use.DiscardPrevious)
                    {
                        pass.ContentDependencies.push_back(*writer[r]);
                    }
                }
                if (Writes(use.Usage))
                {
                    pass.Dependencies.insert(pass.Dependencies.end(), readers[r].begin(), readers[r].end());
                    readers[r].clear();
                    writer[r] = p;
                }
                else
                {
                    readers[r].push_back(p);
                }
            }
        }

        // Culling: live passes are those reachable (through content dependencies) from side-effect passes, the
        // last writer of each history texture and the producers of exports.
        std::vector<size_t> pending;
        for (size_t p = 0; p < m_Passes.size(); ++p)
        {
            if (m_Passes[p].SideEffect)
            {
                pending.push_back(p);
            }
        }
        for (size_t r = 0; r < m_Resources.size(); ++r)
        {
            // Only the final history value is observed next frame.
            if (m_Resources[r].History && writer[r])
            {
                pending.push_back(*writer[r]);
            }
        }
        for (const auto resource : m_Exports)
        {
            if (writer[resource.Index])
            {
                pending.push_back(*writer[resource.Index]);
            }
            else if (!Lookup(resource).Initialized)
            {
                throw std::invalid_argument("Exported resource has no producer: " + Lookup(resource).Name);
            }
        }
        while (!pending.empty())
        {
            auto& pass = m_Passes[pending.back()];
            pending.pop_back();
            if (pass.Live)
            {
                continue;
            }
            pass.Live = true;
            pending.insert(pending.end(), pass.ContentDependencies.begin(), pass.ContentDependencies.end());
        }

        // Lifetimes over live passes (declaration order is the execution order).
        for (auto& entry : m_Resources)
        {
            entry.FirstUse = UINT32_MAX;
            entry.LastUse  = UINT32_MAX;
        }
        for (size_t p = 0; p < m_Passes.size(); ++p)
        {
            if (m_Passes[p].Live)
            {
                for (const auto& use : m_Passes[p].Uses)
                {
                    auto& entry    = Lookup(use.Target);
                    entry.FirstUse = std::min(entry.FirstUse, uint32_t(p));
                    entry.LastUse  = uint32_t(p);
                }
            }
        }
        std::vector<uint32_t> order;
        for (uint32_t i = 0; i < m_Resources.size(); ++i)
        {
            // An exported, read-only import / history is live even without a pass.
            if (m_Resources[i].FirstUse != UINT32_MAX || IsExported(i))
            {
                order.push_back(i);
            }
        }
        std::stable_sort(order.begin(), order.end(), [this](uint32_t a, uint32_t b) {
            return m_Resources[a].FirstUse < m_Resources[b].FirstUse;
        });

        // Allocation. Imports, exports and history textures never share memory.
        std::vector<uint32_t> allocations;
        std::vector<uint32_t> ends(m_Resources.size(), UINT32_MAX);
        for (const auto index : order)
        {
            auto& entry      = m_Resources[index];
            entry.Allocation = index;
            if (entry.Imported)
            {
                continue;
            }
            if (aliasTransients && !entry.History && !IsExported(index))
            {
                for (const auto owner : allocations)
                {
                    const auto& candidate = m_Resources[owner];
                    if (ends[owner] >= entry.FirstUse || candidate.History || IsExported(owner) ||
                        candidate.IsTexture != entry.IsTexture)
                    {
                        continue;
                    }
                    const bool same = entry.IsTexture ? SameTexture(candidate.TextureDesc, entry.TextureDesc) :
                                                        SameBuffer(candidate.BufferDesc, entry.BufferDesc);
                    if (same)
                    {
                        entry.Texture    = candidate.Texture;
                        entry.Buffer     = candidate.Buffer;
                        entry.Allocation = owner;
                        ends[owner]      = entry.LastUse;
                        break;
                    }
                }
            }
            if (entry.Texture == nullptr && entry.Buffer == nullptr)
            {
                if (entry.IsTexture)
                {
                    const auto aspect =
                        entry.TextureDesc.format == VriFormat_D32_SFLOAT ? VriImageAspect_Depth : VriImageAspect_Color;
                    entry.OwnedTexture = CreateScope<RHI::Texture>(m_Device, entry.TextureDesc, nullptr, aspect);
                    entry.Texture      = entry.OwnedTexture.get();
                    m_Device.Core.SetDebugName(entry.Texture->Handle, entry.Name.c_str());
                }
                else
                {
                    entry.OwnedBuffer = CreateScope<RHI::Buffer>(m_Device, entry.BufferDesc);
                    entry.Buffer      = entry.OwnedBuffer.get();
                    m_Device.Core.SetDebugName(entry.Buffer->Handle, entry.Name.c_str());
                }
            }
            if (entry.Allocation == index)
            {
                allocations.push_back(index);
                ends[index] = entry.LastUse;
            }
        }
        m_Aliased  = aliasTransients;
        m_Compiled = true;
    }

    void RenderGraph::Execute(VriCommandBuffer* cmd, RHI::GpuProfiler* profiler)
    {
        if (!m_Compiled)
        {
            throw std::logic_error("Compile the graph before Execute");
        }
        auto& core = m_Device.Core;
        if (m_ResetHistory)
        {
            core.CmdBeginDebugGroup(cmd, "Reset graph history");
            for (auto& entry : m_Resources)
            {
                if (entry.History && entry.Texture != nullptr)
                {
                    entry.Texture->Transition(cmd, StateForUsage(GraphUsage::CopyDestination));
                    core.CmdClearStorageTexture(cmd, entry.Texture->Handle, &entry.Initial);
                }
            }
            core.CmdEndDebugGroup(cmd);
            m_ResetHistory = false;
        }
        for (const auto& pass : m_Passes)
        {
            if (!pass.Live)
            {
                continue;
            }
            core.CmdBeginDebugGroup(cmd, pass.Name.c_str());
            if (profiler != nullptr)
            {
                profiler->BeginPass(cmd, pass.Name);
            }
            for (const auto& use : pass.Uses)
            {
                auto&      entry = Lookup(use.Target);
                const auto state = StateForUsage(use.Usage);
                if (entry.IsTexture)
                {
                    entry.Texture->Transition(cmd, state);
                }
                else
                {
                    entry.Buffer->Transition(cmd, {state.access, state.stages});
                }
            }
            if (profiler != nullptr)
            {
                profiler->BeginCommands(cmd);
            }
            pass.Execute(cmd, *this);
            if (profiler != nullptr)
            {
                profiler->EndPass(cmd);
            }
            core.CmdEndDebugGroup(cmd);
        }
    }

    void RenderGraph::Bind(Resource resource, RHI::Texture& texture)
    {
        ValidateTexture(texture.Desc);
        auto& entry = Lookup(resource);
        if (!entry.Imported || !entry.IsTexture || texture.Desc.width != entry.TextureDesc.width ||
            texture.Desc.height != entry.TextureDesc.height || texture.Desc.format != entry.TextureDesc.format ||
            texture.Desc.sampleNum != entry.TextureDesc.sampleNum || texture.Desc.usage != entry.TextureDesc.usage)
        {
            throw std::invalid_argument("Rebinding needs an imported texture of the same format and extent");
        }
        for (const auto& other : m_Resources)
        {
            if (&other != &entry && other.Texture != nullptr && other.Texture->Handle == texture.Handle)
            {
                throw std::invalid_argument("Rebinding would alias another graph texture");
            }
        }
        entry.Texture = &texture;
        m_Device.Core.SetDebugName(texture.Handle, entry.Name.c_str());
    }

    RHI::Texture& RenderGraph::GetTexture(Resource resource)
    {
        auto& entry = Lookup(resource);
        if (!entry.IsTexture || entry.Texture == nullptr)
        {
            throw std::logic_error("Texture is not allocated (compile the graph, or export it): " + entry.Name);
        }
        return *entry.Texture;
    }

    RHI::Buffer& RenderGraph::GetBuffer(Resource resource)
    {
        auto& entry = Lookup(resource);
        if (entry.IsTexture || entry.Buffer == nullptr)
        {
            throw std::logic_error("Buffer is not allocated (compile the graph, or export it): " + entry.Name);
        }
        return *entry.Buffer;
    }

    RenderGraph::Snapshot RenderGraph::GetSnapshot() const
    {
        if (!m_Compiled)
        {
            throw std::logic_error("Compile the graph before inspecting it");
        }
        Snapshot result;
        result.Resources.reserve(m_Resources.size());
        result.Passes.reserve(m_Passes.size());
        for (size_t i = 0; i < m_Resources.size(); ++i)
        {
            const auto& entry    = m_Resources[i];
            const bool  exported = IsExported(i);
            result.Resources.push_back({entry.Name,
                                        entry.Imported,
                                        exported,
                                        exported,
                                        entry.IsTexture,
                                        entry.TextureDesc,
                                        entry.BufferDesc,
                                        entry.History,
                                        entry.FirstUse,
                                        entry.LastUse,
                                        entry.Allocation});
        }
        for (const auto& pass : m_Passes)
        {
            PassInfo info {pass.Name, pass.Live, pass.Dependencies, {}, pass.ContentDependencies};
            for (const auto& use : pass.Uses)
            {
                info.Uses.push_back({use.Target.Index, use.Usage, use.DiscardPrevious});
                if (pass.Live)
                {
                    result.Resources[use.Target.Index].Active = true;
                }
            }
            result.Passes.push_back(std::move(info));
        }

        // Memory per resource, where the backend reports it.
        auto&      core        = m_Device.Core;
        uint32_t   objectCount = 0;
        const auto enumerated  = core.EnumerateObjects != nullptr ?
                                     core.EnumerateObjects(m_Device.Handle, &objectCount, nullptr) :
                                     VriResult_Unsupported;
        if (enumerated != VriResult_Unsupported)
        {
            RHI::Check(enumerated, "Count graph device allocations");
            std::vector<VriObjectInfo> objects(objectCount);
            if (objectCount != 0)
            {
                RHI::Check(core.EnumerateObjects(m_Device.Handle, &objectCount, objects.data()),
                           "Inspect graph device allocations");
                objects.resize(objectCount);
            }
            for (size_t i = 0; i < m_Resources.size(); ++i)
            {
                const auto& entry  = m_Resources[i];
                const void* handle = nullptr;
                if (entry.IsTexture && entry.Texture != nullptr)
                {
                    handle = entry.Texture->Handle;
                }
                else if (!entry.IsTexture && entry.Buffer != nullptr)
                {
                    handle = entry.Buffer->Handle;
                }
                const auto object = std::find_if(
                    objects.begin(), objects.end(), [handle](const VriObjectInfo& info) { return info.handle == handle; });
                if (handle != nullptr && object != objects.end())
                {
                    result.Resources[i].MemoryBytes = object->memoryBytes;
                    result.Resources[i].MemoryKnown = true;
                }
            }
        }
        return result;
    }

    RenderGraph::ResourceInfo RenderGraph::GetResourceInfo(Resource resource) const
    {
        const auto& entry    = Lookup(resource);
        const bool  exported = IsExported(resource.Index);
        bool        active   = m_Compiled && exported;
        if (m_Compiled && !active)
        {
            for (const auto& pass : m_Passes)
            {
                if (pass.Live && std::any_of(pass.Uses.begin(), pass.Uses.end(), [resource](const Use& use) {
                        return use.Target.Index == resource.Index;
                    }))
                {
                    active = true;
                    break;
                }
            }
        }
        return {entry.Name,
                entry.Imported,
                exported,
                active,
                entry.IsTexture,
                entry.TextureDesc,
                entry.BufferDesc,
                entry.History,
                entry.FirstUse,
                entry.LastUse,
                entry.Allocation};
    }

    std::vector<std::string> RenderGraph::GetActivePasses() const
    {
        std::vector<std::string> result;
        for (const auto& pass : m_Passes)
        {
            if (pass.Live)
            {
                result.push_back(pass.Name);
            }
        }
        return result;
    }
} // namespace SnowLeopardEngine
