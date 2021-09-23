#include "RTX.hpp"

namespace vkcv::rtx {

    RTXModule::RTXModule(Core* core, ASManager* asManager, std::vector<float>& vertices,
        std::vector<uint32_t>& indices, std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles){
        m_core = core;
        m_asManager = asManager;
        // build acceleration structures BLAS then TLAS --> see ASManager
        RTXBuffer vertexBuffer = m_asManager->makeBufferFromData(vertices);
        RTXBuffer indexBuffer  = m_asManager->makeBufferFromData(indices);
        m_asManager->buildBLAS(vertexBuffer, vertices.size(), indexBuffer,indices.size());
        m_asManager->buildTLAS();
        RTXDescriptors(descriptorSetHandles);
    }


    RTXModule::~RTXModule()
    {
        m_core->getContext().getDevice().destroy(m_pipeline);
        m_core->getContext().getDevice().destroy(m_pipelineLayout);
        m_core->getContext().getDevice().destroy(m_shaderBindingTableBuffer.vulkanHandle);
        m_core->getContext().getDevice().freeMemory(m_shaderBindingTableBuffer.deviceMemory);
    }
    
    void RTXModule::createShaderBindingTable(uint32_t shaderCount) {
        vk::PhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProperties;

        vk::PhysicalDeviceProperties2 physicalProperties;
        physicalProperties.pNext = &rayTracingProperties;

        m_core->getContext().getPhysicalDevice().getProperties2(&physicalProperties);

        vk::DeviceSize shaderBindingTableSize = rayTracingProperties.shaderGroupHandleSize * shaderCount;


        m_shaderBindingTableBuffer.bufferType = RTXBufferType::SHADER_BINDING;
        m_shaderBindingTableBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eShaderBindingTableKHR | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
        m_shaderBindingTableBuffer.memoryPropertyFlagBits = vk::MemoryPropertyFlagBits::eHostVisible;
        m_shaderBindingTableBuffer.deviceSize = shaderBindingTableSize;

        m_asManager->createBuffer(m_shaderBindingTableBuffer);

        void* shaderHandleStorage = (void*)malloc(sizeof(uint8_t) * shaderBindingTableSize);
        
        if (m_core->getContext().getDevice().getRayTracingShaderGroupHandlesKHR(m_pipeline, 0, shaderCount, shaderBindingTableSize,
            shaderHandleStorage, m_asManager->getDispatcher()) != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "Could not retrieve shader binding table group handles.");
        }

        m_shaderGroupBaseAlignment =  rayTracingProperties.shaderGroupBaseAlignment;
        uint8_t* mapped = (uint8_t*) m_core->getContext().getDevice().mapMemory(m_shaderBindingTableBuffer.deviceMemory, 0, shaderBindingTableSize);
        for (int i = 0; i < shaderCount; i++) {
            memcpy(mapped, (uint8_t*)shaderHandleStorage + (i * rayTracingProperties.shaderGroupHandleSize), rayTracingProperties.shaderGroupHandleSize);
            mapped += m_shaderGroupBaseAlignment;
        }

