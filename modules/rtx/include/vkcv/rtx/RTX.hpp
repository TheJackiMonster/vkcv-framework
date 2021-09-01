#pragma once

#include <vector>
#include "vulkan/vulkan.hpp"
#include "vkcv/Core.hpp"

namespace vkcv::rtx {

    class RTXModule {
    private:

        std::vector<const char*> m_instanceExtensions;
        std::vector<const char*> m_deviceExtensions;
        vkcv::Features m_features;

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
         * @return
         */
        vkcv::Features getFeatures();

        /**
         * @brief TODO
         * @param core
         * @param vertexBuffer
         * @param indexBuffer
         */
        void init(Core* core, Buffer<uint16_t> &vertexBuffer, Buffer<uint16_t> &indexBuffer);
    };

}
