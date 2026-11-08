#include "SnowLeopardEngine/Function/Rendering/RHI/GpuProfiler.h"

#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>
#include <client/TracyProfiler.hpp>

#include <cstring>
#include <stdexcept>

namespace SnowLeopardEngine::RHI
{
    GpuProfiler::GpuProfiler(Device& device, uint32_t eventCapacity) : m_Device(device), m_EventCapacity(eventCapacity)
    {
        m_Records.reserve(eventCapacity);
        const auto* desc = device.Core.GetDeviceDesc(device.Handle);
        m_TickNs         = desc->timestampPeriodNanoseconds;
        if (!desc->hasTimestampQueries || m_TickNs <= 0 ||
            vriGetInterface(device.Handle, VRI_INTERFACE_QUERY, sizeof(m_Api), &m_Api) != VriResult_Success)
        {
            return; // CPU timings only
        }
        m_Readback = CreateScope<Buffer>(
            device, VriBufferDesc {uint64_t(eventCapacity) * 3 * sizeof(uint64_t), 0, VriBufferUsage_TransferDst, VriMemoryLocation_HostReadback});
        VriQueryPoolDesc query {VriQueryType_Timestamp, eventCapacity * 3};
        Check(m_Api.CreateQueryPool(device.Handle, &query, &m_Pool), "Create timestamp query pool");
    }

    GpuProfiler::~GpuProfiler()
    {
        m_Device.WaitIdle();
        if (m_Pool != nullptr)
        {
            m_Api.DestroyQueryPool(m_Pool);
        }
    }

    void GpuProfiler::BeginFrame(VriCommandBuffer* cmd)
    {
        m_Records.clear();
        m_QueryCount = 0;
        m_Open       = false;
        if (m_Pool != nullptr)
        {
            m_Api.CmdResetQueries(cmd, m_Pool, 0, m_EventCapacity * 3);
        }
    }

    void GpuProfiler::BeginPass(VriCommandBuffer* cmd, const std::string& name)
    {
        if (m_Open)
        {
            throw std::logic_error("GpuProfiler: passes do not nest");
        }
        if (m_Records.size() == m_EventCapacity)
        {
            return; // over capacity: the pass is not timed
        }
        Record record;
        record.Timing.Name = name;
        record.Begin       = std::chrono::steady_clock::now();
        record.FirstQuery  = m_QueryCount;
        m_Records.push_back(std::move(record));
        m_Open = true;
        if (m_Pool != nullptr)
        {
            m_Api.CmdWriteTimestamp(cmd, m_Pool, m_QueryCount++);
        }
    }

    void GpuProfiler::BeginCommands(VriCommandBuffer* cmd)
    {
        if (!m_Open)
        {
            return;
        }
        auto& record        = m_Records.back();
        record.CommandQuery = m_QueryCount;
        if (m_Pool != nullptr)
        {
            m_Api.CmdWriteTimestamp(cmd, m_Pool, m_QueryCount++);
        }
    }

    void GpuProfiler::EndPass(VriCommandBuffer* cmd)
    {
        if (!m_Open)
        {
            return;
        }
        auto& record        = m_Records.back();
        record.Timing.CpuMs = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - record.Begin).count();
        record.LastQuery    = m_QueryCount;
        m_Open              = false;
        if (m_Pool != nullptr)
        {
            m_Api.CmdWriteTimestamp(cmd, m_Pool, m_QueryCount++);
        }
    }

    void GpuProfiler::Resolve(VriCommandBuffer* cmd)
    {
        if (m_Pool != nullptr && m_QueryCount > 0)
        {
            m_Readback->Transition(cmd, {VriAccess_CopyDestinationWrite, VriPipelineStage_Transfer});
            m_Api.CmdCopyQueries(cmd, m_Pool, 0, m_QueryCount, m_Readback->Handle, 0);
        }
    }

    void GpuProfiler::Collect()
    {
        m_Results.clear();
        m_FrameGpuMs = 0;
        for (const auto& record : m_Records)
        {
            m_Results.push_back(record.Timing);
        }
        if (m_Pool == nullptr || m_QueryCount == 0)
        {
            return;
        }
        std::vector<uint64_t> ticks(m_QueryCount);
        const auto*           data = m_Device.Core.MapBuffer(m_Readback->Handle, 0, m_QueryCount * sizeof(uint64_t));
        if (data == nullptr)
        {
            throw std::runtime_error("Map timestamp results failed");
        }
        std::memcpy(ticks.data(), data, ticks.size() * sizeof(uint64_t));
        m_Device.Core.UnmapBuffer(m_Readback->Handle);

        const auto ms = [&](uint32_t from, uint32_t to) { return ticks[to] >= ticks[from] ? double(ticks[to] - ticks[from]) * m_TickNs / 1e6 : 0.0; };
        for (size_t i = 0; i < m_Records.size(); ++i)
        {
            const auto& record    = m_Records[i];
            m_Results[i].GpuMs    = ms(record.FirstQuery, record.LastQuery);
            m_Results[i].GpuBarrierMs = record.CommandQuery != UINT32_MAX ? ms(record.FirstQuery, record.CommandQuery) : 0.0;
        }
        if (!m_Records.empty())
        {
            m_FrameGpuMs = ms(m_Records.front().FirstQuery, m_Records.back().LastQuery);
        }
        SendToTracy(ticks);
    }

    void GpuProfiler::SendToTracy(const std::vector<uint64_t>& ticks)
    {
#ifdef TRACY_ENABLE
        if (!TracyIsConnected || m_Records.empty())
        {
            return;
        }
        if (!m_TracyContext)
        {
            // Tracy places the context's first GPU time at the CPU time it receives it: use this frame's first
            // timestamp (close enough to "now" with one frame in flight).
            m_TracyId = tracy::GetGpuCtxCounter().fetch_add(1);
            ___tracy_emit_gpu_new_context_serial({int64_t(ticks[m_Records.front().FirstQuery]), float(m_TickNs), m_TracyId, 0, 2 /* Vulkan */});
            static const char kName[] = "VRI";
            ___tracy_emit_gpu_context_name_serial({m_TracyId, kName, uint16_t(sizeof(kName) - 1)});
            m_TracyContext = true;
        }
        static const char kFile[] = __FILE__;
        for (const auto& record : m_Records)
        {
            const auto&    name   = record.Timing.Name;
            const uint64_t srcloc = ___tracy_alloc_srcloc(uint32_t(__LINE__), kFile, sizeof(kFile) - 1, name.c_str(), name.size(), 0);
            const uint16_t begin  = m_TracyQuery++;
            const uint16_t end    = m_TracyQuery++;
            ___tracy_emit_gpu_zone_begin_alloc_serial({srcloc, begin, m_TracyId});
            ___tracy_emit_gpu_time_serial({int64_t(ticks[record.FirstQuery]), begin, m_TracyId});
            ___tracy_emit_gpu_zone_end_serial({end, m_TracyId});
            ___tracy_emit_gpu_time_serial({int64_t(ticks[record.LastQuery]), end, m_TracyId});
        }
#else
        (void)ticks;
#endif
    }
} // namespace SnowLeopardEngine::RHI
