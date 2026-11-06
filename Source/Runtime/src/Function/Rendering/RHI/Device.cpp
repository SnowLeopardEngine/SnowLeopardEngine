#include "SnowLeopardEngine/Function/Rendering/RHI/Device.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace SnowLeopardEngine::RHI
{
    void Check(VriResult result, const char* operation)
    {
        if (result != VriResult_Success)
        {
            throw std::runtime_error(std::string(operation) + " failed: " + std::to_string(int(result)));
        }
    }

    namespace
    {
        void VRI_CALL Log(void*, VriMessageSeverity severity, const char* message)
        {
            const char* text = message != nullptr ? message : "";
            if (severity == VriMessageSeverity_Error)
            {
                SNOW_LEOPARD_CORE_ERROR("[VRI] {0}", text);
            }
            else if (severity == VriMessageSeverity_Warning)
            {
                SNOW_LEOPARD_CORE_WARN("[VRI] {0}", text);
            }
            else
            {
                SNOW_LEOPARD_CORE_INFO("[VRI] {0}", text);
            }
        }

        // VRI reports granted features and queues only after device creation, so every adapter is probed once and
        // the best complete one is kept: discrete > integrated > other > software, then VRAM, then queue types.
        VriDevice* SelectDevice(VriDeviceCreationDesc desc)
        {
            std::vector<VriAdapterDesc> adapters(vriEnumerateAdapters(desc.graphicsAPI, nullptr, 0));
            const auto count = vriEnumerateAdapters(desc.graphicsAPI, adapters.data(), uint32_t(adapters.size()));
            if (count == 0 || count > adapters.size())
            {
                throw std::runtime_error("No Vulkan adapter found");
            }
            adapters.resize(count);

            using DeviceOwner = std::unique_ptr<VriDevice, decltype(&vriDestroyDevice)>;
            using Score       = std::tuple<int, uint64_t, uint32_t, uint64_t>;
            DeviceOwner selected(nullptr, vriDestroyDevice);
            Score       bestScore {};
            desc.bestEffort = VRI_TRUE;
            for (uint32_t index = 0; index < count; ++index)
            {
                desc.adapterIndex          = index;
                VriDevice*  candidateHandle = nullptr;
                const auto  result          = vriCreateDevice(&desc, &candidateHandle);
                DeviceOwner candidate(candidateHandle, vriDestroyDevice);
                if (result != VriResult_Success)
                {
                    SNOW_LEOPARD_CORE_WARN("[RHI] Skipping GPU {0} ({1}): device creation failed ({2})",
                                           index,
                                           adapters[index].name,
                                           int(result));
                    continue;
                }

                VriCoreInterface core {};
                Check(vriGetInterface(candidate.get(), VRI_INTERFACE_CORE, sizeof(core), &core),
                      "Get candidate core interface");
                const auto& info    = *core.GetDeviceDesc(candidate.get());
                const auto  missing = desc.enabledFeatures & ~info.enabledFeatures;
                if (missing != 0 || info.queueCount[VriQueueType_Graphics] == 0)
                {
                    SNOW_LEOPARD_CORE_INFO("[RHI] Skipping GPU {0} ({1}): missing features 0x{2:x}, graphics queues {3}",
                                           index,
                                           info.adapter.name,
                                           missing,
                                           info.queueCount[VriQueueType_Graphics]);
                    continue;
                }

                int typeRank = 1;
                switch (info.adapter.type)
                {
                    case VriAdapterType_Discrete:
                        typeRank = 3;
                        break;
                    case VriAdapterType_Integrated:
                        typeRank = 2;
                        break;
                    case VriAdapterType_Software:
                        typeRank = 0;
                        break;
                    default:
                        break;
                }
                uint32_t queueTypes = 0;
                uint64_t queueCount = 0;
                for (const auto queues : info.queueCount)
                {
                    queueTypes += uint32_t(queues != 0);
                    queueCount += queues;
                }
                const Score score {typeRank, info.adapter.videoMemorySize, queueTypes, queueCount};
                if (!selected || score > bestScore)
                {
                    bestScore = score;
                    selected  = std::move(candidate);
                }
            }
            if (!selected)
            {
                throw std::runtime_error("No Vulkan GPU provides the required features and a graphics queue");
            }
            return selected.release();
        }
    } // namespace

    Device::Device(bool validation, uint64_t features) : Features(features)
    {
        static const VriCallbackInterface callbacks {.MessageCallback = Log};

        auto desc              = vriDefaultDeviceCreationDesc();
        desc.graphicsAPI       = VriGraphicsAPI_Vulkan;
        desc.enableValidation  = validation;
        desc.enabledFeatures   = features;
        desc.callbackInterface = &callbacks;
        // The engine's cameras and shaders are authored Y-up (OpenGL style); VRI's default is Vulkan's Y-down.
        desc.coordinates.clipSpaceY = VriClipSpaceY_Up;
        Handle                      = SelectDevice(desc);
        try
        {
            Check(vriGetInterface(Handle, VRI_INTERFACE_CORE, sizeof(Core), &Core), "Get core interface");
            const auto& info = *Core.GetDeviceDesc(Handle);
            SNOW_LEOPARD_CORE_INFO("[RHI] GPU: {0} ({1} MiB VRAM), validation {2}",
                                   info.adapter.name,
                                   info.adapter.videoMemorySize / (1024 * 1024),
                                   validation ? "on" : "off");
            Check(vriGetInterface(Handle, VRI_INTERFACE_SWAPCHAIN, sizeof(Swap), &Swap), "Get swapchain interface");
            Check(Core.GetQueue(Handle, VriQueueType_Graphics, 0, &Queue), "Get graphics queue");
            Check(vriGetInterface(
                      Handle, VRI_INTERFACE_PIPELINE_CACHE, sizeof(m_PipelineCacheApi), &m_PipelineCacheApi),
                  "Get pipeline cache interface");
            Check(m_PipelineCacheApi.CreatePipelineCache(Handle, nullptr, 0, &PipelineCache), "Create pipeline cache");
        }
        catch (...)
        {
            vriDestroyDevice(Handle);
            throw;
        }
    }

    Device::~Device()
    {
        WaitIdle();
        m_PipelineCacheApi.DestroyPipelineCache(PipelineCache);
        vriDestroyDevice(Handle);
    }

    void Device::WaitIdle() const { Core.DeviceWaitIdle(Handle); }

    Frame::Frame(Device& device) : m_Device(device)
    {
        auto& core = device.Core;
        Check(core.CreateCommandAllocator(device.Handle, VriQueueType_Graphics, &m_Allocator), "Create command allocator");
        try
        {
            Check(core.CreateCommandBuffer(m_Allocator, &m_Commands), "Create command buffer");
            Check(core.CreateFence(device.Handle, 0, &m_Fence), "Create frame fence");
        }
        catch (...)
        {
            core.DestroyCommandAllocator(m_Allocator);
            throw;
        }
    }

    Frame::~Frame()
    {
        m_Device.WaitIdle();
        m_Device.Core.DestroyFence(m_Fence);
        m_Device.Core.DestroyCommandAllocator(m_Allocator);
    }

    VriCommandBuffer* Frame::Begin()
    {
        m_Device.Core.ResetCommandAllocator(m_Allocator);
        Check(m_Device.Core.BeginCommandBuffer(m_Commands), "Begin command buffer");
        return m_Commands;
    }

    void Frame::SubmitAndWait()
    {
        auto& core = m_Device.Core;
        Check(core.EndCommandBuffer(m_Commands), "End command buffer");
        VriFenceSubmitDesc signal {m_Fence, ++m_Value, VriPipelineStage_AllCommands};
        VriQueueSubmitDesc submit {};
        submit.commandBuffers   = &m_Commands;
        submit.commandBufferNum = 1;
        submit.signalFences     = &signal;
        submit.signalFenceNum   = 1;
        core.QueueSubmit(m_Device.Queue, &submit);
        core.Wait(m_Fence, m_Value);
    }
} // namespace SnowLeopardEngine::RHI
