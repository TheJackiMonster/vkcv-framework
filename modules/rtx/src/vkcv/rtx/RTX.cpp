#include "vkcv/rtx/RTX.hpp"
#include "vkcv/rtx/ASManager.hpp"

namespace vkcv::rtx {

    RTXModule::RTXModule() {

        // prepare needed raytracing extensions
        m_instanceExtensions = {
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
        };
        m_deviceExtensions = {
                VK_KHR_MAINTENANCE3_EXTENSION_NAME,
                VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                VK_KHR_SPIRV_1_4_EXTENSION_NAME,
                VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
                VK_KHR_RAY_QUERY_EXTENSION_NAME,
                VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME
        };

        // get all features required by the device extensions
        for(auto deviceExtension : m_deviceExtensions) {
            m_features.requireExtension(deviceExtension);
        }

        /* FIXME : We must disable features that will be mentioned as "not supported" by the FeatureManager. If every unsupported feature is disabled, this should work.
         * Maybe we find a better workaround...
         */
        m_features.requireFeature<vk::PhysicalDeviceVulkan12Features>(
                [](vk::PhysicalDeviceVulkan12Features &features) {
                    features.setSamplerMirrorClampToEdge(true);
                    features.setDrawIndirectCount(true);
                    features.setStorageBuffer8BitAccess(true);
                    features.setUniformAndStorageBuffer8BitAccess(true);
                    features.setStoragePushConstant8(true);
                    features.setShaderBufferInt64Atomics(true);
                    features.setShaderSharedInt64Atomics(true);
                    features.setShaderFloat16(true);
                    features.setShaderInt8(true);
                    features.setDescriptorIndexing(true);
                    features.setShaderInputAttachmentArrayDynamicIndexing(true);
                    features.setShaderUniformTexelBufferArrayDynamicIndexing(true);
                    features.setShaderStorageTexelBufferArrayDynamicIndexing(true);
                    features.setShaderUniformBufferArrayNonUniformIndexing(true);
                    features.setShaderSampledImageArrayNonUniformIndexing(true);
                    features.setShaderStorageBufferArrayNonUniformIndexing(true);
                    features.setShaderStorageImageArrayNonUniformIndexing(true);
                    features.setShaderInputAttachmentArrayNonUniformIndexing(true);
                    features.setShaderUniformTexelBufferArrayNonUniformIndexing(true);
                    features.setShaderStorageTexelBufferArrayNonUniformIndexing(true);
                    features.setDescriptorBindingUniformBufferUpdateAfterBind(true);
                    features.setDescriptorBindingSampledImageUpdateAfterBind(true);
                    features.setDescriptorBindingStorageImageUpdateAfterBind(true);
                    features.setDescriptorBindingStorageBufferUpdateAfterBind(true);
                    features.setDescriptorBindingUniformTexelBufferUpdateAfterBind(true);
                    features.setDescriptorBindingStorageTexelBufferUpdateAfterBind(true);
                    features.setDescriptorBindingUpdateUnusedWhilePending(true);
                    features.setDescriptorBindingPartiallyBound(true);
                    features.setDescriptorBindingVariableDescriptorCount(true);
                    features.setRuntimeDescriptorArray(true);
                    features.setSamplerFilterMinmax(true);
                    features.setScalarBlockLayout(true);
                    features.setImagelessFramebuffer(true);
                    features.setUniformBufferStandardLayout(true);
                    features.setShaderSubgroupExtendedTypes(true);
                    features.setSeparateDepthStencilLayouts(true);
                    features.setHostQueryReset(true);
                    features.setTimelineSemaphore(true);
                    features.setBufferDeviceAddress(true);
                    features.setBufferDeviceAddressCaptureReplay(true);
                    features.setBufferDeviceAddressMultiDevice(true);
                    features.setVulkanMemoryModel(true);
                    features.setVulkanMemoryModelDeviceScope(true);
                    features.setVulkanMemoryModelAvailabilityVisibilityChains(true);
                    features.setShaderOutputViewportIndex(true);
                    features.setShaderOutputLayer(true);
                    features.setSubgroupBroadcastDynamicId(true);
                });
        m_features.requireFeature<vk::PhysicalDeviceVulkan11Features>(
                [](vk::PhysicalDeviceVulkan11Features &features) {
                    features.setMultiview(true);
                    features.setMultiviewGeometryShader(true);
                    features.setMultiviewTessellationShader(true);
//                    features.setProtectedMemory(true);    // not supported
                    features.setSamplerYcbcrConversion(true);
                    features.setShaderDrawParameters(true);
                    features.setStorageBuffer16BitAccess(true);
//                    features.setStorageInputOutput16(true);   // not supported
                    features.setStoragePushConstant16(true);
                    features.setUniformAndStorageBuffer16BitAccess(true);
                    features.setVariablePointers(true);
                    features.setVariablePointersStorageBuffer(true);
                });
        m_features.requireFeature<vk::PhysicalDeviceAccelerationStructureFeaturesKHR>(
                [](vk::PhysicalDeviceAccelerationStructureFeaturesKHR &features) {
                    features.setAccelerationStructure(true);
                    features.setAccelerationStructureCaptureReplay(true);
//                    features.setAccelerationStructureIndirectBuild(true); // not supported
//                    features.setAccelerationStructureHostCommands(true);  // not supported
                    features.setDescriptorBindingAccelerationStructureUpdateAfterBind(true);
                });
        m_features.requireExtensionFeature<vk::PhysicalDeviceRayTracingPipelineFeaturesKHR>(
                VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, [](vk::PhysicalDeviceRayTracingPipelineFeaturesKHR &features) {
                    features.setRayTracingPipeline(true);
//                    features.setRayTracingPipelineShaderGroupHandleCaptureReplay(true);   // not supported
//                    features.setRayTracingPipelineShaderGroupHandleCaptureReplayMixed(true);  // not supported
                    features.setRayTracingPipelineTraceRaysIndirect(true);
                    features.setRayTraversalPrimitiveCulling(true);
                });
    }

    void RTXModule::init(Core* core, std::vector<uint8_t> &vertices, std::vector<uint8_t> &indices) {
        // build acceleration structures BLAS then TLAS --> see ASManager
        ASManager asManager(core);
        asManager.buildBLAS(vertices, indices);
        asManager.buildTLAS();
    }

    std::vector<const char*> RTXModule::getInstanceExtensions() {
        return m_instanceExtensions;
    }

    std::vector<const char*> RTXModule::getDeviceExtensions() {
        return m_deviceExtensions;
    }

    vkcv::Features RTXModule::getFeatures() {
        return m_features;
    }

}