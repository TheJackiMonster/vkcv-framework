
#include <GLFW/glfw3.h>

#include "vkcv/Context.hpp"

namespace vkcv
{
    Context::Context(Context &&other) noexcept:
            m_Instance(other.m_Instance),
            m_PhysicalDevice(other.m_PhysicalDevice),
            m_Device(other.m_Device),
            m_QueueManager(other.m_QueueManager),
            m_Allocator(other.m_Allocator)
    {
        other.m_Instance        = nullptr;
        other.m_PhysicalDevice  = nullptr;
        other.m_Device          = nullptr;
		other.m_Allocator		= nullptr;
    }

    Context & Context::operator=(Context &&other) noexcept
    {
        m_Instance          = other.m_Instance;
        m_PhysicalDevice    = other.m_PhysicalDevice;
        m_Device            = other.m_Device;
        m_QueueManager		= other.m_QueueManager;
        m_Allocator			= other.m_Allocator;

        other.m_Instance        = nullptr;
        other.m_PhysicalDevice  = nullptr;
        other.m_Device          = nullptr;
        other.m_Allocator		= nullptr;

        return *this;
    }

    Context::Context(vk::Instance instance,
                     vk::PhysicalDevice physicalDevice,
                     vk::Device device,
					 QueueManager&& queueManager,
					 vma::Allocator&& allocator) noexcept :
    m_Instance(instance),
    m_PhysicalDevice(physicalDevice),
    m_Device(device),
    m_QueueManager(queueManager),
    m_Allocator(allocator)
    {}

    Context::~Context() noexcept
    {
    	m_Allocator.destroy();
        m_Device.destroy();
        m_Instance.destroy();
    }

    const vk::Instance &Context::getInstance() const
    {
        return m_Instance;
    }

    const vk::PhysicalDevice &Context::getPhysicalDevice() const
    {
        return m_PhysicalDevice;
    }

    const vk::Device &Context::getDevice() const
    {
        return m_Device;
    }
    
    const QueueManager& Context::getQueueManager() const {
    	return m_QueueManager;
    }
    
    const vma::Allocator& Context::getAllocator() const {
    	return m_Allocator;
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
	 * @brief With the help of the reference "supported" all elements in "check" checked,
	 * if they are supported by the physical device.
	 * @param supported The reference that can be used to check "check"
	 * @param check The elements to be checked
	 * @return True, if all elements in "check" are supported
	*/
	bool checkSupport(const std::vector<const char*>& supported, const std::vector<const char*>& check)
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
	
	
	std::vector<const char*> getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		
		return extensions;
	}
	
	bool isPresentInCharPtrVector(const std::vector<const char*>& v, const char* term){
		for (const auto& entry : v) {
			if (strcmp(entry, term) != 0) {
				return true;
			}
		}
		return false;
	}
	
