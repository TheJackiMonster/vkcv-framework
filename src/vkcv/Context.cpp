
#include "vkcv/Context.hpp"
#include "vkcv/Core.hpp"
#include "vkcv/Window.hpp"

namespace vkcv {
	Context::Context(Context &&other) noexcept :
		m_Instance(other.m_Instance), m_PhysicalDevice(other.m_PhysicalDevice),
		m_Device(other.m_Device), m_FeatureManager(std::move(other.m_FeatureManager)),
		m_QueueManager(std::move(other.m_QueueManager)), m_Allocator(other.m_Allocator) {
		other.m_Instance = nullptr;
		other.m_PhysicalDevice = nullptr;
		other.m_Device = nullptr;
		other.m_Allocator = nullptr;
	}

	Context &Context::operator=(Context &&other) noexcept {
		m_Instance = other.m_Instance;
		m_PhysicalDevice = other.m_PhysicalDevice;
		m_Device = other.m_Device;
		m_FeatureManager = std::move(other.m_FeatureManager);
		m_QueueManager = std::move(other.m_QueueManager);
		m_Allocator = other.m_Allocator;

		other.m_Instance = nullptr;
		other.m_PhysicalDevice = nullptr;
		other.m_Device = nullptr;
		other.m_Allocator = nullptr;

		return *this;
	}

	Context::Context(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device,
					 FeatureManager &&featureManager, QueueManager &&queueManager,
					 vma::Allocator &&allocator) noexcept :
		m_Instance(instance),
		m_PhysicalDevice(physicalDevice), m_Device(device),
		m_FeatureManager(std::move(featureManager)), m_QueueManager(std::move(queueManager)),
		m_Allocator(allocator) {}

	Context::~Context() noexcept {
		m_Allocator.destroy();
		m_Device.destroy();
		m_Instance.destroy();
	}

	const vk::Instance &Context::getInstance() const {
		return m_Instance;
	}

	const vk::PhysicalDevice &Context::getPhysicalDevice() const {
		return m_PhysicalDevice;
	}

	const vk::Device &Context::getDevice() const {
		return m_Device;
	}

	const FeatureManager &Context::getFeatureManager() const {
		return m_FeatureManager;
	}

	const QueueManager &Context::getQueueManager() const {
		return m_QueueManager;
	}

	const vma::Allocator &Context::getAllocator() const {
		return m_Allocator;
	}

	/**
	 * @brief All existing physical devices will be evaluated.
	 * @param instance The instance
	 * @param physicalDevice The optimal physical device
	 * @return Returns if a suitable GPU is found as physical device
	 */
	static bool pickPhysicalDevice(const vk::Instance &instance,
								   vk::PhysicalDevice &physicalDevice) {
		const std::vector<vk::PhysicalDevice> &devices = instance.enumeratePhysicalDevices();

		if (devices.empty()) {
			vkcv_log(LogLevel::ERROR, "Failed to find GPUs with Vulkan support");
			return false;
		}

		vk::PhysicalDeviceType type = vk::PhysicalDeviceType::eOther;
		vk::DeviceSize vramSize = 0;

		for (const auto &device : devices) {
			const auto &properties = device.getProperties();

			if (vk::PhysicalDeviceType::eOther == type) {
				type = properties.deviceType;
				physicalDevice = device;
				continue;
			}

			if (vk::PhysicalDeviceType::eDiscreteGpu != properties.deviceType)
				continue;

			if (vk::PhysicalDeviceType::eDiscreteGpu != type) {
				type = properties.deviceType;
				physicalDevice = device;
				continue;
			}

			vk::PhysicalDeviceMemoryProperties memoryProperties =
				physicalDevice.getMemoryProperties();
			vk::DeviceSize maxSize = 0;

			for (uint32_t i = 0; i < memoryProperties.memoryHeapCount; i++)
				if (memoryProperties.memoryHeaps [i].size > maxSize)
					maxSize = memoryProperties.memoryHeaps [i].size;

			if (maxSize > vramSize) {
				type = properties.deviceType;
				physicalDevice = device;
				vramSize = maxSize;
			}
		}

		return true;
	}

