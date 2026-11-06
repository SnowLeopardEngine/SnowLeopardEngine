#pragma once

#include <vri/ext/vri_ext_pipeline_cache.h>
#include <vri/vri.h>

#include <cstdint>

namespace SnowLeopardEngine::RHI
{
    // Throws std::runtime_error naming the operation when a VRI call fails.
    void Check(VriResult result, const char* operation);

    // Owns the VRI device and its interface tables. VRI stays the rendering API; this class only owns handles
    // (ported from libvultra's Device).
    class Device
    {
    public:
        explicit Device(bool validation, uint64_t features = 0);
        ~Device();

        Device(const Device&)            = delete;
        Device& operator=(const Device&) = delete;

        void WaitIdle() const;

        VriDevice*            Handle   = nullptr;
        uint64_t              Features = 0;
        VriQueue*             Queue    = nullptr; // graphics queue
        VriCoreInterface      Core {};
        VriSwapChainInterface Swap {};
        VriPipelineCache*     PipelineCache = nullptr; // pass to graphics / compute pipeline descriptors

    private:
        VriPipelineCacheInterface m_PipelineCacheApi {};
    };

    // One frame in flight: SubmitAndWait is the lifetime boundary for every resource the frame used.
    class Frame
    {
    public:
        explicit Frame(Device& device);
        ~Frame();

        Frame(const Frame&)            = delete;
        Frame& operator=(const Frame&) = delete;

        VriCommandBuffer* Begin();
        void              SubmitAndWait();

    private:
        Device&              m_Device;
        VriCommandAllocator* m_Allocator = nullptr;
        VriCommandBuffer*    m_Commands  = nullptr;
        VriFence*            m_Fence     = nullptr;
        uint64_t             m_Value     = 0;
    };
} // namespace SnowLeopardEngine::RHI
