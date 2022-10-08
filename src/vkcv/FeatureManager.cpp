
#include "vkcv/FeatureManager.hpp"

#include <cstddef>
#include <cstring>

#ifdef _MSVC_LANG
#include <type_traits>
#endif

namespace vkcv {

#ifdef _MSVC_LANG
#define typeof(var) std::decay<decltype((var))>::type
#endif

#define vkcv_check_init_features2(type) \
	type supported;                     \
	vk::PhysicalDeviceFeatures2 query;  \
	query.setPNext(&supported);         \
	m_physicalDevice.getFeatures2(&query)

#define vkcv_check_feature(attribute)                                                         \
	{                                                                                         \
		const char* f = reinterpret_cast<const char*>(&(features));                           \
		const char* s = reinterpret_cast<const char*>(&(supported));                          \
		const vk::Bool32* fb =                                                                \
			reinterpret_cast<const vk::Bool32*>(f + offsetof(typeof((features)), attribute)); \
		const vk::Bool32* sb =                                                                \
			reinterpret_cast<const vk::Bool32*>(s + offsetof(typeof((features)), attribute)); \
		if ((*fb) && (!*sb)) {                                                                \
			vkcv_log(((required) ? LogLevel::ERROR : LogLevel::WARNING),                      \
					 "Feature '" #attribute "' is not supported");                            \
			return false;                                                                     \
		}                                                                                     \
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceFeatures &features,
									  bool required) const {
		const auto &supported = m_physicalDevice.getFeatures();

		vkcv_check_feature(alphaToOne);
		vkcv_check_feature(depthBiasClamp);
		vkcv_check_feature(depthBounds);
		vkcv_check_feature(depthClamp);
		vkcv_check_feature(drawIndirectFirstInstance);
		vkcv_check_feature(dualSrcBlend);
		vkcv_check_feature(fillModeNonSolid);
		vkcv_check_feature(fragmentStoresAndAtomics);
		vkcv_check_feature(fullDrawIndexUint32);
		vkcv_check_feature(geometryShader);
		vkcv_check_feature(imageCubeArray);
		vkcv_check_feature(independentBlend);
		vkcv_check_feature(inheritedQueries);
		vkcv_check_feature(largePoints);
		vkcv_check_feature(logicOp);
		vkcv_check_feature(multiDrawIndirect);
		vkcv_check_feature(multiViewport);
		vkcv_check_feature(occlusionQueryPrecise);
		vkcv_check_feature(pipelineStatisticsQuery);
		vkcv_check_feature(robustBufferAccess);
		vkcv_check_feature(sampleRateShading);
		vkcv_check_feature(samplerAnisotropy);
		vkcv_check_feature(shaderClipDistance);
		vkcv_check_feature(shaderCullDistance);
		vkcv_check_feature(shaderFloat64);
		vkcv_check_feature(shaderImageGatherExtended);
		vkcv_check_feature(shaderInt16);
		vkcv_check_feature(shaderInt64);
		vkcv_check_feature(shaderResourceMinLod);
		vkcv_check_feature(shaderResourceResidency);
		vkcv_check_feature(shaderSampledImageArrayDynamicIndexing);
		vkcv_check_feature(shaderStorageBufferArrayDynamicIndexing);
		vkcv_check_feature(shaderStorageImageArrayDynamicIndexing);
		vkcv_check_feature(shaderStorageImageExtendedFormats);
		vkcv_check_feature(shaderStorageImageMultisample);
		vkcv_check_feature(shaderStorageImageReadWithoutFormat);
		vkcv_check_feature(shaderStorageImageWriteWithoutFormat);
		vkcv_check_feature(shaderTessellationAndGeometryPointSize);
		vkcv_check_feature(shaderUniformBufferArrayDynamicIndexing);
		vkcv_check_feature(sparseBinding);
		vkcv_check_feature(sparseResidency2Samples);
		vkcv_check_feature(sparseResidency4Samples);
		vkcv_check_feature(sparseResidency8Samples);
		vkcv_check_feature(sparseResidency16Samples);
		vkcv_check_feature(sparseResidencyAliased);
		vkcv_check_feature(sparseResidencyBuffer);
		vkcv_check_feature(sparseResidencyImage2D);
		vkcv_check_feature(sparseResidencyImage3D);
		vkcv_check_feature(tessellationShader);
		vkcv_check_feature(textureCompressionASTC_LDR);
		vkcv_check_feature(textureCompressionBC);
		vkcv_check_feature(textureCompressionETC2);
		vkcv_check_feature(variableMultisampleRate);
		vkcv_check_feature(vertexPipelineStoresAndAtomics);
		vkcv_check_feature(wideLines);
		vkcv_check_feature(multiDrawIndirect);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDevice16BitStorageFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDevice16BitStorageFeatures);

		vkcv_check_feature(storageBuffer16BitAccess);
		vkcv_check_feature(storageInputOutput16);
		vkcv_check_feature(storagePushConstant16);
		vkcv_check_feature(uniformAndStorageBuffer16BitAccess);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDevice8BitStorageFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDevice8BitStorageFeatures);

