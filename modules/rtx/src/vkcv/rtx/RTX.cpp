#include "vkcv/rtx/RTX.hpp"

namespace vkcv::rtx {

    RTXModule::RTXModule(){

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

    void RTXModule::init(Core* core, std::vector<uint8_t>& vertices,
        std::vector<uint8_t>& indices, std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles)
    {
        m_core = core;
        // build acceleration structures BLAS then TLAS --> see ASManager
        //asManager(core);
        ASManager temp(core);
        m_asManager = &temp;
        m_asManager->buildBLAS(vertices, indices);
        m_asManager->buildTLAS();
        RTXDescriptors(m_asManager, core, descriptorSetHandles);
        
    }

    void RTXModule::RTXDescriptors(ASManager* asManager,Core* core, std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles)
    {
        //TLAS-Descriptor-Write
        TopLevelAccelerationStructure tlas = asManager->getTLAS();
        RTXBuffer tlasBuffer = tlas.tlasBuffer;
        vk::WriteDescriptorSetAccelerationStructureKHR AccelerationDescriptor = {};
        AccelerationDescriptor.accelerationStructureCount = 1;
        const TopLevelAccelerationStructure constTLAS = tlas;
        AccelerationDescriptor.pAccelerationStructures = &constTLAS.vulkanHandle;

        vk::WriteDescriptorSet tlasWrite;
        tlasWrite.setPNext(&AccelerationDescriptor);
        tlasWrite.setDstSet(core->getDescriptorSet(descriptorSetHandles[0]).vulkanHandle);
        tlasWrite.setDstBinding(1);
        tlasWrite.setDstArrayElement(0);
        tlasWrite.setDescriptorCount(1);
        tlasWrite.setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR);
        core->getContext().getDevice().updateDescriptorSets(tlasWrite, nullptr);

        //INDEX & VERTEX BUFFER
        BottomLevelAccelerationStructure blas = asManager->getBLAS(0);//HARD CODED

        //VERTEX BUFFER

        vk::DescriptorBufferInfo vertexInfo = {};
        vertexInfo.setBuffer(blas.vertexBuffer.vulkanHandle);
        vertexInfo.setOffset(0);
        vertexInfo.setRange(blas.vertexBuffer.deviceSize); //maybe check if size is correct

        vk::WriteDescriptorSet vertexWrite;
        vertexWrite.setDstSet(core->getDescriptorSet(descriptorSetHandles[1]).vulkanHandle);
        vertexWrite.setDstBinding(3);
        vertexWrite.setDescriptorCount(1);
        vertexWrite.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        vertexWrite.setPBufferInfo(&vertexInfo);
        core->getContext().getDevice().updateDescriptorSets(vertexWrite, nullptr);

        //INDEXBUFFER
        vk::DescriptorBufferInfo indexInfo = {};
        indexInfo.setBuffer(blas.indexBuffer.vulkanHandle);
        indexInfo.setOffset(0);
        indexInfo.setRange(blas.indexBuffer.deviceSize); //maybe check if size is correct

        vk::WriteDescriptorSet indexWrite;
        indexWrite.setDstSet(core->getDescriptorSet(descriptorSetHandles[1]).vulkanHandle);
        indexWrite.setDstBinding(4);
        indexWrite.setDescriptorCount(1);
        indexWrite.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        indexWrite.setPBufferInfo(&indexInfo);
        core->getContext().getDevice().updateDescriptorSets(indexWrite, nullptr);


    }