        m_core->getContext().getDevice().unmapMemory(m_shaderBindingTableBuffer.deviceMemory);
        free(shaderHandleStorage);        
    }

    ShaderBindingTableRegions RTXModule::createRegions() {
        // Define offsets for the RTX shaders. RayGen is the first allocated shader. Each following shader is
        // shifted by shaderGroupBaseAlignment.
        // Offset Calculation: offset = count of previous shaders * m_shaderGroupBaseAlignment
        // Regions are hard coded
        vk::DeviceSize rayGenOffset = 0; //First Shader group -> offset 0 * m_shaderGroupBaseAlignment =0
        vk::DeviceSize missOffset = m_shaderGroupBaseAlignment;//Second group, offset = 1 * m_shaderGroupBaseAlignment
        vk::DeviceSize closestHitOffset = 2 * m_shaderGroupBaseAlignment; //Third group, offset = 2 * m_shaderGroupBaseAlignment
        vk::DeviceSize shaderBindingTableSize = m_shaderGroupBaseAlignment * 3; // 3 hardcoded to rtx-shader count

        auto m_rtxDispatcher = vk::DispatchLoaderDynamic((PFN_vkGetInstanceProcAddr)m_core->getContext().getInstance().getProcAddr("vkGetInstanceProcAddr"));
        m_rtxDispatcher.init(m_core->getContext().getInstance());
        

        // Create regions for the shader binding table buffer which are used for vk::CommandBuffer::traceRaysKHR
        vk::StridedDeviceAddressRegionKHR rgenRegion;
        vk::BufferDeviceAddressInfoKHR shaderBindingTableAddressInfo(m_shaderBindingTableBuffer.vulkanHandle);
        rgenRegion.deviceAddress = m_core->getContext().getDevice().getBufferAddressKHR(shaderBindingTableAddressInfo, m_rtxDispatcher) + rayGenOffset;
        rgenRegion.setStride(shaderBindingTableSize);
        rgenRegion.setSize(shaderBindingTableSize);
        vk::StridedDeviceAddressRegionKHR rmissRegion;
        rmissRegion.deviceAddress = m_core->getContext().getDevice().getBufferAddressKHR(shaderBindingTableAddressInfo, m_rtxDispatcher) + missOffset;
        rmissRegion.setStride(shaderBindingTableSize);
        rmissRegion.setSize(shaderBindingTableSize);
        vk::StridedDeviceAddressRegionKHR rchitRegion;
        rchitRegion.deviceAddress = m_core->getContext().getDevice().getBufferAddressKHR(shaderBindingTableAddressInfo, m_rtxDispatcher) + closestHitOffset;
        rchitRegion.setStride(shaderBindingTableSize);
        rchitRegion.setSize(shaderBindingTableSize);
        vk::StridedDeviceAddressRegionKHR rcallRegion = {};

        return ShaderBindingTableRegions{ rgenRegion, rmissRegion, rchitRegion, rcallRegion };
    }


    void RTXModule::RTXDescriptors(std::vector<vkcv::DescriptorSetHandle>& descriptorSetHandles)
    {
        //TLAS-Descriptor-Write
        TopLevelAccelerationStructure tlas = m_asManager->getTLAS();
        RTXBuffer tlasBuffer = tlas.tlasBuffer;
        vk::WriteDescriptorSetAccelerationStructureKHR AccelerationDescriptor = {};
        AccelerationDescriptor.accelerationStructureCount = 1;
        const TopLevelAccelerationStructure constTLAS = tlas;
        AccelerationDescriptor.pAccelerationStructures = &constTLAS.vulkanHandle;

        vk::WriteDescriptorSet tlasWrite;
        tlasWrite.setPNext(&AccelerationDescriptor);
        tlasWrite.setDstSet(m_core->getDescriptorSet(descriptorSetHandles[0]).vulkanHandle);
        tlasWrite.setDstBinding(1);
        tlasWrite.setDstArrayElement(0);
        tlasWrite.setDescriptorCount(1);
        tlasWrite.setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR);
        m_core->getContext().getDevice().updateDescriptorSets(tlasWrite, nullptr);
        tlasWrite.setDstBinding(2);
        m_core->getContext().getDevice().updateDescriptorSets(tlasWrite, nullptr);

        //INDEX & VERTEX BUFFER
        BottomLevelAccelerationStructure blas = m_asManager->getBLAS(0);//HARD CODED

        //VERTEX BUFFER
        vk::DescriptorBufferInfo vertexInfo = {};
        vertexInfo.setBuffer(blas.vertexBuffer.vulkanHandle);
        vertexInfo.setOffset(0);
        vertexInfo.setRange(blas.vertexBuffer.deviceSize); //maybe check if size is correct

        vk::WriteDescriptorSet vertexWrite;
        vertexWrite.setDstSet(m_core->getDescriptorSet(descriptorSetHandles[0]).vulkanHandle);
        vertexWrite.setDstBinding(3);
        vertexWrite.setDescriptorCount(1);
        vertexWrite.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        vertexWrite.setPBufferInfo(&vertexInfo);
        m_core->getContext().getDevice().updateDescriptorSets(vertexWrite, nullptr);

        //INDEXBUFFER
        vk::DescriptorBufferInfo indexInfo = {};
        indexInfo.setBuffer(blas.indexBuffer.vulkanHandle);
        indexInfo.setOffset(0);
        indexInfo.setRange(blas.indexBuffer.deviceSize); //maybe check if size is correct

        vk::WriteDescriptorSet indexWrite;
        indexWrite.setDstSet(m_core->getDescriptorSet(descriptorSetHandles[0]).vulkanHandle);
        indexWrite.setDstBinding(4);
        indexWrite.setDescriptorCount(1);
        indexWrite.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        indexWrite.setPBufferInfo(&indexInfo);
        m_core->getContext().getDevice().updateDescriptorSets(indexWrite, nullptr);
    }

    void RTXModule::createRTXPipelineAndLayout(uint32_t pushConstantSize, std::vector<DescriptorSetLayoutHandle> descriptorSetLayouts, ShaderProgram &rtxShader) {
        // -- process vkcv::ShaderProgram into vk::ShaderModule
        std::vector<char> rayGenShaderCode = rtxShader.getShader(ShaderStage::RAY_GEN).shaderCode;

        vk::ShaderModuleCreateInfo rayGenShaderModuleInfo(
            vk::ShaderModuleCreateFlags(), // vk::ShaderModuleCreateFlags flags_,
            rayGenShaderCode.size(), // size_t codeSize
            (const uint32_t*)rayGenShaderCode.data() // const uint32_t* pCode
        );
        vk::ShaderModule rayGenShaderModule = m_core->getContext().getDevice().createShaderModule(rayGenShaderModuleInfo);
        if (!rayGenShaderModule) {
            vkcv_log(LogLevel::ERROR, "The Ray Generation Shader Module could not be created!");
        }

        std::vector<char> rayMissShaderCode = rtxShader.getShader(ShaderStage::RAY_MISS).shaderCode;
        vk::ShaderModuleCreateInfo rayMissShaderModuleInfo(
            vk::ShaderModuleCreateFlags(), // vk::ShaderModuleCreateFlags flags_,
            rayMissShaderCode.size(), //size_t codeSize
            (const uint32_t*)rayMissShaderCode.data() // const uint32_t* pCode
        );

        vk::ShaderModule rayMissShaderModule = m_core->getContext().getDevice().createShaderModule(rayMissShaderModuleInfo);
        if (!rayMissShaderModule) {
            vkcv_log(LogLevel::ERROR, "The Ray Miss Shader Module could not be created!");
        }

        std::vector<char> rayClosestHitShaderCode = rtxShader.getShader(ShaderStage::RAY_CLOSEST_HIT).shaderCode;
        vk::ShaderModuleCreateInfo rayClosestHitShaderModuleInfo(
            vk::ShaderModuleCreateFlags(), // vk::ShaderModuleCreateFlags flags_,
            rayClosestHitShaderCode.size(), //size_t codeSize
            (const uint32_t*)rayClosestHitShaderCode.data() // const uint32_t* pCode_
        );
        vk::ShaderModule rayClosestHitShaderModule = m_core->getContext().getDevice().createShaderModule(rayClosestHitShaderModuleInfo);
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
      
        // ray closest hit
        vk::PipelineShaderStageCreateInfo rayClosestHitShaderStageInfo(
            vk::PipelineShaderStageCreateFlags(), // vk::PipelineShaderStageCreateFlags flags_ = {}
            vk::ShaderStageFlagBits::eClosestHitKHR, // vk::ShaderStageFlagBits stage_ = vk::ShaderStageFlagBits::eVertex,
            rayClosestHitShaderModule, // vk::ShaderModule module_ = {},
            "main" // const char* pName_ = {},
        );
        
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {   // HARD CODED. TODO: Support more shader stages.
            rayGenShaderStageInfo, rayMissShaderStageInfo, rayClosestHitShaderStageInfo
        };

        // -- PipelineLayouts

        std::vector<vk::RayTracingShaderGroupCreateInfoKHR> shaderGroups(shaderStages.size());
        // Ray Gen
        shaderGroups[0] = vk::RayTracingShaderGroupCreateInfoKHR(
            vk::RayTracingShaderGroupTypeKHR::eGeneral, // vk::RayTracingShaderGroupTypeKHR type_ = vk::RayTracingShaderGroupTypeKHR::eGeneral
            0, // uint32_t generalShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t closestHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t anyHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t intersectionShader_ = {}
            nullptr // const void* pShaderGroupCaptureReplayHandle_ = {}
        );
        // Ray Miss
        shaderGroups[1] = vk::RayTracingShaderGroupCreateInfoKHR(
            vk::RayTracingShaderGroupTypeKHR::eGeneral, // vk::RayTracingShaderGroupTypeKHR type_ = vk::RayTracingShaderGroupTypeKHR::eGeneral
            1, // uint32_t generalShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t closestHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t anyHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t intersectionShader_ = {}
            nullptr // const void* pShaderGroupCaptureReplayHandle_ = {}
        );
        // Ray Closest Hit
        shaderGroups[2] = vk::RayTracingShaderGroupCreateInfoKHR(
            vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup, // vk::RayTracingShaderGroupTypeKHR type_ = vk::RayTracingShaderGroupTypeKHR::eGeneral
            VK_SHADER_UNUSED_KHR, // uint32_t generalShader_ = {}
            2, // uint32_t closestHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t anyHitShader_ = {}
            VK_SHADER_UNUSED_KHR, // uint32_t intersectionShader_ = {}
            nullptr // const void* pShaderGroupCaptureReplayHandle_ = {}
        );

        std::vector<vk::DescriptorSetLayout> descriptorSetLayoutsVulkan;
        for (size_t i=0; i<descriptorSetLayouts.size(); i++) {
            descriptorSetLayoutsVulkan.push_back(m_core->getDescriptorSetLayout(descriptorSetLayouts[i]).vulkanHandle);
        }

        vk::PushConstantRange pushConstant(
            vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eMissKHR, // vk::ShaderStageFlags stageFlags_ = {},
            0, // uint32_t offset_ = {},
            pushConstantSize // uint32_t size_ = {}
        );

        vk::PipelineLayoutCreateInfo rtxPipelineLayoutCreateInfo(
            vk::PipelineLayoutCreateFlags(), // vk::PipelineLayoutCreateFlags flags_ = {}
            (uint32_t) descriptorSetLayoutsVulkan.size(), // uint32_t setLayoutCount_ = {}     HARD CODED (2)
            descriptorSetLayoutsVulkan.data(), // const vk::DescriptorSetLayout* pSetLayouts_ = {}
            1, //            0, // uint32_t pushConstantRangeCount_ = {}
            &pushConstant //            nullptr // const vk::PushConstantRange* pPushConstantRanges_ = {}
        );

        m_pipelineLayout = m_core->getContext().getDevice().createPipelineLayout(rtxPipelineLayoutCreateInfo);
        if (!m_pipelineLayout) {
            vkcv_log(LogLevel::ERROR, "The RTX Pipeline Layout could not be created!");
        }

        vk::PipelineLibraryCreateInfoKHR rtxPipelineLibraryCreateInfo(
            0, // uint32_t libraryCount_ = {}
            nullptr // const vk::Pipeline* pLibraries_ = {}
        );

        // -- RTX Pipeline

        vk::RayTracingPipelineCreateInfoKHR rtxPipelineInfo(
            vk::PipelineCreateFlags(), // vk::PipelineCreateFlags flags_ = {}
            (uint32_t) shaderStages.size(), // uint32_t stageCount_ = {}
            shaderStages.data(), // const vk::PipelineShaderStageCreateInfo* pStages_ = {}
            (uint32_t) shaderGroups.size(), // uint32_t groupCount_ = {}
            shaderGroups.data(), // const vk::RayTracingShaderGroupCreateInfoKHR* pGroups_ = {}
            16, // uint32_t maxPipelineRayRecursionDepth_ = {}
            &rtxPipelineLibraryCreateInfo, // const vk::PipelineLibraryCreateInfoKHR* pLibraryInfo_ = {}
            nullptr, // const vk::RayTracingPipelineInterfaceCreateInfoKHR* pLibraryInterface_ = {}
            nullptr, // const vk::PipelineDynamicStateCreateInfo* pDynamicState_ = {}
            m_pipelineLayout // vk::PipelineLayout layout_ = {}
        );

        auto pipelineResult = m_core->getContext().getDevice().createRayTracingPipelineKHR(
                vk::DeferredOperationKHR(),
                vk::PipelineCache(),
                rtxPipelineInfo,
                nullptr,
                m_asManager->getDispatcher()
        );

        if (pipelineResult.result != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "The RTX Pipeline could not be created!");
        }

        m_pipeline = pipelineResult.value;

        m_core->getContext().getDevice().destroy(rayGenShaderModule);
        m_core->getContext().getDevice().destroy(rayMissShaderModule);
        m_core->getContext().getDevice().destroy(rayClosestHitShaderModule);

        // TODO: add possibility of more than one shader per stage
        createShaderBindingTable(shaderStages.size());
    }

    vk::Pipeline RTXModule::getPipeline() {
        return m_pipeline;
    }

    vk::Buffer RTXModule::getShaderBindingTableBuffer()
    {
        return m_shaderBindingTableBuffer.vulkanHandle;
    }

    vk::DeviceSize RTXModule::getShaderGroupBaseAlignment()
    {
        return m_shaderGroupBaseAlignment;
    }

    vk::PipelineLayout RTXModule::getPipelineLayout() {
        return m_pipelineLayout;
    }

}