	Context Context::create(const char *applicationName,
							uint32_t applicationVersion,
							const std::vector<vk::QueueFlagBits>& queueFlags,
							const std::vector<const char *>& instanceExtensions,
							const std::vector<const char *>& deviceExtensions) {
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
		
		// for GLFW: get all required extensions
		std::vector<const char*> requiredExtensions = getRequiredExtensions();
		requiredExtensions.insert(requiredExtensions.end(), instanceExtensions.begin(), instanceExtensions.end());
		
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
				static_cast<uint32_t>(requiredExtensions.size()),
				requiredExtensions.data()
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
		
		std::vector<vk::DeviceQueueCreateInfo> qCreateInfos;
		
		// create required queues
		std::vector<float> qPriorities;
		qPriorities.resize(queueFlags.size(), 1.f);
		std::vector<std::pair<int, int>> queuePairsGraphics, queuePairsCompute, queuePairsTransfer;
		
		QueueManager::queueCreateInfosQueueHandles(physicalDevice, qPriorities, queueFlags, qCreateInfos, queuePairsGraphics, queuePairsCompute, queuePairsTransfer);
		
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
		const bool shaderFloat16 = checkSupport(deviceExtensions, { "VK_KHR_shader_float16_int8" });
		const bool storage16bit  = checkSupport(deviceExtensions, { "VK_KHR_16bit_storage" });
		
		// FIXME: check if device feature is supported
		vk::PhysicalDeviceShaderFloat16Int8Features deviceShaderFloat16Int8Features;
		deviceShaderFloat16Int8Features.shaderFloat16 = shaderFloat16;
		
		vk::PhysicalDevice16BitStorageFeatures device16BitStorageFeatures;
		device16BitStorageFeatures.storageBuffer16BitAccess = storage16bit;
		
		vk::PhysicalDeviceFeatures2 deviceFeatures2;
		deviceFeatures2.features.fragmentStoresAndAtomics = true;
		deviceFeatures2.features.geometryShader = true;
		deviceFeatures2.features.depthClamp = true;
		deviceFeatures2.features.shaderInt16 = true;
		
		const bool usingMeshShaders = isPresentInCharPtrVector(deviceExtensions, VK_NV_MESH_SHADER_EXTENSION_NAME);
		vk::PhysicalDeviceMeshShaderFeaturesNV meshShadingFeatures;
		if (usingMeshShaders) {
			meshShadingFeatures.taskShader = true;
			meshShadingFeatures.meshShader = true;
            deviceFeatures2.setPNext(&meshShadingFeatures);
		}
		
		if (shaderFloat16) {
			deviceFeatures2.setPNext(&deviceShaderFloat16Int8Features);
		}
		
		if (storage16bit) {
			deviceShaderFloat16Int8Features.setPNext(&device16BitStorageFeatures);
		}

		// add raytracing features, if available
		const bool usingRTX = isPresentInCharPtrVector(deviceExtensions, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		if (usingRTX) {
		    vk::PhysicalDeviceFeatures2 raytracingFeatures2;    // INFO: vulkan says we can only use one PhysicalDeviceFeatures2 struct -> replace deviceFeatures2 by raytracingFeatures2
		    vk::PhysicalDeviceVulkan12Features raytracingFeatures12;
		    vk::PhysicalDeviceVulkan11Features raytracingFeatures11;
		    vk::PhysicalDeviceAccelerationStructureFeaturesKHR raytracingAccelerationFeatures;
		    vk::PhysicalDeviceRayTracingPipelineFeaturesKHR raytracingPipelineFeatures;

		    raytracingFeatures2.setPNext(&raytracingFeatures12);
		    raytracingFeatures12.setPNext(&raytracingFeatures11);
		    raytracingFeatures11.setPNext(&raytracingAccelerationFeatures);
		    raytracingAccelerationFeatures.setPNext(&raytracingPipelineFeatures);

		    physicalDevice.getFeatures2(&raytracingFeatures2);  // query supported features

		    // add raytracing features to deviceFeatures2
		    deviceFeatures2.setFeatures(raytracingFeatures2.features);  // TODO: Ist das so korrekt? Haette sonst an dem letzten pNext Eintrag (leider ein void*) die anderen Features angekettet
		    deviceFeatures2.features.fragmentStoresAndAtomics = true;   // TODO: Wir koennten hier auch die .features Zeilen von oben runternehmen zu nach den Klammern
		    deviceFeatures2.features.geometryShader = true;
		    deviceFeatures2.features.depthClamp = true;
		    deviceFeatures2.features.shaderInt16 = true;
		}

		deviceCreateInfo.setPNext(&deviceFeatures2);

		// Ablauf
		// qCreateInfos erstellen --> braucht das Device
		// device erstellen
		// jetzt koennen wir mit dem device die queues erstellen
		
		vk::Device device = physicalDevice.createDevice(deviceCreateInfo);

		if (usingMeshShaders)
		{
			InitMeshShaderDrawFunctions(device);
		}
		
		QueueManager queueManager = QueueManager::create(
				device,
				queuePairsGraphics,
				queuePairsCompute,
				queuePairsTransfer
		);
		
		const vma::AllocatorCreateInfo allocatorCreateInfo (
				vma::AllocatorCreateFlags(),
				physicalDevice,
				device,
				0,
				nullptr,
				nullptr,
				0,
				nullptr,
				nullptr,
				nullptr,
				instance,
				
				/* Uses default version when set to 0 (currently VK_VERSION_1_0):
				 *
				 * The reason for this is that the allocator restricts the allowed version
				 * to be at maximum VK_VERSION_1_1 which is already less than
				 * VK_HEADER_VERSION_COMPLETE at most platforms.
				 * */
				0
		);
		
		vma::Allocator allocator = vma::createAllocator(allocatorCreateInfo);
		
		return Context(
				instance,
				physicalDevice,
				device,
				std::move(queueManager),
				std::move(allocator)
		);
	}
 
}