    vk::Pipeline RTXModule::createRTXPipeline(std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts, ShaderProgram &rayGenShader, ShaderProgram &rayMissShader, ShaderProgram &rayClosestHitShader) {
        // TODO: maybe all of this must be moved to the vkcv::PipelineManager? If we use scene.recordDrawcalls(), this requires a vkcv::PipelineHandle and not a vk::Pipeline

        // -- process vkcv::ShaderProgram into vk::ShaderModule
        std::vector<char> rayGenShaderCodeTemp = rayGenShader.getShader(ShaderStage::RAY_GEN).shaderCode;
        std::vector<uint32_t> rayGenShaderCode(rayGenShaderCodeTemp.begin(), rayGenShaderCodeTemp.end());
        vk::ShaderModuleCreateInfo rayGenShaderModuleInfo(
            vk::ShaderModuleCreateFlags(), // vk::ShaderModuleCreateFlags flags_,
            rayGenShaderCode // vk::ArrayProxyNoTemporaries<const uint32_t> const & code_
        );
        vk::ShaderModule rayGenShaderModule = m_core->getContext().getDevice().createShaderModule(rayGenShaderModuleInfo, nullptr, m_asManager->getDispatcher());
        if (!rayGenShaderModule) {
            vkcv_log(LogLevel::ERROR, "The Ray Generation Shader Module could not be created!");
        }

        std::vector<char> rayMissShaderCodeTemp = rayMissShader.getShader(ShaderStage::RAY_MISS).shaderCode;
        std::vector<uint32_t> rayMissShaderCode(rayMissShaderCodeTemp.begin(), rayMissShaderCodeTemp.end());
        vk::ShaderModuleCreateInfo rayMissShaderModuleInfo(
            vk::ShaderModuleCreateFlags(), // vk::ShaderModuleCreateFlags flags_,
            rayMissShaderCode // vk::ArrayProxyNoTemporaries<const uint32_t> const & code_
        );
        vk::ShaderModule rayMissShaderModule = m_core->getContext().getDevice().createShaderModule(rayMissShaderModuleInfo, nullptr, m_asManager->getDispatcher());
        if (!rayMissShaderModule) {
            vkcv_log(LogLevel::ERROR, "The Ray Miss Shader Module could not be created!");
        }

        std::vector<char> rayClosestHitShaderTemp = rayClosestHitShader.getShader(ShaderStage::RAY_CLOSEST_HIT).shaderCode;
        std::vector<uint32_t> rayClosestHitShaderCode(rayClosestHitShaderTemp.begin(), rayClosestHitShaderTemp.end());
        vk::ShaderModuleCreateInfo rayClosestHitShaderModuleInfo(
            vk::ShaderModuleCreateFlags(), // vk::ShaderModuleCreateFlags flags_,
            rayClosestHitShaderCode // vk::ArrayProxyNoTemporaries<const uint32_t> const & code_
        );
        vk::ShaderModule rayClosestHitShaderModule = m_core->getContext().getDevice().createShaderModule(rayClosestHitShaderModuleInfo, nullptr, m_asManager->getDispatcher());
        if (!rayClosestHitShaderModule) {
            vkcv_log(LogLevel::ERROR, "The Ray Closest Hit Shader Module could not be created!");
        }

        // -- PipelineShaderStages

        // ray generation
        vk::PipelineShaderStageCreateInfo rayGenShaderStageInfo(
        vk::PipelineShaderStageCreateFlags(), // vk::PipelineShaderStageCreateFlags flags_ = {}
            vk::ShaderStageFlagBits::eRaygenKHR, // vk::ShaderStageFlagBits stage_ = vk::ShaderStageFlagBits::eVertex,
            rayGenShaderModule, // vk::ShaderModule module_ = {},
            "main" // const char* pName_ = {},
        );

        // ray miss
        vk::PipelineShaderStageCreateInfo rayMissShaderStageInfo(
            vk::PipelineShaderStageCreateFlags(), // vk::PipelineShaderStageCreateFlags flags_ = {}
            vk::ShaderStageFlagBits::eMissKHR, // vk::ShaderStageFlagBits stage_ = vk::ShaderStageFlagBits::eVertex,
            rayMissShaderModule, // vk::ShaderModule module_ = {},
            "main" // const char* pName_ = {},
        );

        // ray clostest hit
        vk::PipelineShaderStageCreateInfo rayClosestHitShaderStageInfo(
            vk::PipelineShaderStageCreateFlags(), // vk::PipelineShaderStageCreateFlags flags_ = {}
            vk::ShaderStageFlagBits::eClosestHitKHR, // vk::ShaderStageFlagBits stage_ = vk::ShaderStageFlagBits::eVertex,
            rayClosestHitShaderModule, // vk::ShaderModule module_ = {},
            "main" // const char* pName_ = {},
        );

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {   // HARD CODED
            rayGenShaderStageInfo, rayMissShaderStageInfo, rayClosestHitShaderStageInfo
        };

        // -- PipelineLayouts

        std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups(3);   // HARD CODED
        shaderGroups[0] = vk::RayTracingShaderGroupCreateInfoKHR(
            vk::RayTracingShaderGroupTypeKHR::eGeneral, // vk::RayTracingShaderGroupTypeKHR type_ = vk::RayTracingShaderGroupTypeKHR::eGeneral
            0, // uint32_t generalShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t closestHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t anyHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t intersectionShader_ = {}
            nullptr // const void* pShaderGroupCaptureReplayHandle_ = {}
        );
        shaderGroups[1] = vk::RayTracingShaderGroupCreateInfoKHR(
            vk::RayTracingShaderGroupTypeKHR::eGeneral, // vk::RayTracingShaderGroupTypeKHR type_ = vk::RayTracingShaderGroupTypeKHR::eGeneral
            1, // uint32_t generalShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t closestHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t anyHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t intersectionShader_ = {}
            nullptr // const void* pShaderGroupCaptureReplayHandle_ = {}
        );
        shaderGroups[2] = vk::RayTracingShaderGroupCreateInfoKHR(
            vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup, // vk::RayTracingShaderGroupTypeKHR type_ = vk::RayTracingShaderGroupTypeKHR::eGeneral
            2, // uint32_t generalShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t closestHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t anyHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t intersectionShader_ = {}
            nullptr // const void* pShaderGroupCaptureReplayHandle_ = {}
        );

        std::vector<vk::DescriptorSetLayout> descriptorSetLayoutsVulkan;
        for (size_t i=0; i<descriptorSetLayouts.size(); i++) {
            descriptorSetLayoutsVulkan.push_back(m_core->getDescriptorSetLayout(descriptorSetLayouts[i]).vulkanHandle);
        }

        vk::PipelineLayoutCreateInfo rtxPipelineLayoutCreateInfo(
            vk::PipelineLayoutCreateFlags(), // vk::PipelineLayoutCreateFlags flags_ = {}
            2, // uint32_t setLayoutCount_ = {}     HARD CODED
            descriptorSetLayoutsVulkan.data(), // const vk::DescriptorSetLayout* pSetLayouts_ = {}
            0, // uint32_t pushConstantRangeCount_ = {}
            nullptr // const vk::PushConstantRange* pPushConstantRanges_ = {}
        );

        vk::PipelineLayout rtxPipelineLayout = m_core->getContext().getDevice().createPipelineLayout(rtxPipelineLayoutCreateInfo, nullptr, m_asManager->getDispatcher());
        if (!rtxPipelineLayout) {
            vkcv_log(LogLevel::ERROR, "The RTX Pipeline Layout could not be created!");
        }

        vk::PipelineLibraryCreateInfoKHR rtxPipelineLibraryCreateInfo(
            0, // uint32_t libraryCount_ = {}
            nullptr // const vk::Pipeline* pLibraries_ = {}
        );

        // -- RTX Pipeline

        vk::RayTracingPipelineCreateInfoKHR rtxPipelineInfo(
            vk::PipelineCreateFlags(), // vk::PipelineCreateFlags flags_ = {}
            shaderStages.size(), // uint32_t stageCount_ = {}
            shaderStages.data(), // const vk::PipelineShaderStageCreateInfo* pStages_ = {}
            shaderGroups.size(), // uint32_t groupCount_ = {}
            shaderGroups.data(), // const vk::RayTracingShaderGroupCreateInfoKHR* pGroups_ = {}
            16, // uint32_t maxPipelineRayRecursionDepth_ = {}
            &rtxPipelineLibraryCreateInfo, // const vk::PipelineLibraryCreateInfoKHR* pLibraryInfo_ = {}
            nullptr, // const vk::RayTracingPipelineInterfaceCreateInfoKHR* pLibraryInterface_ = {}
            nullptr, // const vk::PipelineDynamicStateCreateInfo* pDynamicState_ = {}
            rtxPipelineLayout, // vk::PipelineLayout layout_ = {}
            nullptr, // vk::Pipeline basePipelineHandle_ = {}
            -1 // int32_t basePipelineIndex_ = {}
        );

        vk::Pipeline rtxPipeline = m_core->getContext().getDevice().createRayTracingPipelineKHR(nullptr, nullptr, rtxPipelineInfo, nullptr, m_asManager->getDispatcher());
        if (!rtxPipeline) {
            vkcv_log(LogLevel::ERROR, "The RTX Pipeline could not be created!");
        }

        m_core->getContext().getDevice().destroy(rayGenShaderModule, nullptr, m_asManager->getDispatcher());
        m_core->getContext().getDevice().destroy(rayMissShaderModule, nullptr, m_asManager->getDispatcher());
        m_core->getContext().getDevice().destroy(rayClosestHitShaderModule, nullptr, m_asManager->getDispatcher());

        return rtxPipeline;
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