/**
 * @authors Sebastian Gaida
 * @file src/vkcv/CoreManager.cpp
 * @brief Handling of global states regarding dependencies
 */

#include "vkcv/Core.hpp"

namespace vkcv
{
    static vk::ImageLayout getVkLayoutFromAttachLayout(AttachmentLayout layout)
    {
        switch(layout)
        {
            case AttachmentLayout::GENERAL:
                return vk::ImageLayout::eGeneral;
            case AttachmentLayout::COLOR_ATTACHMENT:
                return vk::ImageLayout::eColorAttachmentOptimal;
            case AttachmentLayout::SHADER_READ_ONLY:
                return vk::ImageLayout::eShaderReadOnlyOptimal;
            case AttachmentLayout::DEPTH_STENCIL_ATTACHMENT:
                return vk::ImageLayout::eDepthStencilAttachmentOptimal;
            case AttachmentLayout::DEPTH_STENCIL_READ_ONLY:
                return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            case AttachmentLayout::PRESENTATION:
                return vk::ImageLayout::ePresentSrcKHR;
            default:
                return vk::ImageLayout::eUndefined;
        }
    }

    static vk::AttachmentStoreOp getVkStoreOpFromAttachOp(AttachmentOperation op)
    {
        switch(op)
        {
            case AttachmentOperation::STORE:
                return vk::AttachmentStoreOp::eStore;
            default:
                return vk::AttachmentStoreOp::eDontCare;
        }
    }

    static vk::AttachmentLoadOp getVKLoadOpFromAttachOp(AttachmentOperation op)
    {
        switch(op)
        {
            case AttachmentOperation::LOAD:
                return vk::AttachmentLoadOp::eLoad;
            case AttachmentOperation::CLEAR:
                return vk::AttachmentLoadOp::eClear;
            default:
                return vk::AttachmentLoadOp::eDontCare;
        }
    }

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
            m_NextPassId(0),
            m_Renderpasses{}
    {}

    Core::~Core() noexcept
    {
        for(const auto &pass : m_Renderpasses)
            m_Context.m_Device.destroy(pass);
        m_Renderpasses.clear();
        m_NextPassId = 0;
    }

    bool Core::createRenderpass(const Renderpass &pass, RenderpassHandle &handle)
    {
        // description of all {color, input, depth/stencil} attachments of the render pass
        std::vector<vk::AttachmentDescription> attachmentDescriptions{};

        // individual references to color attachments (of a subpass)
        std::vector<vk::AttachmentReference> colorAttachmentReferences{};
        // individual reference to depth attachment (of a subpass)
        vk::AttachmentReference depthAttachmentReference{};

        for(uint32_t i = 0; i < pass.attachments.size(); i++)
        {
            // TODO: Renderpass struct should hold proper format information
            vk::Format format;

            if(pass.attachments[i].layout_in_pass == AttachmentLayout::DEPTH_STENCIL_ATTACHMENT)
            {
                format = vk::Format::eD16Unorm; // depth attachments;

                depthAttachmentReference.attachment = i;
                depthAttachmentReference.layout = getVkLayoutFromAttachLayout(pass.attachments[i].layout_in_pass);
            }
            else
            {
                format = vk::Format::eB8G8R8A8Srgb; // color attachments, compatible with swapchain
                vk::AttachmentReference attachmentRef(i, getVkLayoutFromAttachLayout(pass.attachments[i].layout_in_pass));
                colorAttachmentReferences.push_back(attachmentRef);
            }

            vk::AttachmentDescription attachmentDesc({},
                                                     format,
                                                     vk::SampleCountFlagBits::e1,
                                                     getVKLoadOpFromAttachOp(pass.attachments[i].load_operation),
                                                     getVkStoreOpFromAttachOp(pass.attachments[i].load_operation),
                                                     vk::AttachmentLoadOp::eDontCare,
                                                     vk::AttachmentStoreOp::eDontCare,
                                                     getVkLayoutFromAttachLayout(pass.attachments[i].layout_initial),
                                                     getVkLayoutFromAttachLayout(pass.attachments[i].layout_final));
            attachmentDescriptions.push_back(attachmentDesc);
        }

        vk::SubpassDescription subpassDescription({},
                                           vk::PipelineBindPoint::eGraphics,
                                           0,
                                           {},
                                           static_cast<uint32_t>(colorAttachmentReferences.size()),
                                           colorAttachmentReferences.data(),
                                           {},
                                           &depthAttachmentReference,
                                           0,
                                           {});

        vk::RenderPassCreateInfo passInfo({},
                                          static_cast<uint32_t>(attachmentDescriptions.size()),
                                          attachmentDescriptions.data(),
                                          1,
                                          &subpassDescription,
                                          0,
                                          {});

        vk::RenderPass vkObject{nullptr};
        if(m_Context.m_Device.createRenderPass(&passInfo, nullptr, &vkObject) != vk::Result::eSuccess)
            return false;

        m_Renderpasses.push_back(vkObject);
        handle.id = m_NextPassId++;
        return true;
    }
}
