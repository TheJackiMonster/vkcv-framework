#include "vkcv/rtx/RTX.hpp"

namespace vkcv::rtx {

    RTXModule::RTXModule() {

        // prepare needed raytracing extensions
        m_instanceExtensions = {
                "VK_KHR_get_physical_device_properties2"
        };
        m_deviceExtensions = {
                "VK_KHR_maintenance3",
                "VK_EXT_descriptor_indexing",
                "VK_KHR_buffer_device_address",
                "VK_KHR_deferred_host_operations",
                "VK_KHR_acceleration_structure",
                "VK_KHR_spirv_1_4",
                "VK_KHR_ray_tracing_pipeline",
                "VK_KHR_ray_query",
                "VK_KHR_pipeline_library"
        };

        // enable raytracing features!
        vk::PhysicalDeviceAccelerationStructureFeaturesKHR accelerationFeature(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR);
        vk::PhysicalDeviceRayTracingPipelineFeaturesKHR raytracingPipelineFeature(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR);
    }

    void RTXModule::init(vk::PhysicalDevice &physicalDevice) {
        m_physicalDevice = physicalDevice;

        // Requesting ray tracing properties
        vk::PhysicalDeviceProperties2 prop2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        prop2.pNext = &m_rtProperties;
        m_physicalDevice.getProperties2(&prop2);
    }

    std::vector<const char*> RTXModule::getInstanceExtensions() {
        return m_instanceExtensions;
    }

    std::vector<const char*> RTXModule::getDeviceExtensions() {
        return m_deviceExtensions;
    }


}