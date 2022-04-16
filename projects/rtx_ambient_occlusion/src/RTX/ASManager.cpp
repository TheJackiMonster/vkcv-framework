#include "ASManager.hpp"
#include <array>

namespace vkcv::rtx {
    

    ASManager::ASManager(vkcv::Core *core) :
    m_core(core),
    m_device(&(core->getContext().getDevice())){
        // INFO: Using RTX extensions implies that we cannot use the standard dispatcher from Vulkan because using RTX
        // specific functions via vk::Device will result in validation errors. Instead we need to use a
        // vk::DispatchLoaderDynamic which is used as dispatcher parameter of the device functions.
        m_rtxDispatcher = vk::DispatchLoaderDynamic( (PFN_vkGetInstanceProcAddr) m_core->getContext().getInstance().getProcAddr("vkGetInstanceProcAddr") );
        m_rtxDispatcher.init(m_core->getContext().getInstance());

        // TODO: Recursive call of buildBLAS for bigger scenes. Currently, the RTX module only supports one mesh.
    }

    ASManager::~ASManager() noexcept {
        m_rtxDispatcher = vk::DispatchLoaderDynamic( (PFN_vkGetInstanceProcAddr) m_core->getContext().getInstance().getProcAddr("vkGetInstanceProcAddr") );
        m_rtxDispatcher.init(m_core->getContext().getInstance());

        // destroy every BLAS, its data containers and free used memory blocks
        for (size_t i=0; i < m_bottomLevelAccelerationStructures.size(); i++) {
            BottomLevelAccelerationStructure blas = m_bottomLevelAccelerationStructures[i];
            m_core->getContext().getDevice().destroyAccelerationStructureKHR(blas.vulkanHandle, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().destroy(blas.accelerationBuffer.vulkanHandle);
            m_core->getContext().getDevice().destroy(blas.indexBuffer.vulkanHandle);
            m_core->getContext().getDevice().destroy(blas.vertexBuffer.vulkanHandle);

            m_core->getContext().getDevice().freeMemory(blas.accelerationBuffer.deviceMemory);
            m_core->getContext().getDevice().freeMemory(blas.indexBuffer.deviceMemory);
            m_core->getContext().getDevice().freeMemory(blas.vertexBuffer.deviceMemory);
        }

        // destroy the TLAS, its data containers and free used memory blocks
        TopLevelAccelerationStructure tlas = m_topLevelAccelerationStructure;
        m_core->getContext().getDevice().destroyAccelerationStructureKHR(tlas.vulkanHandle, nullptr, m_rtxDispatcher);
        m_core->getContext().getDevice().destroy(tlas.tlasBuffer.vulkanHandle);
        m_core->getContext().getDevice().destroy(tlas.gpuBufferInstances.vulkanHandle);

        m_core->getContext().getDevice().freeMemory(tlas.tlasBuffer.deviceMemory);
        m_core->getContext().getDevice().freeMemory(tlas.gpuBufferInstances.deviceMemory);
    }


  
    vk::CommandPool ASManager::createCommandPool() {
        vk::CommandPool commandPool;
        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.setQueueFamilyIndex(m_core->getContext().getQueueManager().getComputeQueues()[0].familyIndex);
        vk::Result res = m_device->createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool);
        if (res != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "ASManager: command pool could not be created! (%s)", vk::to_string(res).c_str());
        }
        return commandPool;
    };

    vk::CommandBuffer ASManager::createAndBeginCommandBuffer(vk::CommandPool commandPool)
    {
        vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
        commandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary);
        commandBufferAllocateInfo.setCommandPool(commandPool);
        commandBufferAllocateInfo.setCommandBufferCount(1);
        vk::CommandBuffer commandBuffer;
        vk::Result result = m_device->allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer);
        if (result != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "ASManager: command buffer for Acceleration Strucutre Build could not be allocated! (%s)", vk::to_string(result).c_str());
        }

        beginCommandBuffer(commandBuffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        return commandBuffer;
    }

    void ASManager::submitCommandBuffer(vk::CommandPool commandPool, vk::CommandBuffer& commandBuffer)
    {
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.setCommandBufferCount(1);
        submitInfo.setPCommandBuffers(&commandBuffer);

        vkcv::Queue queue = m_core->getContext().getQueueManager().getComputeQueues()[0];

        queue.handle.submit(submitInfo);
        queue.handle.waitIdle();

        m_device->freeCommandBuffers(commandPool, 1, &commandBuffer);
        m_device->destroyCommandPool(commandPool);
    }

    vk::DeviceAddress ASManager::getBufferDeviceAddress(vk::Buffer buffer)
    {
        vk::BufferDeviceAddressInfo bufferDeviceAddressInfo(buffer);
        return m_device->getBufferAddress(bufferDeviceAddressInfo);
    }

    void ASManager::createBuffer(RTXBuffer& buffer) {

        vk::BufferCreateInfo bufferCreateInfo;
        bufferCreateInfo.setFlags(vk::BufferCreateFlags());
        bufferCreateInfo.setUsage(buffer.bufferUsageFlagBits);
        bufferCreateInfo.setSize(buffer.deviceSize);

        buffer.vulkanHandle = m_device->createBuffer(bufferCreateInfo);
        vk::MemoryRequirements2 memoryRequirements2;
        vk::MemoryDedicatedRequirements dedicatedRequirements;
        vk::BufferMemoryRequirementsInfo2 bufferRequirements;

        bufferRequirements.setBuffer(buffer.vulkanHandle);
        memoryRequirements2.pNext = &dedicatedRequirements;
        m_device->getBufferMemoryRequirements2(&bufferRequirements, &memoryRequirements2); 

        vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = m_core->getContext().getPhysicalDevice().getMemoryProperties();

        uint32_t memoryTypeIndex = -1;
        for (size_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
            if ((memoryRequirements2.memoryRequirements.memoryTypeBits & (1 << i))
                    && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & buffer.memoryPropertyFlagBits) == buffer.memoryPropertyFlagBits) {
                memoryTypeIndex = i;
                break;
            }
        }

        vk::MemoryAllocateInfo memoryAllocateInfo(
            memoryRequirements2.memoryRequirements.size,  // size of allocation in bytes
            memoryTypeIndex // index identifying a memory type from the memoryTypes array of the vk::PhysicalDeviceMemoryProperties structure.
        );
        vk::MemoryAllocateFlagsInfo allocateFlagsInfo(
            vk::MemoryAllocateFlagBits::eDeviceAddress  // vk::MemoryAllocateFlags
        );
        memoryAllocateInfo.setPNext(&allocateFlagsInfo);  // extend memory allocate info with allocate flag info
        buffer.deviceMemory = m_device->allocateMemory(memoryAllocateInfo);

        uint32_t memoryOffset = 0;
        m_device->bindBufferMemory(buffer.vulkanHandle, buffer.deviceMemory, memoryOffset);

        // only fill data in case of CPU buffer
        if (buffer.bufferType == RTXBufferType::STAGING) {
            void* mapped = m_device->mapMemory(buffer.deviceMemory, memoryOffset, buffer.deviceSize);
            std::memcpy(mapped, buffer.data, buffer.deviceSize);
            m_device->unmapMemory(buffer.deviceMemory);
        }
    }

    void ASManager::copyFromCPUToGPU(RTXBuffer& cpuBuffer, RTXBuffer& gpuBuffer) {
        SubmitInfo submitInfo;
        submitInfo.queueType = QueueType::Graphics;

        vk::CommandPool commandPool= createCommandPool();
        vk::CommandBuffer commandBuffer= createAndBeginCommandBuffer(commandPool);
        vk::BufferCopy bufferCopy;
        bufferCopy.size = cpuBuffer.deviceSize;
        commandBuffer.copyBuffer(cpuBuffer.vulkanHandle, gpuBuffer.vulkanHandle, 1, &bufferCopy);

        submitCommandBuffer(commandPool,commandBuffer);     

        m_device->destroyBuffer(cpuBuffer.vulkanHandle);
        m_device->freeMemory(cpuBuffer.deviceMemory);
    }

    void ASManager::buildBLAS(RTXBuffer vertexBuffer, uint32_t vertexCount, RTXBuffer indexBuffer, uint32_t indexCount) {
        // TODO: organize hierarchical structure of multiple BLAS

        vk::DeviceAddress vertexBufferAddress = getBufferDeviceAddress(vertexBuffer.vulkanHandle);
        vk::DeviceAddress indexBufferAddress = getBufferDeviceAddress(indexBuffer.vulkanHandle);

        // triangle mesh data
        vk::AccelerationStructureGeometryTrianglesDataKHR asTriangles(
                vk::Format::eR32G32B32Sfloat,   // vertex format
                vertexBufferAddress, // vertex buffer address (vk::DeviceOrHostAddressConstKHR)
                3 * sizeof(float), // vertex stride (vk::DeviceSize)
                uint32_t(vertexCount - 1), // maxVertex (uint32_t)
                vk::IndexType::eUint32, // indexType (vk::IndexType) --> INFO: UINT16 oder UINT32!
                indexBufferAddress, // indexData (vk::DeviceOrHostAddressConstKHR)
                {} // transformData (vk::DeviceOrHostAddressConstKHR)
        );

        // Geometry data
        vk::AccelerationStructureGeometryKHR asGeometry(
                vk::GeometryTypeKHR::eTriangles, // The geometry type, e.g. triangles, AABBs, instances
                asTriangles, // the geometry data
                vk::GeometryFlagBitsKHR::eOpaque // This flag disables any-hit shaders to increase ray tracing performance
        );

        // Ranges for data lists
        vk::AccelerationStructureBuildRangeInfoKHR asRangeInfo(
                uint32_t(indexCount / 3), // the primitiveCount (uint32_t)
                0, // primitiveOffset (uint32_t)
                0, // firstVertex (uint32_t)
                0  // transformOffset (uint32_t)
        );

        // Settings and array of geometries to build into BLAS
        vk::AccelerationStructureBuildGeometryInfoKHR asBuildInfo(
                vk::AccelerationStructureTypeKHR::eBottomLevel, // type of the AS: bottom vs. top
                vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace, // some flags for different purposes, e.g. efficiency
                vk::BuildAccelerationStructureModeKHR::eBuild, // AS mode: build vs. update
                {}, // src AS (this seems to be for copying AS)
                {}, // dst AS (this seems to be for copying AS)
                1, // the geometryCount. TODO: how many do we need?
                &asGeometry // the next input entry would be a pointer to a pointer to geometries. Maybe geometryCount depends on the next entry?
        );

        // Calculate memory needed for AS
        vk::AccelerationStructureBuildSizesInfoKHR asBuildSizesInfo;
        m_device->getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice, // build on device instead of host
                &asBuildInfo, // pointer to build info
                &asRangeInfo.primitiveCount, // array of number of primitives per geometry
                &asBuildSizesInfo,  // output pointer to store sizes
                m_rtxDispatcher
                );

        // create buffer for acceleration structure
        RTXBuffer blasBuffer;
        blasBuffer.bufferType = RTXBufferType::ACCELERATION;
        blasBuffer.deviceSize = asBuildSizesInfo.accelerationStructureSize;
        blasBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
                | vk::BufferUsageFlagBits::eShaderDeviceAddress
                | vk::BufferUsageFlagBits::eStorageBuffer;
        blasBuffer.memoryPropertyFlagBits = { vk::MemoryPropertyFlagBits::eDeviceLocal };

        createBuffer(blasBuffer); 

        // Create an empty AS object
        vk::AccelerationStructureCreateInfoKHR asCreateInfo(
            vk::AccelerationStructureCreateFlagsKHR(), // creation flags
            blasBuffer.vulkanHandle, // allocated AS buffer.
            0,
            asBuildSizesInfo.accelerationStructureSize, // size of the AS
            asBuildInfo.type // type of the AS
        );

        // Create the intended AS object
        vk::AccelerationStructureKHR blasKHR;
        vk::Result res = m_device->createAccelerationStructureKHR(
            &asCreateInfo, // AS create info
            nullptr, // allocator callbacks
            &blasKHR, // the AS
            m_rtxDispatcher
        );
        if (res != vk::Result::eSuccess) {
            vkcv_log(vkcv::LogLevel::ERROR, "The Bottom Level Acceleration Structure could not be build! (%s)", vk::to_string(res).c_str());
        }
        asBuildInfo.setDstAccelerationStructure(blasKHR);

        // Create temporary scratch buffer used for building the AS
        RTXBuffer scratchBuffer;
        scratchBuffer.bufferType = RTXBufferType::SCRATCH;
        scratchBuffer.deviceSize = asBuildSizesInfo.buildScratchSize;
        scratchBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eShaderDeviceAddress
            | vk::BufferUsageFlagBits::eStorageBuffer;
        scratchBuffer.memoryPropertyFlagBits = { vk::MemoryPropertyFlagBits::eDeviceLocal };

        createBuffer(scratchBuffer);

        asBuildInfo.setScratchData(getBufferDeviceAddress(scratchBuffer.vulkanHandle));

        // Pointer to rangeInfo, used later for build
        vk::AccelerationStructureBuildRangeInfoKHR* pointerToRangeInfo = &asRangeInfo;

        vk::CommandPool commandPool = createCommandPool();
        vk::CommandBuffer commandBuffer = createAndBeginCommandBuffer(commandPool);

        commandBuffer.buildAccelerationStructuresKHR(1, &asBuildInfo, &pointerToRangeInfo, m_rtxDispatcher);

        submitCommandBuffer(commandPool, commandBuffer);
  
        m_core->getContext().getDevice().destroyBuffer(scratchBuffer.vulkanHandle, nullptr, m_rtxDispatcher);
        m_core->getContext().getDevice().freeMemory(scratchBuffer.deviceMemory, nullptr, m_rtxDispatcher);
                
        BottomLevelAccelerationStructure blas = {
                vertexBuffer,
                indexBuffer,
                blasBuffer,
                blasKHR
        };
        m_bottomLevelAccelerationStructures.push_back(blas);
    }

    void ASManager::buildTLAS() {
        // TODO: organize hierarchical structure of multiple BLAS

        // We need an the device address of each BLAS --> TODO: for loop for bigger scenes
        vk::AccelerationStructureDeviceAddressInfoKHR addressInfo(
            m_bottomLevelAccelerationStructures[0].vulkanHandle
        );
        vk::DeviceAddress blasAddress = m_device->getAccelerationStructureAddressKHR(&addressInfo, m_rtxDispatcher);

        std::array<std::array<float, 4>, 3> transformMatrix = {
                std::array<float, 4>{1.f, 0.f, 0.f, 0.f},
                std::array<float, 4>{0.f, 1.f, 0.f, 0.f},
                std::array<float, 4>{0.f, 0.f, 1.f, 0.f},
        };

        vk::TransformMatrixKHR transformMatrixKhr(
            transformMatrix   // std::array<std::array<float,4>,3> const&
        );

        vk::AccelerationStructureInstanceKHR accelerationStructureInstanceKhr(
            transformMatrixKhr,    // vk::TransformMatrixKHR transform_ = {},
            0,   // uint32_t instanceCustomIndex,
            0xFF,    //uint32_t mask_ = {},
            0,  // uint32_t instanceShaderBindingTableRecordOffset,
            vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable,    // vk::GeometryInstanceFlagsKHR
            blasAddress // uint64_t accelerationStructureReference (the device address of the BLAS)
        );

        // create a buffer of instances on the device and upload the array of instances to it
        RTXBuffer stagingBufferInstances;
        stagingBufferInstances.bufferType = RTXBufferType::STAGING;
        stagingBufferInstances.deviceSize = sizeof(accelerationStructureInstanceKhr);
        stagingBufferInstances.data = &accelerationStructureInstanceKhr;
        stagingBufferInstances.bufferUsageFlagBits = vk::BufferUsageFlagBits::eShaderDeviceAddress
            | vk::BufferUsageFlagBits::eTransferSrc;
        stagingBufferInstances.memoryPropertyFlagBits = vk::MemoryPropertyFlagBits::eHostCoherent
            | vk::MemoryPropertyFlagBits::eHostVisible;

        createBuffer(stagingBufferInstances);

        RTXBuffer bufferInstances;
        bufferInstances.bufferType = RTXBufferType::GPU;
        bufferInstances.deviceSize = sizeof(accelerationStructureInstanceKhr);
        bufferInstances.bufferUsageFlagBits = vk::BufferUsageFlagBits::eShaderDeviceAddress
            | vk::BufferUsageFlagBits::eTransferDst
			| vk::BufferUsageFlagBits::eTransferSrc
			| vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;
        bufferInstances.memoryPropertyFlagBits = vk::MemoryPropertyFlagBits::eDeviceLocal;

        createBuffer(bufferInstances);
        copyFromCPUToGPU(stagingBufferInstances, bufferInstances);   // automatically deletes and frees memory of stagingBufferInstances

        vk::MemoryBarrier barrier;
        barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        barrier.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR);
        vk::CommandPool commandPool = createCommandPool();
        vk::CommandBuffer commandBuffer = createAndBeginCommandBuffer(commandPool);
        commandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
            {},1,&barrier,0, nullptr,0,nullptr);
        submitCommandBuffer(commandPool, commandBuffer);


        // ranging information for TLAS build
        vk::AccelerationStructureBuildRangeInfoKHR asRangeInfo(
            1, // primitiveCount -> number of instances
            0, // primitiveOffset
            0, // firstVertex
            0 //transformOffset
        );

        vk::DeviceAddress bufferInstancesAddress = getBufferDeviceAddress(bufferInstances.vulkanHandle);

        vk::AccelerationStructureGeometryInstancesDataKHR asInstances(
            false,    // vk::Bool32 arrayOfPointers
            bufferInstancesAddress    // vk::DeviceOrHostAddressConstKHR data_ = {}
        );

        // Geometry, in this case instances of BLAS
        vk::AccelerationStructureGeometryKHR asGeometry(
            vk::GeometryTypeKHR::eInstances,    // vk::GeometryTypeKHR geometryType_ = vk::GeometryTypeKHR::eTriangles
            asInstances,    // vk::AccelerationStructureGeometryDataKHR geometry_ = {}
            {}    // vk::GeometryFlagsKHR flags_ = {}
        );

        // Finally, create the TLAS
        vk::AccelerationStructureBuildGeometryInfoKHR asBuildInfo(
            vk::AccelerationStructureTypeKHR::eTopLevel, // type of the AS: bottom vs. top
            vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace, // some flags for different purposes, e.g. efficiency
            vk::BuildAccelerationStructureModeKHR::eBuild, // AS mode: build vs. update
            {}, // src AS (this seems to be for copying AS)
            {}, // dst AS (this seems to be for copying AS)
            1, // the geometryCount.
            &asGeometry // the next input entry would be a pointer to a pointer to geometries. Maybe geometryCount depends on the next entry?
        );

        // AS size and scratch space size, given by count of instances (1)
        vk::AccelerationStructureBuildSizesInfoKHR asSizeInfo;
        m_core->getContext().getDevice().getAccelerationStructureBuildSizesKHR(
            vk::AccelerationStructureBuildTypeKHR::eDevice,
            &asBuildInfo,
            &asRangeInfo.primitiveCount,
            &asSizeInfo,
            m_rtxDispatcher
        );

        // Create buffer for the TLAS
        RTXBuffer tlasBuffer;
        tlasBuffer.bufferType = RTXBufferType::ACCELERATION;
        tlasBuffer.deviceSize = asSizeInfo.accelerationStructureSize;
        tlasBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
            | vk::BufferUsageFlagBits::eShaderDeviceAddress
            | vk::BufferUsageFlagBits::eStorageBuffer;

        createBuffer(tlasBuffer);

        // Create empty TLAS object
        vk::AccelerationStructureCreateInfoKHR asCreateInfo(
            {}, // creation flags
            tlasBuffer.vulkanHandle, // allocated AS buffer.
            0,
            asSizeInfo.accelerationStructureSize, // size of the AS
            asBuildInfo.type // type of the AS
        );

        vk::AccelerationStructureKHR tlas;
        vk::Result res = m_device->createAccelerationStructureKHR(&asCreateInfo, nullptr, &tlas, m_rtxDispatcher);
        if (res != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "Top Level Acceleration Structure could not be created! (%s)", vk::to_string(res).c_str());
        }
        asBuildInfo.setDstAccelerationStructure(tlas);

        // Create temporary scratch buffer used for building the AS
        RTXBuffer tlasScratchBuffer;  // scratch buffer
        tlasScratchBuffer.bufferType = RTXBufferType::ACCELERATION;
        tlasScratchBuffer.deviceSize = asSizeInfo.buildScratchSize;
        tlasScratchBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eShaderDeviceAddressKHR
            | vk::BufferUsageFlagBits::eStorageBuffer;

        createBuffer(tlasScratchBuffer);

        vk::BufferDeviceAddressInfo tempBuildDataBufferDeviceAddressInfo(tlasScratchBuffer.vulkanHandle);
        vk::DeviceAddress tempBuildBufferDataAddress = m_core->getContext().getDevice().getBufferAddressKHR(tempBuildDataBufferDeviceAddressInfo, m_rtxDispatcher);
        asBuildInfo.setScratchData(tempBuildBufferDataAddress);

        // Pointer to rangeInfo, used later for build
        vk::AccelerationStructureBuildRangeInfoKHR* pointerToRangeInfo = &asRangeInfo;

        // Build the TLAS.

        vk::CommandPool commandPool2 = createCommandPool();
        vk::CommandBuffer commandBuffer2 = createAndBeginCommandBuffer(commandPool2);
        commandBuffer2.buildAccelerationStructuresKHR(1, &asBuildInfo, &pointerToRangeInfo, m_rtxDispatcher);
        submitCommandBuffer(commandPool2, commandBuffer2);
        
        m_device->destroyBuffer(tlasScratchBuffer.vulkanHandle, nullptr, m_rtxDispatcher);
        m_device->freeMemory(tlasScratchBuffer.deviceMemory, nullptr, m_rtxDispatcher);

        m_topLevelAccelerationStructure = {
                bufferInstances,
                tlasBuffer,
                tlasScratchBuffer,
                tlas
        };
    }

    TopLevelAccelerationStructure ASManager::getTLAS()
    {
        return m_topLevelAccelerationStructure;
    }

    BottomLevelAccelerationStructure ASManager::getBLAS(uint32_t id)
    {
        return m_bottomLevelAccelerationStructures[id];
    }

    const vk::DispatchLoaderDynamic& ASManager::getDispatcher() {
        return m_rtxDispatcher;
    }    
}