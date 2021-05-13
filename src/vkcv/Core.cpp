/**
 * @authors Sebastian Gaida
 * @file src/vkcv/CoreManager.cpp
 * @brief Handling of global states regarding dependencies
 */

#include "vkcv/Core.hpp"

namespace vkcv
{
    /**
     * @brief The physical device is evaluated by three categories:
     * discrete GPU vs. integrated GPU, amount of queues and its abilities, and VRAM.physicalDevice.
     * @param physicalDevice The physical device
     * @return Device score as integer
    */
    int deviceScore(const vk::PhysicalDevice& physicalDevice)
    {
        int score = 0;
        vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
        std::vector<vk::QueueFamilyProperties> qFamilyProperties = physicalDevice.getQueueFamilyProperties();

        // for every queue family compute queue flag bits and the amount of queues
        for (const auto& qFamily : qFamilyProperties) {
            uint32_t qCount = qFamily.queueCount;
            uint32_t bitCount = (static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eCompute) != 0)
                                + (static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eGraphics) != 0)
                                + (static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eTransfer) != 0)
                                + (static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eSparseBinding) != 0);
            score += static_cast<int>(qCount * bitCount);
        }

        // compute the VRAM of the physical device
        vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
        auto vram = static_cast<int>(memoryProperties.memoryHeaps[0].size / static_cast<uint32_t>(1E9));
        score *= vram;

        if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            score *= 2;
        }
        else if (properties.deviceType != vk::PhysicalDeviceType::eIntegratedGpu) {
            score = -1;
        }

        return score;
    }

    /**
     * @brief All existing physical devices will be evaluated by deviceScore.
     * @param instance The instance
     * @return The optimal physical device
     * @see Context.deviceScore
    */
    vk::PhysicalDevice pickPhysicalDevice(vk::Instance& instance)
    {
        vk::PhysicalDevice phyDevice;
        std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

        if (devices.empty()) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        int max_score = -1;
        for (const auto& device : devices) {
            int score = deviceScore(device);
            if (score > max_score) {
                max_score = score;
                phyDevice = device;
            }
        }

        if (max_score == -1) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        return phyDevice;
    }


    /**
     * @brief Creates a candidate list of queues that all meet the desired flags and then creates the maximum possible number
     * of queues. If the number of desired queues is not sufficient, the remaining queues are created from the next
     * candidate from the list.
     * @param physicalDevice The physical device
     * @param queueCount The amount of queues to be created
     * @param qPriorities
     * @param queueFlags The abilities which have to be supported by any created queue
     * @return
    */
    std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos(vk::PhysicalDevice& physicalDevice,
                                                               uint32_t queueCount,
                                                               std::vector<float> &qPriorities,
                                                               std::vector<vk::QueueFlagBits>& queueFlags)
    {
        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::vector<vk::QueueFamilyProperties> qFamilyProperties = physicalDevice.getQueueFamilyProperties();
        std::vector<vk::QueueFamilyProperties> qFamilyCandidates;

        // search for queue families which support the desired queue flag bits
        for (auto& qFamily : qFamilyProperties) {
            bool supported = true;
            for (auto qFlag : queueFlags) {
                supported = supported && (static_cast<uint32_t>(qFlag & qFamily.queueFlags) != 0);
            }
            if (supported) {
                qFamilyCandidates.push_back(qFamily);
            }
        }

        uint32_t create = queueCount;
        for (uint32_t i = 0; i < qFamilyCandidates.size() && create > 0; i++) {
            const uint32_t maxCreatableQueues = std::min(create, qFamilyCandidates[i].queueCount);
            vk::DeviceQueueCreateInfo qCreateInfo(
                    vk::DeviceQueueCreateFlags(),
                    i,
                    maxCreatableQueues,
                    qPriorities.data()
            );
            queueCreateInfos.push_back(qCreateInfo);
            create -= maxCreatableQueues;
        }

        return queueCreateInfos;
    }

    /**
     * @brief With the help of the reference "supported" all elements in "check" checked,
     * if they are supported by the physical device.
     * @param supported The reference that can be used to check "check"
     * @param check The elements to be checked
     * @return True, if all elements in "check" are supported
    */
    bool checkSupport(std::vector<const char*>& supported, std::vector<const char*>& check)
    {
        for (auto checkElem : check) {
            bool found = false;
            for (auto supportedElem : supported) {
                if (strcmp(supportedElem, checkElem) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found)
                return false;
        }
        return true;
    }

    Core Core::create(const char *applicationName,
                      uint32_t applicationVersion,
                      uint32_t queueCount,
                      std::vector<vk::QueueFlagBits> queueFlags,
                      std::vector<const char *> instanceExtensions,
                      std::vector<const char *> deviceExtensions)
    {

        // check for layer support

        const std::vector<vk::LayerProperties>& layerProperties = vk::enumerateInstanceLayerProperties();

        std::vector<const char*> supportedLayers;
        supportedLayers.reserve(layerProperties.size());

        for (auto& elem : layerProperties) {
            supportedLayers.push_back(elem.layerName);
        }

// if in debug mode, check if validation layers are supported. Enable them if supported
#ifndef NDEBUG
        std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };

        if (!checkSupport(supportedLayers, validationLayers)) {
            throw std::runtime_error("Validation layers requested but not available!");
        }
#endif

        // check for extension support
        std::vector<vk::ExtensionProperties> instanceExtensionProperties = vk::enumerateInstanceExtensionProperties();

        std::vector<const char*> supportedExtensions;
        supportedExtensions.reserve(instanceExtensionProperties.size());

        for (auto& elem : instanceExtensionProperties) {
            supportedExtensions.push_back(elem.extensionName);
        }

        if (!checkSupport(supportedExtensions, instanceExtensions)) {
            throw std::runtime_error("The requested instance extensions are not supported!");
        }

#ifndef NDEBUG
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        const vk::ApplicationInfo applicationInfo(
                applicationName,
                applicationVersion,
                "vkCV",
                VK_MAKE_VERSION(0, 0, 1),
                VK_HEADER_VERSION_COMPLETE
        );

        vk::InstanceCreateInfo instanceCreateInfo(
                vk::InstanceCreateFlags(),
                &applicationInfo,
                0,
                nullptr,
                static_cast<uint32_t>(instanceExtensions.size()),
                instanceExtensions.data()
        );

#ifndef NDEBUG
        instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif

        vk::Instance instance = vk::createInstance(instanceCreateInfo);

        std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
        vk::PhysicalDevice physicalDevice = pickPhysicalDevice(instance);

        // check for physical device extension support
        std::vector<vk::ExtensionProperties> deviceExtensionProperties = physicalDevice.enumerateDeviceExtensionProperties();
        supportedExtensions.clear();
        for (auto& elem : deviceExtensionProperties) {
            supportedExtensions.push_back(elem.extensionName);
        }
        if (!checkSupport(supportedExtensions, deviceExtensions)) {
            throw std::runtime_error("The requested device extensions are not supported by the physical device!");
        }

        //vector to define the queue priorities
        std::vector<float> qPriorities;
        qPriorities.resize(queueCount, 1.f); // all queues have the same priorities

        // create required queues
        std::vector<vk::DeviceQueueCreateInfo> qCreateInfos = getQueueCreateInfos(physicalDevice, queueCount, qPriorities,queueFlags);

        vk::DeviceCreateInfo deviceCreateInfo(
                vk::DeviceCreateFlags(),
                qCreateInfos.size(),
                qCreateInfos.data(),
                0,
                nullptr,
                deviceExtensions.size(),
                deviceExtensions.data(),
                nullptr		// Should our device use some features??? If yes: TODO
        );

#ifndef NDEBUG
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif


        vk::Device device = physicalDevice.createDevice(deviceCreateInfo);
        // TODO: implement device.getQueue() to access the queues, if needed
        Context context(instance, physicalDevice, device);

        return Core(std::move(context));
    }

    const Context &Core::getContext() const
    {
        return m_Context;
    }

    Core::Core(Context &&context) noexcept :
            m_Context(std::move(context)),
            m_NextPipelineId(0),
            m_Pipelines{},
            m_PipelineLayouts{},
            m_NextRenderpassId(0),
            m_Renderpasses{}
    {}

    bool Core::createPipeline(const Pipeline &pipeline, PipelineHandle &handle) {

        // vertex shader stage
        vk::ShaderModuleCreateInfo vertexModuleInfo({},pipeline.m_vertexCode.size(), pipeline.m_vertexCode.data());
        vk::ShaderModule vertexModule{};
        if(m_Context.m_Device.createShaderModule(&vertexModuleInfo, nullptr, &vertexModule) != vk::Result::eSuccess){
            return false;
        };
        vk::PipelineShaderStageCreateInfo pipelineVertexShaderStageInfo({}, vk::ShaderStageFlagBits::eVertex,vertexModule, "main",
                                                                        nullptr);

        // fragment shader stage
        vk::ShaderModuleCreateInfo fragmentModuleInfo({},pipeline.m_fragCode.size(), pipeline.m_fragCode.data());
        vk::ShaderModule fragmentModule{};
        if(m_Context.m_Device.createShaderModule(&fragmentModuleInfo, nullptr, &fragmentModule) != vk::Result::eSuccess){
            return false;
        };
        vk::PipelineShaderStageCreateInfo pipelineFragmentShaderStageInfo({}, vk::ShaderStageFlagBits::eFragment,fragmentModule, "main",
                                                                        nullptr);

        // vertex input state
        vk::VertexInputBindingDescription vertexInputBindingDescription(0,12,vk::VertexInputRate::eVertex);
        vk::VertexInputAttributeDescription vertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, 0);

        vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo({}, 1, &vertexInputBindingDescription, 1, &vertexInputAttributeDescription);

        // input assembly state
        vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, false);

        // viewport state
        vk::Viewport viewport(0.f, 0.f, static_cast<float>(pipeline.m_width), static_cast<float>(pipeline.m_height), 0.f, 1.f);
        vk::Rect2D scissor({0,0},{pipeline.m_width, pipeline.m_height});
        vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo({}, 1, &viewport, 1, &scissor);

        // rasterization state
        vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
                {},
                false,
                false,
                vk::PolygonMode::eFill,
                vk::CullModeFlagBits::eNone,
                vk::FrontFace::eCounterClockwise,
                false,
                0.f,
                0.f,
                0.f,
                1.f
                );

        // multisample state
        vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
                {},
                vk::SampleCountFlagBits::e1,
                false,
                0.f,
                nullptr,
                false,
                false
                );

        // color blend state
        vk::ColorComponentFlags colorWriteMask(VK_COLOR_COMPONENT_R_BIT |
                                               VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT |
                                               VK_COLOR_COMPONENT_A_BIT);
        vk::PipelineColorBlendAttachmentState colorBlendAttachmentState(
                false,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eOne,
                vk::BlendOp::eAdd,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eOne,
                vk::BlendOp::eAdd,
                colorWriteMask
                );
        vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
                {},
                false,
                vk::LogicOp::eClear,
                0,
                &colorBlendAttachmentState,
                {1.f,1.f,1.f,1.f}
                );

        // pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
                {},
                0,
                {},
                0,
                {}
                );
        vk::PipelineLayout vkPipelineLayout{};
        if(m_Context.m_Device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout) != vk::Result::eSuccess){
            return false;
        }

        // graphics pipeline create
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {pipelineVertexShaderStageInfo, pipelineFragmentShaderStageInfo};
        vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
                {},
                static_cast<uint32_t>(shaderStages.size()),
                shaderStages.data(),
                &pipelineVertexInputStateCreateInfo,
                &pipelineInputAssemblyStateCreateInfo,
                nullptr,
                &pipelineViewportStateCreateInfo,
                &pipelineRasterizationStateCreateInfo,
                &pipelineMultisampleStateCreateInfo,
                nullptr,
                &pipelineColorBlendStateCreateInfo,
                nullptr,
                vkPipelineLayout,
                m_Renderpasses[pipeline.m_passHandle.id],
                0,
                {},
                0
                );

        vk::Pipeline vkPipeline{};
        if(m_Context.m_Device.createGraphicsPipelines(nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &vkPipeline) != vk::Result::eSuccess){
            return false;
        }

        m_Pipelines.push_back(vkPipeline);
        m_PipelineLayouts.push_back(vkPipelineLayout);
        handle.id = m_NextPipelineId++;
        return true;
    }
}