		vkcv_check_feature(storageBuffer8BitAccess);
		vkcv_check_feature(storagePushConstant8);
		vkcv_check_feature(uniformAndStorageBuffer8BitAccess);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceBufferDeviceAddressFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceBufferDeviceAddressFeatures);

		vkcv_check_feature(bufferDeviceAddress);
		vkcv_check_feature(bufferDeviceAddressCaptureReplay);
		vkcv_check_feature(bufferDeviceAddressMultiDevice);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceDescriptorIndexingFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceDescriptorIndexingFeatures);

		vkcv_check_feature(shaderInputAttachmentArrayDynamicIndexing);
		vkcv_check_feature(shaderInputAttachmentArrayNonUniformIndexing);
		vkcv_check_feature(shaderSampledImageArrayNonUniformIndexing);
		vkcv_check_feature(shaderStorageBufferArrayNonUniformIndexing);
		vkcv_check_feature(shaderStorageImageArrayNonUniformIndexing);
		vkcv_check_feature(shaderStorageTexelBufferArrayDynamicIndexing);
		vkcv_check_feature(shaderStorageTexelBufferArrayNonUniformIndexing);
		vkcv_check_feature(shaderUniformBufferArrayNonUniformIndexing);
		vkcv_check_feature(shaderUniformTexelBufferArrayDynamicIndexing);
		vkcv_check_feature(shaderUniformTexelBufferArrayNonUniformIndexing);
		vkcv_check_feature(descriptorBindingPartiallyBound);
		vkcv_check_feature(descriptorBindingSampledImageUpdateAfterBind);
		vkcv_check_feature(descriptorBindingStorageBufferUpdateAfterBind);
		vkcv_check_feature(descriptorBindingStorageImageUpdateAfterBind);
		vkcv_check_feature(descriptorBindingStorageTexelBufferUpdateAfterBind);
		vkcv_check_feature(descriptorBindingUniformBufferUpdateAfterBind);
		vkcv_check_feature(descriptorBindingUniformTexelBufferUpdateAfterBind);
		vkcv_check_feature(descriptorBindingUpdateUnusedWhilePending);
		vkcv_check_feature(descriptorBindingVariableDescriptorCount);
		vkcv_check_feature(runtimeDescriptorArray);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceHostQueryResetFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceHostQueryResetFeatures);

		vkcv_check_feature(hostQueryReset);

		return true;
	}

	bool
	FeatureManager::checkSupport(const vk::PhysicalDeviceImagelessFramebufferFeatures &features,
								 bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceImagelessFramebufferFeatures);

		vkcv_check_feature(imagelessFramebuffer);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceMultiviewFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceMultiviewFeatures);

		vkcv_check_feature(multiview);
		vkcv_check_feature(multiviewGeometryShader);
		vkcv_check_feature(multiviewTessellationShader);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceProtectedMemoryFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceProtectedMemoryFeatures);

		vkcv_check_feature(protectedMemory);

		return true;
	}

	bool
	FeatureManager::checkSupport(const vk::PhysicalDeviceSamplerYcbcrConversionFeatures &features,
								 bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceSamplerYcbcrConversionFeatures);

		vkcv_check_feature(samplerYcbcrConversion);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceScalarBlockLayoutFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceScalarBlockLayoutFeatures);

		vkcv_check_feature(scalarBlockLayout);

		return true;
	}

	bool FeatureManager::checkSupport(
		const vk::PhysicalDeviceSeparateDepthStencilLayoutsFeatures &features,
		bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceSeparateDepthStencilLayoutsFeatures);

		vkcv_check_feature(separateDepthStencilLayouts);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceShaderAtomicInt64Features &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceShaderAtomicInt64Features);

		vkcv_check_feature(shaderBufferInt64Atomics);
		vkcv_check_feature(shaderSharedInt64Atomics);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceShaderFloat16Int8Features &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceShaderFloat16Int8Features);

		vkcv_check_feature(shaderFloat16);
		vkcv_check_feature(shaderInt8);

		return true;
	}

	bool FeatureManager::checkSupport(
		const vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures &features,
		bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures);

		vkcv_check_feature(shaderSubgroupExtendedTypes);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceTimelineSemaphoreFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceTimelineSemaphoreFeatures);

		vkcv_check_feature(timelineSemaphore);

		return true;
	}

	bool FeatureManager::checkSupport(
		const vk::PhysicalDeviceUniformBufferStandardLayoutFeatures &features,
		bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceUniformBufferStandardLayoutFeatures);

		vkcv_check_feature(uniformBufferStandardLayout);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceVariablePointersFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceVariablePointersFeatures);

		vkcv_check_feature(variablePointers);
		vkcv_check_feature(variablePointersStorageBuffer);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceVulkanMemoryModelFeatures &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceVulkanMemoryModelFeatures);

		vkcv_check_feature(vulkanMemoryModel);
		vkcv_check_feature(vulkanMemoryModelDeviceScope);
		vkcv_check_feature(vulkanMemoryModelAvailabilityVisibilityChains);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceMeshShaderFeaturesNV &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceMeshShaderFeaturesNV);

		vkcv_check_feature(taskShader);
		vkcv_check_feature(meshShader);

		return true;
	}

	bool
	FeatureManager::checkSupport(const vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT &features,
								 bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT);

		vkcv_check_feature(shaderBufferFloat32Atomics);
		vkcv_check_feature(shaderBufferFloat32AtomicAdd);
		vkcv_check_feature(shaderBufferFloat64Atomics);
		vkcv_check_feature(shaderBufferFloat64AtomicAdd);
		vkcv_check_feature(shaderSharedFloat32Atomics);
		vkcv_check_feature(shaderSharedFloat32AtomicAdd);
		vkcv_check_feature(shaderSharedFloat64Atomics);
		vkcv_check_feature(shaderSharedFloat64AtomicAdd);
		vkcv_check_feature(shaderImageFloat32Atomics);
		vkcv_check_feature(shaderImageFloat32AtomicAdd);
		vkcv_check_feature(sparseImageFloat32Atomics);
		vkcv_check_feature(sparseImageFloat32AtomicAdd);

		return true;
	}

	bool
	FeatureManager::checkSupport(const vk::PhysicalDeviceShaderAtomicFloat2FeaturesEXT &features,
								 bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceShaderAtomicFloat2FeaturesEXT);

		vkcv_check_feature(shaderBufferFloat16Atomics);
		vkcv_check_feature(shaderBufferFloat16AtomicAdd);
		vkcv_check_feature(shaderBufferFloat16AtomicMinMax);
		vkcv_check_feature(shaderBufferFloat32AtomicMinMax);
		vkcv_check_feature(shaderBufferFloat64AtomicMinMax);
		vkcv_check_feature(shaderSharedFloat16Atomics);
		vkcv_check_feature(shaderSharedFloat16AtomicAdd);
		vkcv_check_feature(shaderSharedFloat16AtomicMinMax);
		vkcv_check_feature(shaderSharedFloat32AtomicMinMax);
		vkcv_check_feature(shaderSharedFloat64AtomicMinMax);
		vkcv_check_feature(shaderImageFloat32AtomicMinMax);
		vkcv_check_feature(sparseImageFloat32AtomicMinMax);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceVulkan12Features &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceVulkan12Features);

		vkcv_check_feature(samplerMirrorClampToEdge);
		vkcv_check_feature(drawIndirectCount);
		vkcv_check_feature(storageBuffer8BitAccess);
		vkcv_check_feature(uniformAndStorageBuffer8BitAccess);
		vkcv_check_feature(storagePushConstant8);
		vkcv_check_feature(shaderBufferInt64Atomics);
		vkcv_check_feature(shaderSharedInt64Atomics);
		vkcv_check_feature(shaderFloat16);
		vkcv_check_feature(shaderInt8);
		vkcv_check_feature(descriptorIndexing);
		vkcv_check_feature(shaderInputAttachmentArrayDynamicIndexing);
		vkcv_check_feature(shaderUniformTexelBufferArrayDynamicIndexing);
		vkcv_check_feature(shaderStorageTexelBufferArrayDynamicIndexing);
		vkcv_check_feature(shaderUniformBufferArrayNonUniformIndexing);
		vkcv_check_feature(shaderSampledImageArrayNonUniformIndexing);
		vkcv_check_feature(shaderStorageBufferArrayNonUniformIndexing);
		vkcv_check_feature(shaderStorageImageArrayNonUniformIndexing);
		vkcv_check_feature(shaderInputAttachmentArrayNonUniformIndexing);
		vkcv_check_feature(shaderUniformTexelBufferArrayNonUniformIndexing);
		vkcv_check_feature(shaderStorageTexelBufferArrayNonUniformIndexing);
		vkcv_check_feature(descriptorBindingUniformBufferUpdateAfterBind);
		vkcv_check_feature(descriptorBindingSampledImageUpdateAfterBind);
		vkcv_check_feature(descriptorBindingStorageImageUpdateAfterBind);
		vkcv_check_feature(descriptorBindingStorageBufferUpdateAfterBind);
		vkcv_check_feature(descriptorBindingUniformTexelBufferUpdateAfterBind);
		vkcv_check_feature(descriptorBindingStorageTexelBufferUpdateAfterBind);
		vkcv_check_feature(descriptorBindingUpdateUnusedWhilePending);
		vkcv_check_feature(descriptorBindingPartiallyBound);
		vkcv_check_feature(descriptorBindingVariableDescriptorCount);
		vkcv_check_feature(runtimeDescriptorArray);
		vkcv_check_feature(samplerFilterMinmax);
		vkcv_check_feature(scalarBlockLayout);
		vkcv_check_feature(imagelessFramebuffer);
		vkcv_check_feature(uniformBufferStandardLayout);
		vkcv_check_feature(shaderSubgroupExtendedTypes);
		vkcv_check_feature(separateDepthStencilLayouts);
		vkcv_check_feature(hostQueryReset);
		vkcv_check_feature(timelineSemaphore);
		vkcv_check_feature(bufferDeviceAddress);
		vkcv_check_feature(bufferDeviceAddressCaptureReplay);
		vkcv_check_feature(bufferDeviceAddressMultiDevice);
		vkcv_check_feature(vulkanMemoryModel);
		vkcv_check_feature(vulkanMemoryModelDeviceScope);
		vkcv_check_feature(vulkanMemoryModelAvailabilityVisibilityChains);
		vkcv_check_feature(shaderOutputViewportIndex);
		vkcv_check_feature(shaderOutputLayer);
		vkcv_check_feature(subgroupBroadcastDynamicId);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceVulkan11Features &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceVulkan11Features);

		vkcv_check_feature(multiview);
		vkcv_check_feature(multiviewGeometryShader);
		vkcv_check_feature(multiviewTessellationShader);
		vkcv_check_feature(protectedMemory);
		vkcv_check_feature(samplerYcbcrConversion);
		vkcv_check_feature(shaderDrawParameters);
		vkcv_check_feature(storageBuffer16BitAccess);
		vkcv_check_feature(storageInputOutput16);
		vkcv_check_feature(storagePushConstant16);
		vkcv_check_feature(uniformAndStorageBuffer16BitAccess);
		vkcv_check_feature(variablePointers);
		vkcv_check_feature(variablePointersStorageBuffer);

		return true;
	}

	bool
	FeatureManager::checkSupport(const vk::PhysicalDeviceAccelerationStructureFeaturesKHR &features,
								 bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceAccelerationStructureFeaturesKHR);

		vkcv_check_feature(accelerationStructure);
		vkcv_check_feature(accelerationStructureCaptureReplay);
		vkcv_check_feature(accelerationStructureIndirectBuild);
		vkcv_check_feature(accelerationStructureHostCommands);
		vkcv_check_feature(descriptorBindingAccelerationStructureUpdateAfterBind);

		return true;
	}

	bool
	FeatureManager::checkSupport(const vk::PhysicalDeviceRayTracingPipelineFeaturesKHR &features,
								 bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceRayTracingPipelineFeaturesKHR);

		vkcv_check_feature(rayTracingPipeline);
		vkcv_check_feature(rayTracingPipelineShaderGroupHandleCaptureReplay);
		vkcv_check_feature(rayTracingPipelineShaderGroupHandleCaptureReplayMixed);
		vkcv_check_feature(rayTracingPipelineTraceRaysIndirect);
		vkcv_check_feature(rayTraversalPrimitiveCulling);

		return true;
	}

	bool FeatureManager::checkSupport(const vk::PhysicalDeviceVulkan13Features &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceVulkan13Features);

		vkcv_check_feature(robustImageAccess);
		vkcv_check_feature(inlineUniformBlock);
		vkcv_check_feature(descriptorBindingInlineUniformBlockUpdateAfterBind);
		vkcv_check_feature(pipelineCreationCacheControl);
		vkcv_check_feature(privateData);
		vkcv_check_feature(shaderDemoteToHelperInvocation);
		vkcv_check_feature(shaderTerminateInvocation);
		vkcv_check_feature(subgroupSizeControl);
		vkcv_check_feature(computeFullSubgroups);
		vkcv_check_feature(synchronization2);
		vkcv_check_feature(textureCompressionASTC_HDR);
		vkcv_check_feature(shaderZeroInitializeWorkgroupMemory);
		vkcv_check_feature(dynamicRendering);
		vkcv_check_feature(shaderIntegerDotProduct);
		vkcv_check_feature(maintenance4);

		return true;
	}
	
	bool FeatureManager::checkSupport(const vk::PhysicalDeviceIndexTypeUint8FeaturesEXT &features,
									  bool required) const {
		vkcv_check_init_features2(vk::PhysicalDeviceIndexTypeUint8FeaturesEXT);
		
		vkcv_check_feature(indexTypeUint8);
		
		return true;
	}

	vk::BaseOutStructure* FeatureManager::findFeatureStructure(vk::StructureType type) const {
		for (auto &base : m_featuresExtensions) {
			if (base->sType == type) {
				return base;
			}
		}

		return nullptr;
	}

	const char* strclone(const char* str) {
		if (!str) {
			return nullptr;
		}

		const size_t length = strlen(str) + 1;

		if (length <= 1) {
			return nullptr;
		}

		char* clone = new char [length];
		strcpy(clone, str);
		return clone;
	}

	FeatureManager::FeatureManager(vk::PhysicalDevice &physicalDevice) :
		m_physicalDevice(physicalDevice), m_supportedExtensions(), m_activeExtensions(),
		m_featuresBase(), m_featuresExtensions() {
		for (const auto &extension : m_physicalDevice.enumerateDeviceExtensionProperties()) {
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
		for (auto &features : m_featuresExtensions) {
			delete features;
		}

		for (auto &extension : m_activeExtensions) {
			delete [] extension;
		}

		for (auto &extension : m_supportedExtensions) {
			delete [] extension;
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

	bool FeatureManager::isExtensionSupported(const std::string &extension) const {
		for (const auto &supported : m_supportedExtensions) {
			if (0 == strcmp(supported, extension.c_str())) {
				return true;
			}
		}

		return false;
	}

	bool FeatureManager::useExtension(const std::string &extension, bool required) {
		const char* clone = strclone(extension.c_str());

		if (!clone) {
			vkcv_log(LogLevel::WARNING, "Extension '%s' is not valid", extension.c_str());
			return false;
		}

		if (!isExtensionSupported(extension)) {
			vkcv_log((required ? LogLevel::ERROR : LogLevel::WARNING),
					 "Extension '%s' is not supported", extension.c_str());

			delete [] clone;
			if (required) {
				vkcv_log_throw_error("Required extension is not supported!");
			}

			return false;
		}

		m_activeExtensions.push_back(clone);
		return true;
	}

	bool FeatureManager::isExtensionActive(const std::string &extension) const {
		for (const auto &supported : m_activeExtensions) {
			if (0 == strcmp(supported, extension.c_str())) {
				return true;
			}
		}

		return false;
	}

	const std::vector<const char*> &FeatureManager::getActiveExtensions() const {
		return m_activeExtensions;
	}

	bool FeatureManager::useFeatures(
		const std::function<void(vk::PhysicalDeviceFeatures &)> &featureFunction, bool required) {
		vk::PhysicalDeviceFeatures features = m_featuresBase.features;

		featureFunction(features);

		if (!checkSupport(features, required)) {
			return false;
		}

		m_featuresBase.features = features;
		return true;
	}

	const vk::PhysicalDeviceFeatures2 &FeatureManager::getFeatures() const {
		return m_featuresBase;
	}

} // namespace vkcv
