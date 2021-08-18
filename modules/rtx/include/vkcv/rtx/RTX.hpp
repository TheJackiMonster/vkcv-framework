#pragma once

#include <vector>
#include "vulkan/vulkan.hpp"

namespace vkcv::rtx {

    class RTXModule {
    private:

        std::vector<const char*> m_instanceExtensions;
        std::vector<const char*> m_deviceExtensions;
        vk::PhysicalDevice m_physicalDevice;
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};

    public:

        /**
         * @brief TODO
         */
        RTXModule();

        /**
         * @brief TODO
         */
        ~RTXModule() {};

        /**
         * @brief Returns the raytracing instance extensions.
         * @return The raytracing instance extensions.
         */
        std::vector<const char*> getInstanceExtensions();

        /**
         * @brief Returns the raytracing device extensions.
         * @return The raytracing device extensions.
         */
        std::vector<const char*> getDeviceExtensions();

        /**
         * @brief TODO
         * @param physicalDevice
         */
        void init(vk::PhysicalDevice &physicalDevice);

    };

}
