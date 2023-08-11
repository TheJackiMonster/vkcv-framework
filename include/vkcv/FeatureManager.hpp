#pragma once
/**
 * @authors Tobias Frisch, Artur Wasmut, Vanessa Karolek, Sebastian Gaida
 * @file vkcv/FeatureManager.hpp
 * @brief Class to manage feature support and extension usage.
 */

#include <functional>
#include <vulkan/vulkan.hpp>

#include "Container.hpp"
#include "Logger.hpp"

namespace vkcv {

	/**
	 * @brief Class to manage extension and feature requirements, support and usage.
	 */
	class FeatureManager {
	private:
		/**
		 * Physical device to check feature support against.
		 */
		vk::PhysicalDevice &m_physicalDevice;

		/**
		 * List of supported extensions.
		 */
		Vector<const char*> m_supportedExtensions;

		/**
		 * List of activated extensions for usage.
		 */
		Vector<const char*> m_activeExtensions;

		/**
		 * Feature structure chain to request activated features.
		 */
		vk::PhysicalDeviceFeatures2 m_featuresBase;

		/**
		 * List of base structures allocated to request extension specific features.
		 */
		Vector<vk::BaseOutStructure*> m_featuresExtensions;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceFeatures &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDevice16BitStorageFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDevice16BitStorageFeatures &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDevice8BitStorageFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDevice8BitStorageFeatures &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceBufferDeviceAddressFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceBufferDeviceAddressFeatures &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceDescriptorIndexingFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceDescriptorIndexingFeatures &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceHostQueryResetFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceHostQueryResetFeatures &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceImagelessFramebufferFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceImagelessFramebufferFeatures &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceMultiviewFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceMultiviewFeatures &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceProtectedMemoryFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceProtectedMemoryFeatures &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceSamplerYcbcrConversionFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceSamplerYcbcrConversionFeatures &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceScalarBlockLayoutFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceScalarBlockLayoutFeatures &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceSeparateDepthStencilLayoutsFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceSeparateDepthStencilLayoutsFeatures &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceShaderAtomicInt64Features.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceShaderAtomicInt64Features &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceShaderFloat16Int8Features.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceShaderFloat16Int8Features &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceTimelineSemaphoreFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceTimelineSemaphoreFeatures &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceUniformBufferStandardLayoutFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceUniformBufferStandardLayoutFeatures &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceVariablePointersFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceVariablePointersFeatures &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceVulkanMemoryModelFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceVulkanMemoryModelFeatures &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceMeshShaderFeaturesNV.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceMeshShaderFeaturesNV &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceShaderAtomicFloat2FeaturesEXT.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceShaderAtomicFloat2FeaturesEXT &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceVulkan12Features.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceVulkan12Features &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceVulkan11Features.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceVulkan11Features &features,
										bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceAccelerationStructureFeaturesKHR.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceAccelerationStructureFeaturesKHR &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceRayTracingPipelineFeaturesKHR.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool
		checkSupport(const vk::PhysicalDeviceRayTracingPipelineFeaturesKHR &features,
					 bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceVulkan13Features.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceVulkan13Features &features,
										bool required) const;
		
		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceCoherentMemoryFeaturesAMD.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceCoherentMemoryFeaturesAMD &features,
										bool required) const;
		
		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceSubgroupSizeControlFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceSubgroupSizeControlFeatures &features,
										bool required) const;
		
		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceIndexTypeUint8FeaturesEXT.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceIndexTypeUint8FeaturesEXT &features,
										bool required) const;
		
		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceShaderTerminateInvocationFeatures.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceShaderTerminateInvocationFeatures &features,
										bool required) const;
		
		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceMeshShaderFeaturesEXT.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceMeshShaderFeaturesEXT &features,
										bool required) const;
		
		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceHostImageCopyFeaturesEXT.
		 *
		 * @param[in] features The features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]] bool checkSupport(const vk::PhysicalDeviceHostImageCopyFeaturesEXT &features,
										bool required) const;

		/**
		 * @brief Searches for a base structure of a given structure type.
		 *
		 * @param[in] type Structure type
		 * @return Pointer to first matching base structure or nullptr
		 */
		[[nodiscard]] vk::BaseOutStructure* findFeatureStructure(vk::StructureType type) const;

	public:
		/**
		 * @brief Constructor of a feature manager with a given physical device.
		 *
		 * @param[in,out] physicalDevice Physical device
		 */
		explicit FeatureManager(vk::PhysicalDevice &physicalDevice);

		FeatureManager(const FeatureManager &other) = delete;

		/**
		 * @brief Move-constructor of a feature manager.
		 *
		 * @param[in,out] other Other feature manager instance
		 */
		FeatureManager(FeatureManager &&other) noexcept;

		/**
		 * @brief Destructor of a feature manager.
		 */
		~FeatureManager();

		FeatureManager &operator=(const FeatureManager &other) = delete;

		/**
		 * @brief Move-operator of a feature manager.
		 *
		 * @param[in,out] other Other feature manager instance
		 * @return Reference to the feature manager itself
		 */
		FeatureManager &operator=(FeatureManager &&other) noexcept;

		/**
		 * @brief Check if a specific extension is supported by the managers physical device.
		 *
		 * @param[in] extension Extension identifier string
		 * @return @p True, if the @p extension is supported, else @p false
		 */
		[[nodiscard]] bool isExtensionSupported(const std::string &extension) const;

		/**
		 * @brief Activate a specific extension if supported by the managers physical device.
		 *
		 * @param[in] extension Extension identifier string
		 * @param[in] required True, if the @p extension is required, else false
		 * @return @p True, if the @p extension could be activated, else @p false
		 */
		bool useExtension(const std::string &extension, bool required = true);

		/**
		 * @brief Check if a specific extension is activated by the manager.
		 *
		 * @param[in] extension Extension identifier string
		 * @return @p True, if the @p extension is activated, else @p false
		 */
		[[nodiscard]] bool isExtensionActive(const std::string &extension) const;

		/**
		 * @brief Return list of activated extensions for usage.
		 *
		 * @return List of activated extensions
		 */
		[[nodiscard]] const Vector<const char*> &getActiveExtensions() const;

		/**
		 * @brief Request specific features for optional or required usage ( only core Vulkan 1.0 ).
		 *
		 * @param[in] featureFunction Function or lambda to request specific features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the requested features could be activated, else @p false
		 */
		bool useFeatures(const std::function<void(vk::PhysicalDeviceFeatures &)> &featureFunction,
						 bool required = true);

		/**
		 * @brief Request specific features for optional or required usage.
		 *
		 * @tparam T Template parameter to use specific base structure types
		 * @param[in] featureFunction Function or lambda to request specific features
		 * @param[in] required True, if the @p features are required, else false
		 * @return @p True, if the requested features could be activated, else @p false
		 * @see checkSupport()
		 */
		template <typename T>
		bool useFeatures(const std::function<void(T &)> &featureFunction, bool required = true) {
			T features;
			T* features_ptr = reinterpret_cast<T*>(findFeatureStructure(features.sType));

			if (features_ptr) {
				features = *features_ptr;
			}

			featureFunction(features);

			if (!checkSupport(features, required)) {
				if (required) {
					vkcv_log_throw_error("Required feature is not supported!");
				}

				return false;
			}

			if (features_ptr) {
				*features_ptr = features;
				return true;
			}

			features_ptr = new T(features);

			if (m_featuresExtensions.empty()) {
				m_featuresBase.setPNext(features_ptr);
			} else {
				m_featuresExtensions.back()->setPNext(
					reinterpret_cast<vk::BaseOutStructure*>(features_ptr));
			}

			m_featuresExtensions.push_back(reinterpret_cast<vk::BaseOutStructure*>(features_ptr));

			return true;
		}

		/**
		 * @brief Return feature structure chain to request all activated features.
		 *
		 * @return Head of feature structure chain
		 */
		[[nodiscard]] const vk::PhysicalDeviceFeatures2 &getFeatures() const;

		/**
		 * @brief Checks all activated features for a specific feature and returns its state.
		 *
		 * @tparam T Template parameter to use specific base structure types
		 * @param[in] type Vulkan structure type identifier
		 * @param[in] featureTestFunction Function to test feature structure with for requested
		 * attributes
		 * @return True, if the requested attributes are available, else false
		 */
		template <typename T>
		bool checkFeatures(vk::StructureType type,
						   const std::function<bool(const T &)> &featureTestFunction) const {
			const auto* base = reinterpret_cast<const vk::BaseInStructure*>(&getFeatures());

			while (base) {
				if ((base->sType == type)
					&& (featureTestFunction(*reinterpret_cast<const T*>(base)))) {
					return true;
				}

				base = base->pNext;
			}

			return false;
		}
	};

} // namespace vkcv
