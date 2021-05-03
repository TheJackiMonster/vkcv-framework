#include "Context.hpp"
#include "CoreManager.hpp"

#include <iostream>

namespace vkcv {

	Context::Context(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device)
		: m_instance(instance), m_physicalDevice(physicalDevice), m_device(device)
	{}

	Context::~Context() {
		m_device.destroy();
		m_instance.destroy();
		vkcv::terminateGLFW();
	}

	Context Context::create(const char* applicationName, uint32_t applicationVersion, uint32_t queueCount, std::vector<vk::QueueFlagBits> queueFlags, std::vector<const char*> instanceExtensions, std::vector<const char*> deviceExtensions) {
		vkcv::initGLFW();

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
		
		if (!Context::checkSupport(supportedLayers, validationLayers)) {
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
		std::vector<const char*> requiredExtensions = Context::getRequiredExtensions();
		instanceExtensions.insert(instanceExtensions.end(), requiredExtensions.begin(), requiredExtensions.end());

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

		return Context(instance, physicalDevice, device);
	}

	const vk::Instance& Context::getInstance() const {
		return m_instance;
	}

	const vk::PhysicalDevice& Context::getPhysicalDevice() const {
		return m_physicalDevice;
	}

	const vk::Device& Context::getDevice() const {
		return m_device;
	}

	/// <summary>
	/// All existing physical devices will be evaluated by 
	/// </summary>
	/// <param name="instance">The instance.</param>
	/// <returns>The optimal physical device.</returns>
	/// <seealso cref="Context.deviceScore">
	vk::PhysicalDevice Context::pickPhysicalDevice(vk::Instance& instance) {
		vk::PhysicalDevice phyDevice;
		std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices();

		if (devices.size() == 0) {
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

	/// <summary>
	/// The physical device is evaluated by three categories: discrete GPU vs. integrated GPU, amount of queues and
	/// its abilities, and VRAM.
	/// </summary>
	/// <param name="physicalDevice"> The physical device. </param>
	/// <returns></returns>
	int Context::deviceScore(const vk::PhysicalDevice& physicalDevice) {
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
			score += qCount * bitCount;
		}

		// compute the VRAM of the physical device
		vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();
		int vram = static_cast<int>(memoryProperties.memoryHeaps[0].size / 1E9);
		score *= vram;

		if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
			score *= 2;
		}
		else if (properties.deviceType != vk::PhysicalDeviceType::eIntegratedGpu) {
			score = -1;
		}

		return score;
	}

	/// <summary>
	/// Creates a candidate list of queues that all meet the desired flags and then creates the maximum possible number
	/// of queues. If the number of desired queues is not sufficient, the remaining queues are created from the next
	/// candidate from the list.
	/// </summary>
	/// <param name="physicalDevice">The physical device</param>
	/// <param name="queueCount">The amount of queues to be created</param>
	/// <param name="queueFlags">The abilities which have to be supported by any created queue</param>
	/// <returns></returns>
	std::vector<vk::DeviceQueueCreateInfo> Context::getQueueCreateInfos(vk::PhysicalDevice& physicalDevice, uint32_t queueCount,std::vector<float> &qPriorities, std::vector<vk::QueueFlagBits>& queueFlags) {
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
		for (int i = 0; i < qFamilyCandidates.size() && create > 0; i++) {
			const int maxCreatableQueues = std::min(create, qFamilyCandidates[i].queueCount);		
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

	/// <summary>
	/// With the help of the reference <paramref name="supported"> all elements in <paramref name="check"/> checked,
	/// if they are supported by the physical device.
	/// </summary>
	/// <param name="supported">The reference that can be used to check <paramref name="check"/></param>
	/// <param name="check">The elements to be checked</param>
	/// <returns>True, if all elements in <param name="check"> are supported</returns>
	bool Context::checkSupport(std::vector<const char*>& supported, std::vector<const char*>& check) {
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

	/// <summary>
	/// Gets all extensions required, i.e. GLFW and advanced debug extensions.
	/// </summary>
	/// <returns>The required extensions</returns>
	std::vector<const char*> Context::getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifndef NDEBUG
	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		return extensions;
	}
}
