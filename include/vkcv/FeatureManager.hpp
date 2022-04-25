#pragma once
/**
 * @authors Tobias Frisch, Artur Wasmut, Vanessa Karolek, Sebastian Gaida
 * @file vkcv/FeatureManager.hpp
 * @brief Class to manage feature support and extension usage.
 */

#include <functional>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "Logger.hpp"

namespace vkcv {

	/**
	 * Class to manage extension and feature requirements, support and usage.
	 */
	class FeatureManager {
	private:
		/**
		 * Physical device to check feature support against.
		 */
		vk::PhysicalDevice& m_physicalDevice;
		
		/**
		 * List of supported extensions.
		 */
		std::vector<const char*> m_supportedExtensions;
		
		/**
		 * List of activated extensions for usage.
		 */
		std::vector<const char*> m_activeExtensions;
		
		/**
		 * Feature structure chain to request activated features.
		 */
		vk::PhysicalDeviceFeatures2 m_featuresBase;
		
		/**
		 * List of base structures allocated to request extension specific features.
		 */
		std::vector<vk::BaseOutStructure*> m_featuresExtensions;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceFeatures& features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDevice16BitStorageFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDevice16BitStorageFeatures& features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDevice8BitStorageFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDevice8BitStorageFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceBufferDeviceAddressFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceBufferDeviceAddressFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceDescriptorIndexingFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceDescriptorIndexingFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceHostQueryResetFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceHostQueryResetFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceImagelessFramebufferFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceImagelessFramebufferFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceMultiviewFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceMultiviewFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceProtectedMemoryFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceProtectedMemoryFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceSamplerYcbcrConversionFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceSamplerYcbcrConversionFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceScalarBlockLayoutFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceScalarBlockLayoutFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceSeparateDepthStencilLayoutsFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceSeparateDepthStencilLayoutsFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceShaderAtomicInt64Features.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceShaderAtomicInt64Features &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceShaderFloat16Int8Features.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceShaderFloat16Int8Features& features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceTimelineSemaphoreFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceTimelineSemaphoreFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceUniformBufferStandardLayoutFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceUniformBufferStandardLayoutFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceVariablePointersFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceVariablePointersFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceVulkanMemoryModelFeatures.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceVulkanMemoryModelFeatures &features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceMeshShaderFeaturesNV.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceMeshShaderFeaturesNV& features, bool required) const;
		
		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT.
		 *
		 * @param features The features
		 * @param required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceShaderAtomicFloatFeaturesEXT& features, bool required) const;
		
		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceShaderAtomicFloat2FeaturesEXT.
		 *
		 * @param features The features
		 * @param required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceShaderAtomicFloat2FeaturesEXT& features, bool required) const;
		
		/**
         * @brief Checks support of the @p vk::PhysicalDeviceVulkan12Features.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceVulkan12Features& features, bool required) const;

		/**
         * @brief Checks support of the @p vk::PhysicalDeviceVulkan11Features.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceVulkan11Features& features, bool required) const;

		/**
		 * @brief Checks support of the @p vk::PhysicalDeviceAccelerationStructureFeaturesKHR.
		 *
		 * @param features The features
		 * @param required True, if the @p features are required, else false
		 * @return @p True, if the @p features are supported, else @p false
		 */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceAccelerationStructureFeaturesKHR& features, bool required) const;

		/**
         * @brief Checks support of the @p vk::PhysicalDeviceRayTracingPipelineFeaturesKHR.
         *
         * @param features The features
         * @param required True, if the @p features are required, else false
         * @return @p True, if the @p features are supported, else @p false
         */
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceRayTracingPipelineFeaturesKHR& features, bool required) const;
		
		/**
		 * @brief Searches for a base structure of a given structure type.
		 *
		 * @param type Structure type
		 * @return Pointer to first matching base structure or nullptr
		 */
		[[nodiscard]]
		vk::BaseOutStructure* findFeatureStructure(vk::StructureType type) const;
	
	public:
		/**
		 * @brief Constructor of a feature manager with a given physical device.
		 *
		 * @param physicalDevice Physical device
		 */
		explicit FeatureManager(vk::PhysicalDevice& physicalDevice);
		
		FeatureManager(const FeatureManager& other) = delete;
		
		/**
		 * @brief Move-constructor of a feature manager.
		 *
		 * @param other Other feature manager instance
		 */
		FeatureManager(FeatureManager&& other) noexcept;
		
		/**
		 * @brief Destructor of a feature manager.
		 */
		~FeatureManager();
		
		FeatureManager& operator=(const FeatureManager& other) = delete;
		
		/**
		 * @brief Move-operator of a feature manager.
		 *
		 * @param other Other feature manager instance
		 * @return Reference to the feature manager itself
		 */
		FeatureManager& operator=(FeatureManager&& other) noexcept;
		
		/**
		 * @brief Check if a specific extension is supported by the managers physical device.
		 *
		 * @param extension Extension identifier string
		 * @return @p True, if the @p extension is supported, else @p false
		 */
		[[nodiscard]]
		bool isExtensionSupported(const std::string& extension) const;
		
		/**
		 * @brief Activate a specific extension if supported by the managers physical device.
		 *
		 * @param extension Extension identifier string
		 * @param required True, if the @p extension is required, else false
		 * @return @p True, if the @p extension could be activated, else @p false
		 */
		bool useExtension(const std::string& extension, bool required = true);
		
		/**
		 * @brief Check if a specific extension is activated by the manager.
		 *
		 * @param extension Extension identifier string
		 * @return @p True, if the @p extension is activated, else @p false
		 */
		[[nodiscard]]
		bool isExtensionActive(const std::string& extension) const;
		
		/**
		 * @brief Return list of activated extensions for usage.
		 *
		 * @return List of activated extensions
		 */
		[[nodiscard]]
		const std::vector<const char*>& getActiveExtensions() const;
		
		/**
		 * @brief Request specific features for optional or required usage ( only core Vulkan 1.0 ).
		 *
		 * @param featureFunction Function or lambda to request specific features
		 * @param required True, if the @p features are required, else false
		 * @return @p True, if the requested features could be activated, else @p false
		 */
		bool useFeatures(const std::function<void(vk::PhysicalDeviceFeatures&)>& featureFunction, bool required = true);
		
		/**
		 * @brief Request specific features for optional or required usage.
		 *
		 * @tparam T Template parameter to use specific base structure types
		 * @param featureFunction Function or lambda to request specific features
		 * @param required True, if the @p features are required, else false
		 * @return @p True, if the requested features could be activated, else @p false
		 * @see checkSupport()
		 */
		template<typename T>
		bool useFeatures(const std::function<void(T&)>& featureFunction, bool required = true) {
			T features;
			T* features_ptr = reinterpret_cast<T*>(findFeatureStructure(features.sType));
			
			if (features_ptr) {
				features = *features_ptr;
			}
			
			featureFunction(features);
			
			if (!checkSupport(features, required)) {
				if (required) {
					throw std::runtime_error("Required feature is not supported!");
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
						reinterpret_cast<vk::BaseOutStructure*>(features_ptr)
				);
			}
			
			m_featuresExtensions.push_back(
					reinterpret_cast<vk::BaseOutStructure*>(features_ptr)
			);
			
			return true;
		}
		
		/**
		 * @brief Return feature structure chain to request activated features.
		 *
		 * @return Head of feature structure chain
		 */
		[[nodiscard]]
		const vk::PhysicalDeviceFeatures2& getFeatures() const;
		
	};
	
}
