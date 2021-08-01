
#include "vkcv/FeatureManager.hpp"

#include <string.h>

namespace vkcv {
	
	bool FeatureManager::checkSupport(const vk::PhysicalDevice16BitStorageFeatures &features, bool required) const {
		vk::PhysicalDevice16BitStorageFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.storageBuffer16BitAccess) && (!supported.storageBuffer16BitAccess)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'storageBuffer16BitAccess' is not supported");
			return false;
		}
		
		if ((features.storageInputOutput16) && (!supported.storageInputOutput16)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'storageInputOutput16' is not supported");
			return false;
		}
		
		if ((features.storagePushConstant16) && (!supported.storagePushConstant16)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'storagePushConstant16' is not supported");
			return false;
		}
		
		if ((features.uniformAndStorageBuffer16BitAccess) && (!supported.uniformAndStorageBuffer16BitAccess)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'uniformAndStorageBuffer16BitAccess' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDevice8BitStorageFeatures &features, bool required) const {
		vk::PhysicalDevice8BitStorageFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.storageBuffer8BitAccess) && (!supported.storageBuffer8BitAccess)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'storageBuffer8BitAccess' is not supported");
			return false;
		}
		
		if ((features.storagePushConstant8) && (!supported.storagePushConstant8)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'storagePushConstant8' is not supported");
			return false;
		}
		
		if ((features.uniformAndStorageBuffer8BitAccess) && (!supported.uniformAndStorageBuffer8BitAccess)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'uniformAndStorageBuffer8BitAccess' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceBufferDeviceAddressFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceBufferDeviceAddressFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.bufferDeviceAddress) && (!supported.bufferDeviceAddress)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'bufferDeviceAddress' is not supported");
			return false;
		}
		
		if ((features.bufferDeviceAddressCaptureReplay) && (!supported.bufferDeviceAddressCaptureReplay)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'bufferDeviceAddressCaptureReplay' is not supported");
			return false;
		}
		
		if ((features.bufferDeviceAddressMultiDevice) && (!supported.bufferDeviceAddressMultiDevice)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'bufferDeviceAddressMultiDevice' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceDescriptorIndexingFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceDescriptorIndexingFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.shaderInputAttachmentArrayDynamicIndexing) &&
			(!supported.shaderInputAttachmentArrayDynamicIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderInputAttachmentArrayDynamicIndexing' is not supported");
			return false;
		}
		
		if ((features.shaderInputAttachmentArrayNonUniformIndexing) &&
			(!supported.shaderInputAttachmentArrayNonUniformIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderInputAttachmentArrayNonUniformIndexing' is not supported");
			return false;
		}
		
		if ((features.shaderSampledImageArrayNonUniformIndexing) &&
			(!supported.shaderSampledImageArrayNonUniformIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderSampledImageArrayNonUniformIndexing' is not supported");
			return false;
		}
		
		if ((features.shaderStorageBufferArrayNonUniformIndexing) &&
			(!supported.shaderStorageBufferArrayNonUniformIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderStorageBufferArrayNonUniformIndexing' is not supported");
			return false;
		}
		
		if ((features.shaderStorageImageArrayNonUniformIndexing) &&
			(!supported.shaderStorageImageArrayNonUniformIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderStorageImageArrayNonUniformIndexing' is not supported");
			return false;
		}
		
		if ((features.shaderStorageTexelBufferArrayDynamicIndexing) &&
			(!supported.shaderStorageTexelBufferArrayDynamicIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderStorageTexelBufferArrayDynamicIndexing' is not supported");
			return false;
		}
		
		if ((features.shaderStorageTexelBufferArrayNonUniformIndexing) &&
			(!supported.shaderStorageTexelBufferArrayNonUniformIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderStorageTexelBufferArrayNonUniformIndexing' is not supported");
			return false;
		}
		
		if ((features.shaderUniformBufferArrayNonUniformIndexing) &&
			(!supported.shaderUniformBufferArrayNonUniformIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderUniformBufferArrayNonUniformIndexing' is not supported");
			return false;
		}
		
		if ((features.shaderUniformTexelBufferArrayDynamicIndexing) &&
			(!supported.shaderUniformTexelBufferArrayDynamicIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderUniformTexelBufferArrayDynamicIndexing' is not supported");
			return false;
		}
		
		if ((features.shaderUniformTexelBufferArrayNonUniformIndexing) &&
			(!supported.shaderUniformTexelBufferArrayNonUniformIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderUniformTexelBufferArrayNonUniformIndexing' is not supported");
			return false;
		}
		
		if ((features.descriptorBindingPartiallyBound) && (!supported.descriptorBindingPartiallyBound)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'descriptorBindingPartiallyBound' is not supported");
			return false;
		}
		
		
		if ((features.descriptorBindingSampledImageUpdateAfterBind) &&
			(!supported.descriptorBindingSampledImageUpdateAfterBind)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'descriptorBindingSampledImageUpdateAfterBind' is not supported");
			return false;
		}
		
		if ((features.descriptorBindingStorageBufferUpdateAfterBind) &&
			(!supported.descriptorBindingStorageBufferUpdateAfterBind)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'descriptorBindingStorageBufferUpdateAfterBind' is not supported");
			return false;
		}
		
		if ((features.descriptorBindingStorageImageUpdateAfterBind) &&
			(!supported.descriptorBindingStorageImageUpdateAfterBind)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'descriptorBindingStorageImageUpdateAfterBind' is not supported");
			return false;
		}
		
		if ((features.descriptorBindingStorageTexelBufferUpdateAfterBind) &&
			(!supported.descriptorBindingStorageTexelBufferUpdateAfterBind)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'descriptorBindingStorageTexelBufferUpdateAfterBind' is not supported");
			return false;
		}
		
		if ((features.descriptorBindingUniformBufferUpdateAfterBind) &&
			(!supported.descriptorBindingUniformBufferUpdateAfterBind)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'descriptorBindingUniformBufferUpdateAfterBind' is not supported");
			return false;
		}
		
		if ((features.descriptorBindingUniformTexelBufferUpdateAfterBind) &&
			(!supported.descriptorBindingUniformTexelBufferUpdateAfterBind)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'descriptorBindingUniformTexelBufferUpdateAfterBind' is not supported");
			return false;
		}
		
		if ((features.descriptorBindingUpdateUnusedWhilePending) &&
			(!supported.descriptorBindingUpdateUnusedWhilePending)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'descriptorBindingUpdateUnusedWhilePending' is not supported");
			return false;
		}
		
		if ((features.descriptorBindingVariableDescriptorCount) &&
			(!supported.descriptorBindingVariableDescriptorCount)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'descriptorBindingVariableDescriptorCount' is not supported");
			return false;
		}
		
		if ((features.runtimeDescriptorArray) && (!supported.runtimeDescriptorArray)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'runtimeDescriptorArray' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceHostQueryResetFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceHostQueryResetFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.hostQueryReset) && (!supported.hostQueryReset)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'hostQueryReset' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceImagelessFramebufferFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceImagelessFramebufferFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.imagelessFramebuffer) && (!supported.imagelessFramebuffer)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'imagelessFramebuffer' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceMultiviewFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceMultiviewFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.multiview) && (!supported.multiview)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'multiview' is not supported");
			return false;
		}
		
		if ((features.multiviewGeometryShader) && (!supported.multiviewGeometryShader)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'multiviewGeometryShader' is not supported");
			return false;
		}
		
		if ((features.multiviewTessellationShader) && (!supported.multiviewTessellationShader)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'multiviewTessellationShader' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceProtectedMemoryFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceProtectedMemoryFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.protectedMemory) && (!supported.protectedMemory)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'protectedMemory' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceSamplerYcbcrConversionFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceSamplerYcbcrConversionFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.samplerYcbcrConversion) && (!supported.samplerYcbcrConversion)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'samplerYcbcrConversion' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceScalarBlockLayoutFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceScalarBlockLayoutFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.scalarBlockLayout) && (!supported.scalarBlockLayout)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'scalarBlockLayout' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceSeparateDepthStencilLayoutsFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceSeparateDepthStencilLayoutsFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.separateDepthStencilLayouts) && (!supported.separateDepthStencilLayouts)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'separateDepthStencilLayouts' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceShaderAtomicInt64Features &features,
									  bool required) const {
		vk::PhysicalDeviceShaderAtomicInt64Features supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.shaderBufferInt64Atomics) && (!supported.shaderBufferInt64Atomics)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderBufferInt64Atomics' is not supported");
			return false;
		}
		
		if ((features.shaderSharedInt64Atomics) && (!supported.shaderSharedInt64Atomics)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderSharedInt64Atomics' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceShaderFloat16Int8Features &features, bool required) const {
		vk::PhysicalDeviceShaderFloat16Int8Features supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.shaderFloat16) && (!supported.shaderFloat16)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderFloat16' is not supported");
			return false;
		}
		
		if ((features.shaderInt8) && (!supported.shaderInt8)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderInt8' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.shaderSubgroupExtendedTypes) && (!supported.shaderSubgroupExtendedTypes)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderSubgroupExtendedTypes' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceTimelineSemaphoreFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceTimelineSemaphoreFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.timelineSemaphore) && (!supported.timelineSemaphore)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'timelineSemaphore' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceUniformBufferStandardLayoutFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceUniformBufferStandardLayoutFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.uniformBufferStandardLayout) && (!supported.uniformBufferStandardLayout)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'uniformBufferStandardLayout' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceVariablePointersFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceVariablePointersFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.variablePointers) && (!supported.variablePointers)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'variablePointers' is not supported");
			return false;
		}
		
		if ((features.variablePointersStorageBuffer) && (!supported.variablePointersStorageBuffer)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'variablePointersStorageBuffer' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceVulkanMemoryModelFeatures &features,
									  bool required) const {
		vk::PhysicalDeviceVulkanMemoryModelFeatures supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.vulkanMemoryModel) && (!supported.vulkanMemoryModel)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'vulkanMemoryModel' is not supported");
			return false;
		}
		
		if ((features.vulkanMemoryModelDeviceScope) && (!supported.vulkanMemoryModelDeviceScope)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'vulkanMemoryModelDeviceScope' is not supported");
			return false;
		}
		
		if ((features.vulkanMemoryModelAvailabilityVisibilityChains) &&
			(!supported.vulkanMemoryModelAvailabilityVisibilityChains)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'vulkanMemoryModelAvailabilityVisibilityChains' is not supported");
			return false;
		}
		
		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceMeshShaderFeaturesNV &features, bool required) const {
		vk::PhysicalDeviceMeshShaderFeaturesNV supported;
		vk::PhysicalDeviceFeatures2 query;
		query.setPNext(&supported);
		
		m_physicalDevice.getFeatures2(&query);
		
		if ((features.taskShader) && (!supported.taskShader)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'taskShader' is not supported");
			return false;
		}
		
		if ((features.meshShader) && (!supported.meshShader)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'meshShader' is not supported");
			return false;
		}
		
		return true;
	}
	
	const char* strclone(const char* str) {
		if (!str) {
			return nullptr;
		}
		
		const size_t length = strlen(str) + 1;
		
		if (length <= 1) {
			return nullptr;
		}
		
		char* clone = new char[length];
		strcpy(clone, str);
		return clone;
	}
	
	FeatureManager::FeatureManager(vk::PhysicalDevice &physicalDevice) :
	m_physicalDevice(physicalDevice),
	m_supportedExtensions(),
	m_activeExtensions(),
	m_featuresBase(),
	m_featuresExtensions() {
		for (const auto& extension : m_physicalDevice.enumerateDeviceExtensionProperties()) {
			const char* clone = strclone(extension.extensionName);
			
			if (clone) {
				m_supportedExtensions.push_back(clone);
			}
		}
	}
	
	FeatureManager::FeatureManager(FeatureManager &&other) noexcept :
	m_physicalDevice(other.m_physicalDevice),
	m_supportedExtensions(std::move(other.m_supportedExtensions)),
	m_activeExtensions(std::move(other.m_activeExtensions)),
    m_featuresBase(other.m_featuresBase),
    m_featuresExtensions(std::move(other.m_featuresExtensions)) {
		other.m_featuresExtensions.clear();
		other.m_activeExtensions.clear();
		other.m_supportedExtensions.clear();
	}
	
	FeatureManager::~FeatureManager() {
		for (auto& features : m_featuresExtensions) {
			delete features;
		}
		
		for (auto& extension : m_activeExtensions) {
			delete[] extension;
		}
		
		for (auto& extension : m_supportedExtensions) {
			delete[] extension;
		}
	}
	
	FeatureManager &FeatureManager::operator=(FeatureManager &&other) noexcept {
		m_physicalDevice = other.m_physicalDevice;
		m_supportedExtensions = std::move(other.m_supportedExtensions);
		m_activeExtensions = std::move(other.m_activeExtensions);
		m_featuresBase = other.m_featuresBase;
		m_featuresExtensions = std::move(other.m_featuresExtensions);
		
		other.m_featuresExtensions.clear();
		other.m_activeExtensions.clear();
		other.m_supportedExtensions.clear();
		
		return *this;
	}
	
	bool FeatureManager::isExtensionSupported(const char *extension) const {
		for (const auto& supported : m_supportedExtensions) {
			if (0 == strcmp(supported, extension)) {
				return true;
			}
		}
		
		return false;
	}
	
	bool FeatureManager::useExtension(const char *extension, bool required) {
		const char* clone = strclone(extension);
		
		if (!clone) {
			vkcv_log(LogLevel::WARNING, "Extension '%s' is not valid", extension);
			return false;
		}
		
		if (!isExtensionSupported(extension)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING), "Extension '%s' is not supported", extension);
			
			delete[] clone;
			return false;
		}
		
		m_activeExtensions.push_back(clone);
		return true;
	}
	
	bool FeatureManager::isExtensionActive(const char *extension) const {
		for (const auto& supported : m_activeExtensions) {
			if (0 == strcmp(supported, extension)) {
				return true;
			}
		}
		
		return false;
	}
	
	const std::vector<const char*>& FeatureManager::getActiveExtensions() const {
		return m_activeExtensions;
	}
	
	bool FeatureManager::useFeatures(const std::function<void(vk::PhysicalDeviceFeatures &)> &featureFunction,
									 bool required) {
		featureFunction(m_featuresBase.features);
		
		const auto& supported = m_physicalDevice.getFeatures();
		
		if ((m_featuresBase.features.alphaToOne) && (!supported.alphaToOne)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'alphaToOne' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.depthBiasClamp) && (!supported.depthBiasClamp)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'depthBiasClamp' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.depthBounds) && (!supported.depthBounds)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'depthBounds' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.depthClamp) && (!supported.depthClamp)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'depthClamp' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.drawIndirectFirstInstance) && (!supported.drawIndirectFirstInstance)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'drawIndirectFirstInstance' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.dualSrcBlend) && (!supported.dualSrcBlend)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'dualSrcBlend' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.fillModeNonSolid) && (!supported.fillModeNonSolid)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'fillModeNonSolid' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.fragmentStoresAndAtomics) && (!supported.fragmentStoresAndAtomics)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'fragmentStoresAndAtomics' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.fullDrawIndexUint32) && (!supported.fullDrawIndexUint32)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'fullDrawIndexUint32' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.geometryShader) && (!supported.geometryShader)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'geometryShader' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.imageCubeArray) && (!supported.imageCubeArray)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'imageCubeArray' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.independentBlend) && (!supported.independentBlend)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'independentBlend' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.inheritedQueries) && (!supported.inheritedQueries)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'inheritedQueries' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.largePoints) && (!supported.largePoints)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'largePoints' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.logicOp) && (!supported.logicOp)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'logicOp' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.multiDrawIndirect) && (!supported.multiDrawIndirect)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'multiDrawIndirect' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.multiViewport) && (!supported.multiViewport)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'multiViewport' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.occlusionQueryPrecise) && (!supported.occlusionQueryPrecise)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'occlusionQueryPrecise' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.pipelineStatisticsQuery) && (!supported.pipelineStatisticsQuery)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'pipelineStatisticsQuery' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.robustBufferAccess) && (!supported.robustBufferAccess)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'robustBufferAccess' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.sampleRateShading) && (!supported.sampleRateShading)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'sampleRateShading' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.samplerAnisotropy) && (!supported.samplerAnisotropy)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'samplerAnisotropy' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderClipDistance) && (!supported.shaderClipDistance)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderClipDistance' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderCullDistance) && (!supported.shaderCullDistance)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderCullDistance' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderFloat64) && (!supported.shaderFloat64)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderFloat64' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderImageGatherExtended) && (!supported.shaderImageGatherExtended)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderImageGatherExtended' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderInt16) && (!supported.shaderInt16)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderInt16' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderInt64) && (!supported.shaderInt64)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderInt64' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderResourceMinLod) && (!supported.shaderResourceMinLod)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderResourceMinLod' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderResourceResidency) && (!supported.shaderResourceResidency)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderResourceResidency' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderSampledImageArrayDynamicIndexing) &&
			(!supported.shaderSampledImageArrayDynamicIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderSampledImageArrayDynamicIndexing' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderStorageBufferArrayDynamicIndexing) &&
			(!supported.shaderStorageBufferArrayDynamicIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderStorageBufferArrayDynamicIndexing' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderStorageImageArrayDynamicIndexing) &&
			(!supported.shaderStorageImageArrayDynamicIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderStorageImageArrayDynamicIndexing' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderStorageImageExtendedFormats) &&
			(!supported.shaderStorageImageExtendedFormats)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderStorageImageExtendedFormats' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderStorageImageMultisample) &&
			(!supported.shaderStorageImageMultisample)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderStorageImageMultisample' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderStorageImageReadWithoutFormat) &&
			(!supported.shaderStorageImageReadWithoutFormat)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderStorageImageReadWithoutFormat' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderStorageImageWriteWithoutFormat) &&
			(!supported.shaderStorageImageWriteWithoutFormat)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderStorageImageWriteWithoutFormat' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderTessellationAndGeometryPointSize) &&
			(!supported.shaderTessellationAndGeometryPointSize)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderTessellationAndGeometryPointSize' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.shaderUniformBufferArrayDynamicIndexing) &&
			(!supported.shaderUniformBufferArrayDynamicIndexing)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'shaderUniformBufferArrayDynamicIndexing' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.sparseBinding) && (!supported.sparseBinding)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'sparseBinding' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.sparseResidency2Samples) && (!supported.sparseResidency2Samples)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'sparseResidency2Samples' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.sparseResidency4Samples) && (!supported.sparseResidency4Samples)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'sparseResidency4Samples' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.sparseResidency8Samples) && (!supported.sparseResidency8Samples)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'sparseResidency8Samples' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.sparseResidency16Samples) && (!supported.sparseResidency16Samples)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'sparseResidency16Samples' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.sparseResidencyAliased) && (!supported.sparseResidencyAliased)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'sparseResidencyAliased' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.sparseResidencyBuffer) && (!supported.sparseResidencyBuffer)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'sparseResidencyBuffer' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.sparseResidencyImage2D) && (!supported.sparseResidencyImage2D)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'sparseResidencyImage2D' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.sparseResidencyImage3D) && (!supported.sparseResidencyImage3D)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'sparseResidencyImage2D' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.tessellationShader) && (!supported.tessellationShader)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'tessellationShader' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.textureCompressionASTC_LDR) && (!supported.textureCompressionASTC_LDR)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'textureCompressionASTC_LDR' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.textureCompressionBC) && (!supported.textureCompressionBC)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'textureCompressionBC' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.textureCompressionETC2) && (!supported.textureCompressionETC2)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'textureCompressionETC2' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.variableMultisampleRate) && (!supported.variableMultisampleRate)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'variableMultisampleRate' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.vertexPipelineStoresAndAtomics) && (!supported.vertexPipelineStoresAndAtomics)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'vertexPipelineStoresAndAtomics' is not supported");
			return false;
		}
		
		if ((m_featuresBase.features.wideLines) && (!supported.wideLines)) {
			vkcv_log((required? LogLevel::ERROR : LogLevel::WARNING),
					 "Feature 'wideLines' is not supported");
			return false;
		}
		
		return true;
	}
	
	const vk::PhysicalDeviceFeatures2& FeatureManager::getFeatures() const {
		return m_featuresBase;
	}
	
}
