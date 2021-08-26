#pragma once

#include <vector>
#include "vulkan/vulkan.hpp"

namespace vkcv::rtx {

    class RTXModule {
    private:

        std::vector<const char*> m_instanceExtensions;
        std::vector<const char*> m_deviceExtensions;

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
         * @param core
         * @param vertexBuffer
         * @param indexBuffer
         */
        void init(vkcv::Buffer<uint8_t> vertexBuffer, vkcv::Buffer<uint8_t> indexBuffer);
    };

}