	/**
	 * @brief Check whether all string occurrences in "check" are contained in "supported"
	 * @param supported The const vector const char* reference used to compare entries in "check"
	 * @param check The const vector const char* reference elements to be checked by "supported"
	 * @return True, if all elements in "check" are supported (contained in supported)
	 */
	bool checkSupport(const std::vector<const char*> &supported,
					  const std::vector<const char*> &check) {
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

	std::vector<std::string> getRequiredExtensions() {
		std::vector<std::string> extensions = Window::getExtensions();

#ifndef NDEBUG
		extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

		return extensions;
	}

	/**
	 * Given the @p physicalDevice and the @p queuePriorities, the @p queueCreateInfos are computed.
	 * First, the requested queues are sorted by priority depending on the availability of queues in
	 * the queue families of the given
	 * @p physicalDevice. Then check, if all requested queues are creatable. If so, the @p
	 * queueCreateInfos will be computed. Furthermore, lists of index pairs (queueFamilyIndex,
	 * queueIndex) for later referencing of the separate queues will be computed.
	 * @param[in] physicalDevice The physical device
	 * @param[in] queuePriorities The queue priorities used for the computation of @p
	 * queueCreateInfos
	 * @param[in] queueFlags The queue flags requesting the queues
	 * @param[in,out] queueCreateInfos The queue create info structures to be created
	 * @param[in,out] queuePairsGraphics The list of index pairs (queueFamilyIndex, queueIndex) of
	 * queues of type vk::QueueFlagBits::eGraphics
	 * @param[in,out] queuePairsCompute The list of index pairs (queueFamilyIndex, queueIndex) of
	 * queues of type vk::QueueFlagBits::eCompute
	 * @param[in,out] queuePairsTransfer The list of index pairs (queueFamilyIndex, queueIndex) of
	 * queues of type vk::QueueFlagBits::eTransfer
	 * @throws std::runtime_error If the requested queues from @p queueFlags are not creatable due
	 * to insufficient availability.
	 */
	static void
	queueCreateInfosQueueHandles(vk::PhysicalDevice &physicalDevice,
								 const std::vector<float> &queuePriorities,
								 const std::vector<vk::QueueFlagBits> &queueFlags,
								 std::vector<vk::DeviceQueueCreateInfo> &queueCreateInfos,
								 std::vector<std::pair<int, int>> &queuePairsGraphics,
								 std::vector<std::pair<int, int>> &queuePairsCompute,
								 std::vector<std::pair<int, int>> &queuePairsTransfer) {
		queueCreateInfos = {};
		queuePairsGraphics = {};
		queuePairsCompute = {};
		queuePairsTransfer = {};
		std::vector<vk::QueueFamilyProperties> qFamilyProperties =
			physicalDevice.getQueueFamilyProperties();

		// check priorities of flags -> the lower prioCount the higher the priority
		std::vector<int> prios;
		for (auto flag : queueFlags) {
			int prioCount = 0;
			for (size_t i = 0; i < qFamilyProperties.size(); i++) {
				prioCount += (static_cast<uint32_t>(flag & qFamilyProperties [i].queueFlags) != 0)
						   * qFamilyProperties [i].queueCount;
			}
			prios.push_back(prioCount);
		}
		// resort flags with heighest priority before allocating the queues
		std::vector<vk::QueueFlagBits> newFlags;
		for (size_t i = 0; i < prios.size(); i++) {
			auto minElem = std::min_element(prios.begin(), prios.end());
			int index = minElem - prios.begin();
			newFlags.push_back(queueFlags [index]);
			prios [index] = std::numeric_limits<int>::max();
		}

		// create requested queues and check if more requested queues are supported
		// herefore: create vector that updates available queues in each queue family
		// structure: [qFamily_0, ..., qFamily_n] where
		// - qFamily_i = [GraphicsCount, ComputeCount, TransferCount], 0 <= i <= n
		std::vector<std::vector<int>> queueFamilyStatus, initialQueueFamilyStatus;

		for (auto qFamily : qFamilyProperties) {
			auto graphicsCount =
				static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eGraphics) != 0 ?
					qFamily.queueCount :
					0;
			auto computeCount =
				static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eCompute) != 0 ?
					qFamily.queueCount :
					0;
			auto transferCount =
				static_cast<uint32_t>(qFamily.queueFlags & vk::QueueFlagBits::eTransfer) != 0 ?
					qFamily.queueCount :
					0;
			queueFamilyStatus.push_back({ static_cast<int>(graphicsCount),
										  static_cast<int>(computeCount),
										  static_cast<int>(transferCount) });
		}

