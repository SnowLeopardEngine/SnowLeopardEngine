#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"

#include <vri/ext/vri_ext_query.h>

#include <chrono>
#include <string>
#include <vector>

namespace SnowLeopardEngine::RHI
{
    // GPU (timestamp query) and CPU time per render-graph pass, ported from libvultra's Profiler. Each event writes
    // a timestamp at its start, after its barriers (the "commands" boundary) and at its end; results are read back
    // after the frame's SubmitAndWait. Completed frames are also sent to Tracy as GPU zones of a "VRI" GPU context
    // while a Tracy client is connected (nothing is queued otherwise).
    class GpuProfiler
    {
    public:
        struct PassTiming
        {
            std::string Name;
            double      CpuMs        = 0; // recording time
            double      GpuMs        = 0; // GPU time, barriers included
            double      GpuBarrierMs = 0; // of which before the pass's own commands
        };

        explicit GpuProfiler(Device& device, uint32_t eventCapacity = 128);
        ~GpuProfiler();

        GpuProfiler(const GpuProfiler&)            = delete;
        GpuProfiler& operator=(const GpuProfiler&) = delete;

        bool HasGpuTimings() const { return m_Pool != nullptr; }

        void BeginFrame(VriCommandBuffer* cmd);
        void BeginPass(VriCommandBuffer* cmd, const std::string& name);
        void BeginCommands(VriCommandBuffer* cmd); // after the pass's barriers
        void EndPass(VriCommandBuffer* cmd);
        void Resolve(VriCommandBuffer* cmd); // end of the frame's recording
        void Collect();                      // after SubmitAndWait

        // Timings of the last collected frame (GPU values are 0 without timestamp support).
        const std::vector<PassTiming>& GetTimings() const { return m_Results; }
        double                         GetFrameGpuMs() const { return m_FrameGpuMs; }

    private:
        struct Record
        {
            PassTiming                            Timing;
            std::chrono::steady_clock::time_point Begin;
            uint32_t                              FirstQuery   = 0;
            uint32_t                              CommandQuery = UINT32_MAX;
            uint32_t                              LastQuery    = 0;
        };

        void SendToTracy(const std::vector<uint64_t>& ticks);

        Device&                 m_Device;
        uint32_t                m_EventCapacity;
        VriQueryInterface       m_Api {};
        VriQueryPool*           m_Pool = nullptr;
        Scope<Buffer>           m_Readback;
        std::vector<Record>     m_Records;
        std::vector<PassTiming> m_Results;
        bool                    m_Open       = false;
        uint32_t                m_QueryCount = 0;
        double                  m_TickNs     = 0;
        double                  m_FrameGpuMs = 0;

        // Tracy GPU context (created when a client connects)
        bool     m_TracyContext = false;
        uint8_t  m_TracyId      = 0;
        uint16_t m_TracyQuery   = 0;
    };
} // namespace SnowLeopardEngine::RHI
