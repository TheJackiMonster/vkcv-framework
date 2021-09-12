#include "vkcv/rtx/ASManager.hpp"
#include <array>

namespace vkcv::rtx {

    ASManager::ASManager(vkcv::Core *core) :
    m_core(core) {
        // INFO: It seems that we need a dynamic dispatch loader because Vulkan is an ASs ...
        m_rtxDispatcher = vk::DispatchLoaderDynamic( (PFN_vkGetInstanceProcAddr) m_core->getContext().getInstance().getProcAddr("vkGetInstanceProcAddr") );
        m_rtxDispatcher.init(m_core->getContext().getInstance());

        // SUGGESTION: recursive call of buildBLAS etc.

    }

    ASManager::~ASManager() noexcept {
        // destroy every BLAS, its data containers and free used memory blocks
        for (size_t i=0; i < m_bottomLevelAccelerationStructures.size(); i++) {
            BottomLevelAccelerationStructure blas = m_bottomLevelAccelerationStructures[i];
            m_core->getContext().getDevice().destroyAccelerationStructureKHR(blas.vulkanHandle, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().destroy(blas.accelerationBuffer.vulkanHandle, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().destroy(blas.indexBuffer.vulkanHandle, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().destroy(blas.vertexBuffer.vulkanHandle, nullptr, m_rtxDispatcher);

            m_core->getContext().getDevice().freeMemory(blas.accelerationBuffer.deviceMemory, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().freeMemory(blas.indexBuffer.deviceMemory, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().freeMemory(blas.vertexBuffer.deviceMemory, nullptr, m_rtxDispatcher);
        }

        // destroy the TLAS, its data containers and free used memory blocks
        TopLevelAccelerationStructure tlas = m_topLevelAccelerationStructure;
        m_core->getContext().getDevice().destroyAccelerationStructureKHR(tlas.vulkanHandle, nullptr, m_rtxDispatcher);
        m_core->getContext().getDevice().destroy(tlas.tempBuildDataBuffer.vulkanHandle, nullptr, m_rtxDispatcher);
        m_core->getContext().getDevice().destroy(tlas.tlasBuffer.vulkanHandle, nullptr, m_rtxDispatcher);
        m_core->getContext().getDevice().destroy(tlas.gpuBufferInstances.vulkanHandle, nullptr, m_rtxDispatcher);

        m_core->getContext().getDevice().freeMemory(tlas.tempBuildDataBuffer.deviceMemory, nullptr, m_rtxDispatcher);
        m_core->getContext().getDevice().freeMemory(tlas.tlasBuffer.deviceMemory, nullptr, m_rtxDispatcher);
        m_core->getContext().getDevice().freeMemory(tlas.gpuBufferInstances.deviceMemory, nullptr, m_rtxDispatcher);
    }

    void ASManager::buildTLAS() {
        // We need an the device address of each BLAS --> TODO: for loop
        vk::AccelerationStructureDeviceAddressInfoKHR addressInfo(
                m_bottomLevelAccelerationStructures[0].vulkanHandle
                );
        vk::DeviceAddress blasAddress = m_core->getContext().getDevice().getAccelerationStructureAddressKHR(&addressInfo, m_rtxDispatcher);

        std::array<std::array<float,4>,3> transformMatrix = {
                std::array<float, 4>{1.f, 0.f, 0.f, 0.f},
                std::array<float, 4>{0.f, 1.f, 0.f, 0.f},
                std::array<float, 4>{0.f, 0.f, 1.f, 0.f},
        };

        vk::TransformMatrixKHR transformMatrixKhr(
                transformMatrix   // std::array<std::array<float,4>,3> const&
                );

        vk::AccelerationStructureInstanceKHR accelerationStructureInstanceKhr (
                transformMatrixKhr,    // vk::TransformMatrixKHR transform_ = {},
                0,   // uint32_t instanceCustomIndex,
                0xFF,    //uint32_t mask_ = {},
                0,  // uint32_t instanceShaderBindingTableRecordOffset,
                vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable,    // vk::GeometryInstanceFlagsKHR
                blasAddress // uint64_t accelerationStructureReference (the device address of the BLAS)
        );

        // create a buffer of instances on the device and upload the array of instances to it
        RTXBuffer cpuBufferInstances;
        cpuBufferInstances.bufferType = RTXBufferType::CPU;
        cpuBufferInstances.deviceSize = sizeof(accelerationStructureInstanceKhr);
        cpuBufferInstances.data = &accelerationStructureInstanceKhr;
        cpuBufferInstances.bufferUsageFlagBits = vk::BufferUsageFlagBits::eShaderDeviceAddressKHR
                | vk::BufferUsageFlagBits::eTransferSrc;
        cpuBufferInstances.memoryPropertyFlagBits = vk::MemoryPropertyFlagBits::eHostCoherent
                | vk::MemoryPropertyFlagBits::eHostVisible;

        createBuffer(cpuBufferInstances);

        RTXBuffer gpuBufferInstances;
        gpuBufferInstances.bufferType = RTXBufferType::GPU;
        gpuBufferInstances.deviceSize = sizeof(accelerationStructureInstanceKhr);
        gpuBufferInstances.bufferUsageFlagBits = vk::BufferUsageFlagBits::eShaderDeviceAddressKHR
                | vk::BufferUsageFlagBits::eTransferDst;
        gpuBufferInstances.memoryPropertyFlagBits = vk::MemoryPropertyFlagBits::eDeviceLocal;

        createBuffer(gpuBufferInstances);
        copyFromCPUToGPU(cpuBufferInstances, gpuBufferInstances);   // automatically deletes and frees memory of cpuBufferInstances

        // ranging information for TLAS build
        vk::AccelerationStructureBuildRangeInfoKHR asRangeInfo(
                1, // primitiveCount -> number of instances
                0, // primitiveOffset
                0, // firstVertex
                0 //transformOffset
                );

        vk::BufferDeviceAddressInfo bufferInstancesDeviceAddressInfo(gpuBufferInstances.vulkanHandle);
        vk::DeviceAddress bufferInstancesAddress = m_core->getContext().getDevice().getBufferAddressKHR(bufferInstancesDeviceAddressInfo, m_rtxDispatcher);
        vk::DeviceOrHostAddressConstKHR bufferInstancesDeviceOrHostAddressConst(bufferInstancesAddress);

        vk::AccelerationStructureGeometryInstancesDataKHR asInstances(
                false,    // vk::Bool32 arrayOfPointers
                bufferInstancesDeviceOrHostAddressConst    // vk::DeviceOrHostAddressConstKHR data_ = {}
                );

        // Like creating the BLAS, point to the geometry (in this case, the instances) in a polymorphic object.
        vk::AccelerationStructureGeometryKHR asGeometry(
                vk::GeometryTypeKHR::eInstances,    // vk::GeometryTypeKHR geometryType_ = vk::GeometryTypeKHR::eTriangles
                asInstances,    // vk::AccelerationStructureGeometryDataKHR geometry_ = {}
                {}    // vk::GeometryFlagsKHR flags_ = {}
                );

        // Finally, create the TLAS (--> ASs)
        vk::AccelerationStructureBuildGeometryInfoKHR asBuildInfo(
                vk::AccelerationStructureTypeKHR::eTopLevel, // type of the AS: bottom vs. top
                vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace, // some flags for different purposes, e.g. efficiency
                vk::BuildAccelerationStructureModeKHR::eBuild, // AS mode: build vs. update
                {}, // src AS (this seems to be for copying AS)
                {}, // dst AS (this seems to be for copying AS)
                1, // the geometryCount. TODO: how many do we need?
                &asGeometry // the next input entry would be a pointer to a pointer to geometries. Maybe geometryCount depends on the next entry?
                );

        // Query the worst -case AS size and scratch space size based on the number of instances (in this case, 1).
        vk::AccelerationStructureBuildSizesInfoKHR asSizeInfo;
        m_core->getContext().getDevice().getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice,
                &asBuildInfo,
                &asRangeInfo.primitiveCount,
                &asSizeInfo,
                m_rtxDispatcher
                );

        // Allocate a buffer for the acceleration structure.
        RTXBuffer tlasBuffer;
        tlasBuffer.bufferType = RTXBufferType::ACCELERATION;
        tlasBuffer.deviceSize = asSizeInfo.accelerationStructureSize;
        tlasBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
                | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR
                | vk::BufferUsageFlagBits::eStorageBuffer;

        createBuffer(tlasBuffer);

        // Create the acceleration structure object. (Data has not yet been set.)
        vk::AccelerationStructureCreateInfoKHR asCreateInfo(
                {}, // creation flags
                tlasBuffer.vulkanHandle, // allocated AS buffer.
                0,
                asSizeInfo.accelerationStructureSize, // size of the AS
                asBuildInfo.type // type of the AS
                );

        vk::AccelerationStructureKHR tlas;
        vk::Result res = m_core->getContext().getDevice().createAccelerationStructureKHR(&asCreateInfo, nullptr, &tlas, m_rtxDispatcher);
        if (res != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "Top Level Acceleration Structure could not be created! (%s)", vk::to_string(res).c_str());
        }
        asBuildInfo.setDstAccelerationStructure(tlas);

        // Allocate the scratch buffer holding temporary build data.
        RTXBuffer tempBuildDataBuffer;  // scratch buffer
        tempBuildDataBuffer.bufferType = RTXBufferType::ACCELERATION;
        tempBuildDataBuffer.deviceSize = asSizeInfo.buildScratchSize;
        tempBuildDataBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eShaderDeviceAddressKHR
                | vk::BufferUsageFlagBits::eStorageBuffer;

        createBuffer(tempBuildDataBuffer);

        vk::BufferDeviceAddressInfo tempBuildDataBufferDeviceAddressInfo(tempBuildDataBuffer.vulkanHandle);
        vk::DeviceAddress tempBuildBufferDataAddress = m_core->getContext().getDevice().getBufferAddressKHR(tempBuildDataBufferDeviceAddressInfo, m_rtxDispatcher);
        asBuildInfo.scratchData.deviceAddress = tempBuildBufferDataAddress;

        // Create a one-element array of pointers to range info objects.
        vk::AccelerationStructureBuildRangeInfoKHR* pRangeInfo = &asRangeInfo;

        // Build the TLAS.
        vk::CommandPool commandPool;

        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.queueFamilyIndex = m_core->getContext().getQueueManager().getGraphicsQueues()[0].familyIndex;

        res = m_core->getContext().getDevice().createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool);
        if (res != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "ASManager: command pool could not be created! (%s)", vk::to_string(res).c_str());
        }

        vk::CommandBufferAllocateInfo bufferAllocateInfo;
        bufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        bufferAllocateInfo.commandPool = commandPool;
        bufferAllocateInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        res = m_core->getContext().getDevice().allocateCommandBuffers(&bufferAllocateInfo, &commandBuffer);
        if (res != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "ASManager: command buffer could not be allocated! (%s)", vk::to_string(res).c_str());
        }

        beginCommandBuffer(commandBuffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        commandBuffer.buildAccelerationStructuresKHR(1, &asBuildInfo, &pRangeInfo, m_rtxDispatcher);
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        Queue graphicsQueue = m_core->getContext().getQueueManager().getGraphicsQueues()[0];

        graphicsQueue.handle.submit(submitInfo);
        graphicsQueue.handle.waitIdle();

        m_core->getContext().getDevice().freeCommandBuffers(commandPool, 1, &commandBuffer, m_rtxDispatcher);
        m_core->getContext().getDevice().destroyCommandPool(commandPool, nullptr, m_rtxDispatcher);

        m_topLevelAccelerationStructure = {
                gpuBufferInstances,
                tlasBuffer,
                tempBuildDataBuffer,
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


    void ASManager::buildBLAS(std::vector<uint8_t> &vertices, std::vector<uint8_t> &indices) {
        uint32_t vertexCount = vertices.size();
        uint32_t indexCount = indices.size();
        RTXBuffer vertexBuffer = makeBufferFromData(vertices);
        RTXBuffer indexBuffer = makeBufferFromData(indices);

        vk::BufferDeviceAddressInfo vertexBufferDeviceAddressInfo(vertexBuffer.vulkanHandle);
        vk::DeviceAddress vertexBufferAddress = m_core->getContext().getDevice().getBufferAddressKHR(vertexBufferDeviceAddressInfo, m_rtxDispatcher);
        vk::DeviceOrHostAddressConstKHR vertexDeviceOrHostAddressConst(vertexBufferAddress);

        vk::BufferDeviceAddressInfo indexBufferDeviceAddressInfo(indexBuffer.vulkanHandle);
        vk::DeviceAddress indexBufferAddress = m_core->getContext().getDevice().getBufferAddressKHR(indexBufferDeviceAddressInfo, m_rtxDispatcher);
        vk::DeviceOrHostAddressConstKHR indexDeviceOrHostAddressConst(indexBufferAddress);

        // Specify triangle mesh data
        vk::AccelerationStructureGeometryTrianglesDataKHR asTriangles(
                vk::Format::eR32G32B32Sfloat,   // vertex format
                vertexDeviceOrHostAddressConst, // vertex buffer address (vk::DeviceOrHostAddressConstKHR)
                3 * sizeof(float), // vertex stride (vk::DeviceSize)
                uint32_t(vertexCount - 1), // maxVertex (uint32_t)
                vk::IndexType::eUint16, // indexType (vk::IndexType) --> INFO: UINT16 oder UINT32!
                indexDeviceOrHostAddressConst, // indexData (vk::DeviceOrHostAddressConstKHR)
                {} // transformData (vk::DeviceOrHostAddressConstKHR)
        );

        // Encapsulate geometry data
        vk::AccelerationStructureGeometryKHR asGeometry(
                vk::GeometryTypeKHR::eTriangles, // The geometry type, e.g. triangles, AABBs, instances
                asTriangles, // the geometry data
                vk::GeometryFlagBitsKHR::eOpaque // This flag disables any-hit shaders to increase ray tracing performance
        );

        // List ranges of data for access
        vk::AccelerationStructureBuildRangeInfoKHR asRangeInfo(
                uint32_t(indexCount / 3), // the primitiveCount (uint32_t)
                0, // primitiveOffset (uint32_t)
                0, // firstVertex (uint32_t)
                0  // transformOffset (uint32_t)
        );

        // Query the worst-case amount of memory needed for the AS
        vk::AccelerationStructureBuildGeometryInfoKHR asBuildInfo(
                vk::AccelerationStructureTypeKHR::eBottomLevel, // type of the AS: bottom vs. top
                vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace, // some flags for different purposes, e.g. efficiency
                vk::BuildAccelerationStructureModeKHR::eBuild, // AS mode: build vs. update
                {}, // src AS (this seems to be for copying AS)
                {}, // dst AS (this seems to be for copying AS)
                1, // the geometryCount. TODO: how many do we need?
                &asGeometry // the next input entry would be a pointer to a pointer to geometries. Maybe geometryCount depends on the next entry?
        );
        vk::AccelerationStructureBuildSizesInfoKHR asBuildSizesInfo;
        m_core->getContext().getDevice().getAccelerationStructureBuildSizesKHR(
                vk::AccelerationStructureBuildTypeKHR::eDevice, // build on device instead of host
                &asBuildInfo, // pointer to build info
                &asRangeInfo.primitiveCount, // array of number of primitives per geometry
                &asBuildSizesInfo,  // output pointer to store sizes
                m_rtxDispatcher
                );

        // create buffer for acceleration structure
        RTXBuffer blasBuffer; // scratch Buffer
        blasBuffer.bufferType = RTXBufferType::ACCELERATION;
        blasBuffer.deviceSize = asBuildSizesInfo.accelerationStructureSize;
        blasBuffer.data = nullptr;
        blasBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
                | vk::BufferUsageFlagBits::eShaderDeviceAddress
                | vk::BufferUsageFlagBits::eStorageBuffer;
        blasBuffer.memoryPropertyFlagBits = {};

        createBuffer(blasBuffer);   // TODO: is this on GPU?

        // Create an empty AS object
        vk::AccelerationStructureCreateInfoKHR asCreateInfo(
                {}, // creation flags
                blasBuffer.vulkanHandle, // allocated AS buffer.
                0,
                asBuildSizesInfo.accelerationStructureSize, // size of the AS
                asBuildInfo.type // type of the AS
        );

        // Create the intended AS object
        vk::AccelerationStructureKHR blasKHR;
        vk::Result res = m_core->getContext().getDevice().createAccelerationStructureKHR(
                &asCreateInfo, // AS create info
                nullptr, // allocator callbacks
                &blasKHR, // the AS
                m_rtxDispatcher
        );
        if(res != vk::Result::eSuccess) {
            vkcv_log(vkcv::LogLevel::ERROR, "The Bottom Level Acceleration Structure could not be build! (%s)", vk::to_string(res).c_str());
        }
        asBuildInfo.setDstAccelerationStructure(blasKHR);

        BottomLevelAccelerationStructure blas = {
                vertexBuffer,
                indexBuffer,
                blasBuffer,
                blasKHR
        };
        m_bottomLevelAccelerationStructures.push_back(blas);
    }

    RTXBuffer ASManager::makeBufferFromData(std::vector<uint8_t> &data) {
        // convert uint8_t data into unit16_t
        std::vector<uint16_t> data_converted;
        for (size_t i=0; i<data.size(); i++) {
            data_converted.push_back((uint16_t)data[i]);
        }

        // now create a vk::Buffer of type uint16_t for the data

        // first: Staging Buffer creation
        RTXBuffer cpuBuffer;
        cpuBuffer.bufferType = RTXBufferType::CPU;
        cpuBuffer.deviceSize = sizeof(data_converted[0]) * data_converted.size();
        cpuBuffer.data = data.data();
        cpuBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eTransferSrc;
        cpuBuffer.memoryPropertyFlagBits = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;

        createBuffer(cpuBuffer);

        // second: create AS Buffer
        RTXBuffer gpuBuffer;
        gpuBuffer.bufferType = RTXBufferType::GPU;
        gpuBuffer.deviceSize = sizeof(data_converted[0]) * data_converted.size();
        gpuBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
                | vk::BufferUsageFlagBits::eTransferDst
                | vk::BufferUsageFlagBits::eStorageBuffer
                | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
        gpuBuffer.memoryPropertyFlagBits = vk::MemoryPropertyFlagBits::eDeviceLocal;

        createBuffer(gpuBuffer);

        // copy from CPU to GPU
        copyFromCPUToGPU(cpuBuffer, gpuBuffer);

        return gpuBuffer;
    }

    void ASManager::createBuffer(RTXBuffer &buffer) {

        vk::BufferCreateInfo bufferCreateInfo(
                vk::BufferCreateFlags(),  // vk::BufferCreateFlags
                buffer.deviceSize, // vk::DeviceSize
                buffer.bufferUsageFlagBits,    // vk::BufferUsageFlags
                vk::SharingMode::eExclusive,    // vk::SharingMode
                {}, // uint32_t queueFamilyIndexCount
                {}  // uint32_t* queueFamilyIndices
                );
        buffer.vulkanHandle = m_core->getContext().getDevice().createBuffer(bufferCreateInfo);
        vk::MemoryRequirements memoryRequirements = m_core->getContext().getDevice().getBufferMemoryRequirements(buffer.vulkanHandle);

        vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = m_core->getContext().getPhysicalDevice().getMemoryProperties();

        uint32_t memoryTypeIndex = -1;
        for (int x = 0; x < physicalDeviceMemoryProperties.memoryTypeCount; x++) {
            if ((memoryRequirements.memoryTypeBits & (1 << x)) && (physicalDeviceMemoryProperties.memoryTypes[x].propertyFlags & buffer.memoryPropertyFlagBits) == buffer.memoryPropertyFlagBits) {
                memoryTypeIndex = x;
                break;
            }
        }

        vk::MemoryAllocateInfo memoryAllocateInfo(
                memoryRequirements.size,  // size of allocation in bytes
                memoryTypeIndex // index identifying a memory type from the memoryTypes array of the vk::PhysicalDeviceMemoryProperties structure.
                );
        vk::MemoryAllocateFlagsInfo allocateFlagsInfo(
                vk::MemoryAllocateFlagBitsKHR::eDeviceAddress  // vk::MemoryAllocateFlags
                );
        memoryAllocateInfo.setPNext(&allocateFlagsInfo);  // extend memory allocate info with allocate flag info
        buffer.deviceMemory = m_core->getContext().getDevice().allocateMemory(memoryAllocateInfo);

        uint32_t memoryOffset = 0;
        m_core->getContext().getDevice().bindBufferMemory(buffer.vulkanHandle, buffer.deviceMemory, memoryOffset);

        // only fill data in case of CPU buffer
        if (buffer.bufferType == RTXBufferType::CPU) {
            void* mapped = m_core->getContext().getDevice().mapMemory(buffer.deviceMemory, memoryOffset, buffer.deviceSize);
            std::memcpy(mapped, buffer.data, buffer.deviceSize);
            m_core->getContext().getDevice().unmapMemory(buffer.deviceMemory);
        }
    }

    void ASManager::copyFromCPUToGPU(RTXBuffer &cpuBuffer, RTXBuffer &gpuBuffer) {
        vk::CommandPool commandPool;

        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.queueFamilyIndex = m_core->getContext().getQueueManager().getGraphicsQueues()[0].familyIndex;

        vk::Result res = m_core->getContext().getDevice().createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool);
        if (res != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "ASManager: command pool could not be created! (%s)", vk::to_string(res).c_str());
        }

        vk::CommandBufferAllocateInfo bufferAllocateInfo;
        bufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        bufferAllocateInfo.commandPool = commandPool;
        bufferAllocateInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        res = m_core->getContext().getDevice().allocateCommandBuffers(&bufferAllocateInfo, &commandBuffer);
        if (res != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "ASManager: command buffer could not be allocated! (%s)", vk::to_string(res).c_str());
        }

        beginCommandBuffer(commandBuffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        vk::BufferCopy bufferCopy;
        bufferCopy.size = cpuBuffer.deviceSize;
        commandBuffer.copyBuffer(cpuBuffer.vulkanHandle, gpuBuffer.vulkanHandle, 1, &bufferCopy);
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        Queue graphicsQueue = m_core->getContext().getQueueManager().getGraphicsQueues()[0];

        graphicsQueue.handle.submit(submitInfo);
        graphicsQueue.handle.waitIdle();

        m_core->getContext().getDevice().freeCommandBuffers(commandPool, 1, &commandBuffer, m_rtxDispatcher);
        m_core->getContext().getDevice().destroyCommandPool(commandPool, nullptr, m_rtxDispatcher);
        m_core->getContext().getDevice().destroyBuffer(cpuBuffer.vulkanHandle, nullptr, m_rtxDispatcher);
        m_core->getContext().getDevice().freeMemory(cpuBuffer.deviceMemory, nullptr, m_rtxDispatcher);
    }

}