		initialQueueFamilyStatus = queueFamilyStatus;
		// check if every queue with the specified queue flag can be created
		// this automatically checks for queue flag support!
		for (auto qFlag : newFlags) {
			bool found;
			switch (qFlag) {
			case vk::QueueFlagBits::eGraphics:
				found = false;
				for (size_t i = 0; i < queueFamilyStatus.size() && !found; i++) {
					if (queueFamilyStatus [i][0] > 0) {
						queuePairsGraphics.emplace_back(std::pair(
							i, initialQueueFamilyStatus [i][0] - queueFamilyStatus [i][0]));
						queueFamilyStatus [i][0]--;
						queueFamilyStatus [i][1]--;
						queueFamilyStatus [i][2]--;
						found = true;
					}
				}
				if (!found) {
					for (size_t i = 0; i < queueFamilyStatus.size() && !found; i++) {
						if (initialQueueFamilyStatus [i][0] > 0) {
							queuePairsGraphics.emplace_back(std::pair(i, 0));
							found = true;
						}
					}

					vkcv_log(LogLevel::WARNING, "Not enough %s queues",
							 vk::to_string(qFlag).c_str());
				}
				break;
			case vk::QueueFlagBits::eCompute:
				found = false;
				for (size_t i = 0; i < queueFamilyStatus.size() && !found; i++) {
					if (queueFamilyStatus [i][1] > 0) {
						queuePairsCompute.emplace_back(std::pair(
							i, initialQueueFamilyStatus [i][1] - queueFamilyStatus [i][1]));
						queueFamilyStatus [i][0]--;
						queueFamilyStatus [i][1]--;
						queueFamilyStatus [i][2]--;
						found = true;
					}
				}
				if (!found) {
					for (size_t i = 0; i < queueFamilyStatus.size() && !found; i++) {
						if (initialQueueFamilyStatus [i][1] > 0) {
							queuePairsCompute.emplace_back(std::pair(i, 0));
							found = true;
						}
					}

					vkcv_log(LogLevel::WARNING, "Not enough %s queues",
							 vk::to_string(qFlag).c_str());
				}
				break;
			case vk::QueueFlagBits::eTransfer:
				found = false;
				for (size_t i = 0; i < queueFamilyStatus.size() && !found; i++) {
					if (queueFamilyStatus [i][2] > 0) {
						queuePairsTransfer.emplace_back(std::pair(
							i, initialQueueFamilyStatus [i][2] - queueFamilyStatus [i][2]));
						queueFamilyStatus [i][0]--;
						queueFamilyStatus [i][1]--;
						queueFamilyStatus [i][2]--;
						found = true;
					}
				}
				if (!found) {
					for (size_t i = 0; i < queueFamilyStatus.size() && !found; i++) {
						if (initialQueueFamilyStatus [i][2] > 0) {
							queuePairsTransfer.emplace_back(std::pair(i, 0));
							found = true;
						}
					}

					vkcv_log(LogLevel::WARNING, "Not enough %s queues",
							 vk::to_string(qFlag).c_str());
				}
				break;
			default:
				vkcv_log(LogLevel::ERROR, "Invalid input for queue flag bits: %s",
						 vk::to_string(qFlag).c_str());
				break;
			}
		}

