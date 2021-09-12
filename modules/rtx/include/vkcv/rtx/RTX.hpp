#pragma once

#include <vector>
#include "vulkan/vulkan.hpp"
#include "vkcv/Core.hpp"
#include "ASManager.hpp"

namespace vkcv::rtx {

    class RTXModule {
    private:

        std::vector<const char*> m_instanceExtensions;  // the instance extensions needed for using RTX
        std::vector<const char*> m_deviceExtensions;    // the device extensions needed for using RTX
        vkcv::Features m_features;                      // the features needed to be enabled for using RTX
        ASManager* m_asManager;
        Core* m_core;

    public:

        /**
         * @brief Default #RTXModule constructor.
         */
        RTXModule();

        /**
         * @brief Default #RTXModule destructor.
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
         * @brief Returns the raytracing features.
         * @return The raytracing features.
         */
        vkcv::Features getFeatures();

        /**
         * @brief Initializes the RTXModule with scene data.
         * @param core The reference to the #Core.
         * @param vertices The scene vertex data of type uint8_t.
         * @param indices The scene index data of type uint8_t.
         * @param descriptorSetHandles The descriptorSetHandles for RTX
         */
        void init(Core* core, std::vector<uint8_t> &vertices, std::vector<uint8_t> &indices, std::vector<vkcv::DescriptorSetHandle> &descriptorSetHandles);

        /**
         * @brief Creates Descriptor-Writes for RTX
         * @param asManager The ASManager of RTX
         * @param descriptorSetHandles The descriptorSetHandles for RTX
         */
        void RTXDescriptors(ASManager* asManager, Core* core, std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles);

        /**
         * @brief Returns the Vulkan handle of the RTX pipeline.
         * @param descriptorSetLayouts The descriptorSetLayouts used for creating a @p vk::PipelineLayoutCreateInfo.
         * @param rayGenShader The ray generation shader.
         * @param rayMissShader The ray miss shader.
         * @param rayClostestHitShader The ray closest hit shader.
         * @return The Vulkan handle of the RTX pipeline.
         */
        vk::Pipeline createRTXPipeline(std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts, ShaderProgram &rayGenShader, ShaderProgram &rayMissShader, ShaderProgram &rayClosestHitShader);
    };

}
