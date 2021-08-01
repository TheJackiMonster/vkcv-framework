#pragma once

#include "Logger.hpp"

#include <functional>
#include <unordered_set>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace vkcv {

	class FeatureManager {
	private:
		vk::PhysicalDevice& m_physicalDevice;
		
		std::vector<const char*> m_supportedExtensions;
		std::vector<const char*> m_activeExtensions;
		
		vk::PhysicalDeviceFeatures2 m_featuresBase;
		std::vector<vk::BaseOutStructure*> m_featuresExtensions;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDevice16BitStorageFeatures& features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDevice8BitStorageFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceBufferDeviceAddressFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceDescriptorIndexingFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceHostQueryResetFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceImagelessFramebufferFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceMultiviewFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceProtectedMemoryFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceSamplerYcbcrConversionFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceScalarBlockLayoutFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceSeparateDepthStencilLayoutsFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceShaderAtomicInt64Features &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceShaderFloat16Int8Features& features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceShaderSubgroupExtendedTypesFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceTimelineSemaphoreFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceUniformBufferStandardLayoutFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceVariablePointersFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceVulkanMemoryModelFeatures &features, bool required) const;
		
		[[nodiscard]]
		bool checkSupport(const vk::PhysicalDeviceMeshShaderFeaturesNV& features, bool required) const;
	
	public:
		explicit FeatureManager(vk::PhysicalDevice& physicalDevice);
		
		FeatureManager(const FeatureManager& other) = delete;
		FeatureManager(FeatureManager&& other) noexcept;
		
		~FeatureManager();
		
		FeatureManager& operator=(const FeatureManager& other) = delete;
		FeatureManager& operator=(FeatureManager&& other) noexcept;
		
		[[nodiscard]]
		bool isExtensionSupported(const char *extension) const;
		
		bool useExtension(const char *extension, bool required = true);
		
		[[nodiscard]]
		bool isExtensionActive(const char *extension) const;
		
		[[nodiscard]]
		const std::vector<const char*>& getActiveExtensions() const;
		
		bool useFeatures(const std::function<void(vk::PhysicalDeviceFeatures&)>& featureFunction, bool required = true);
		
		template<typename T>
		bool useFeatures(const std::function<void(T&)>& featureFunction, bool required = true) {
			T* features = new T();
			featureFunction(*features);
			
			if (!checkSupport(*features, required)) {
				delete features;
				return false;
			}
			
			if (m_featuresExtensions.empty()) {
				m_featuresBase.setPNext(features);
			} else {
				m_featuresExtensions.back()->setPNext(
						reinterpret_cast<vk::BaseOutStructure*>(features)
				);
			}
			
			m_featuresExtensions.push_back(
					reinterpret_cast<vk::BaseOutStructure*>(features)
			);
			
			return true;
		}
		
		[[nodiscard]]
		const vk::PhysicalDeviceFeatures2& getFeatures() const;
		
	};
	
}