		// create all requested queues
		for (size_t i = 0; i < qFamilyProperties.size(); i++) {
			uint32_t create = std::abs(initialQueueFamilyStatus [i][0] - queueFamilyStatus [i][0]);
			if (create > 0) {
				vk::DeviceQueueCreateInfo qCreateInfo(vk::DeviceQueueCreateFlags(), i, create,
													  queuePriorities.data());
				queueCreateInfos.push_back(qCreateInfo);
			}
		}
	}

	Context Context::create(const std::string &applicationName, uint32_t applicationVersion,
							const std::vector<vk::QueueFlagBits> &queueFlags,
							const Features &features,
							const std::vector<const char*> &instanceExtensions) {
		// check for layer support

		const std::vector<vk::LayerProperties> &layerProperties =
			vk::enumerateInstanceLayerProperties();

		std::vector<const char*> supportedLayers;
		supportedLayers.reserve(layerProperties.size());

		for (auto &elem : layerProperties) {
			supportedLayers.push_back(elem.layerName);
		}

// if in debug mode, check if validation layers are supported. Enable them if supported
#ifndef NDEBUG
		std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

		if (!checkSupport(supportedLayers, validationLayers)) {
			vkcv_log_throw_error("Validation layers requested but not available!");
		}
#endif

		// check for instance extension support
		std::vector<vk::ExtensionProperties> instanceExtensionProperties =
			vk::enumerateInstanceExtensionProperties();

		std::vector<const char*> supportedExtensions;
		supportedExtensions.reserve(instanceExtensionProperties.size());

		for (auto &elem : instanceExtensionProperties) {
			supportedExtensions.push_back(elem.extensionName);
		}

		// for GLFW: get all required extensions
		auto requiredStrings = getRequiredExtensions();
		std::vector<const char*> requiredExtensions;

		for (const auto &extension : requiredStrings) {
			requiredExtensions.push_back(extension.c_str());
		}

		requiredExtensions.insert(requiredExtensions.end(), instanceExtensions.begin(),
								  instanceExtensions.end());

		if (!checkSupport(supportedExtensions, requiredExtensions)) {
			vkcv_log_throw_error("The requested instance extensions are not supported!");
		}

		const vk::ApplicationInfo applicationInfo(applicationName.c_str(), applicationVersion,
												  VKCV_FRAMEWORK_NAME, VKCV_FRAMEWORK_VERSION,
												  VK_HEADER_VERSION_COMPLETE);

		vk::InstanceCreateInfo instanceCreateInfo(
			vk::InstanceCreateFlags(), &applicationInfo, 0, nullptr,
			static_cast<uint32_t>(requiredExtensions.size()), requiredExtensions.data());

#ifndef NDEBUG
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif

		vk::Instance instance = vk::createInstance(instanceCreateInfo);

		std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
		vk::PhysicalDevice physicalDevice;

		if (!pickPhysicalDevice(instance, physicalDevice)) {
			vkcv_log_throw_error("Picking suitable GPU as physical device failed!");
		}

		FeatureManager featureManager(physicalDevice);

#ifdef __APPLE__
		featureManager.useExtension("VK_KHR_portability_subset", true);
#endif

		if (featureManager.useExtension(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME, false)) {
			featureManager.useFeatures<vk::PhysicalDeviceShaderFloat16Int8Features>(
				[](vk::PhysicalDeviceShaderFloat16Int8Features &features) {
					features.setShaderFloat16(true);
				},
				false);
		}

		if (featureManager.useExtension(VK_KHR_16BIT_STORAGE_EXTENSION_NAME, false)) {
			featureManager.useFeatures<vk::PhysicalDevice16BitStorageFeatures>(
				[](vk::PhysicalDevice16BitStorageFeatures &features) {
					features.setStorageBuffer16BitAccess(true);
				},
				false);
		}

		featureManager.useFeatures([](vk::PhysicalDeviceFeatures &features) {
			features.setFragmentStoresAndAtomics(true);
			features.setGeometryShader(true);
			features.setDepthClamp(true);
			features.setShaderInt16(true);
		});

		for (const auto &feature : features.getList()) {
			feature(featureManager);
		}

		const auto &extensions = featureManager.getActiveExtensions();

		std::vector<vk::DeviceQueueCreateInfo> qCreateInfos;
		std::vector<float> qPriorities;
		qPriorities.resize(queueFlags.size(), 1.f);
		std::vector<std::pair<int, int>> queuePairsGraphics, queuePairsCompute, queuePairsTransfer;

		queueCreateInfosQueueHandles(physicalDevice, qPriorities, queueFlags, qCreateInfos,
									 queuePairsGraphics, queuePairsCompute, queuePairsTransfer);

		vk::DeviceCreateInfo deviceCreateInfo(
			vk::DeviceCreateFlags(), qCreateInfos.size(), qCreateInfos.data(), 0, nullptr,
			extensions.size(), extensions.data(), nullptr, &(featureManager.getFeatures()));

#ifndef NDEBUG
		deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
#endif

		vk::Device device = physicalDevice.createDevice(deviceCreateInfo);

		QueueManager queueManager =
			QueueManager::create(device, queuePairsGraphics, queuePairsCompute, queuePairsTransfer);

		vma::AllocatorCreateFlags vmaFlags;
		const vma::AllocatorCreateInfo allocatorCreateInfo(vmaFlags, physicalDevice, device, 0,
														   nullptr, nullptr, nullptr, nullptr,
														   instance, VK_HEADER_VERSION_COMPLETE);

		vma::Allocator allocator = vma::createAllocator(allocatorCreateInfo);

		return Context(instance, physicalDevice, device, std::move(featureManager),
					   std::move(queueManager), std::move(allocator));
	}

} // namespace vkcv
