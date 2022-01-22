#include "RTXExtensions.hpp"

namespace vkcv::rtx{

RTXExtensions::RTXExtensions()
{

    // prepare needed raytracing extensions
    m_instanceExtensions = {
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
    };
    m_deviceExtensions = {
            VK_KHR_MAINTENANCE3_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
            VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME
    };

    // get all features required by the device extensions
    for (auto deviceExtension : m_deviceExtensions) {
        m_features.requireExtension(deviceExtension);
    }
	
	m_features.requireExtensionFeature<vk::PhysicalDeviceDescriptorIndexingFeatures>(
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
			[](vk::PhysicalDeviceDescriptorIndexingFeatures& features) {}
	);
	
	m_features.requireExtensionFeature<vk::PhysicalDeviceBufferDeviceAddressFeatures>(
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
			[](vk::PhysicalDeviceBufferDeviceAddressFeatures& features) {
				features.setBufferDeviceAddress(true);
			}
	);
	
    m_features.requireExtensionFeature<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>(
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			[](vk::PhysicalDeviceAccelerationStructureFeaturesKHR& features) {
				features.setAccelerationStructure(true);
			}
	);
	
    m_features.requireExtensionFeature<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>(
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			[](vk::PhysicalDeviceRayTracingPipelineFeaturesKHR& features) {
				features.setRayTracingPipeline(true);
			}
	);
}

std::vector<const char*> RTXExtensions::getInstanceExtensions()
{
	return m_instanceExtensions;
}

std::vector<const char*> RTXExtensions::getDeviceExtensions()
{
	return m_deviceExtensions;
}

vkcv::Features RTXExtensions::getFeatures()
{
	return m_features;
}

}