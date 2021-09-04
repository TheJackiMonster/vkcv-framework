#include "vkcv/rtx/ASManager.hpp"

namespace vkcv::rtx {

    ASManager::ASManager(vkcv::Core *core) :
    m_core(core) {
        // INFO: It seems that we need a dynamic dispatch loader because Vulkan is an ASs ...
        m_rtxDispatcher = vk::DispatchLoaderDynamic( (PFN_vkGetInstanceProcAddr) m_core->getContext().getInstance().getProcAddr("vkGetInstanceProcAddr") );
        m_rtxDispatcher.init(m_core->getContext().getInstance());

        // SUGGESTION: recursive call of buildBLAS etc.

    }

    ASManager::~ASManager() noexcept {
        // destroy every acceleration structure, its data containers and free used memory blocks
        for (size_t i=0; i < m_accelerationStructures.size(); i++) {
            AccelerationStructure as = m_accelerationStructures[i];
            m_core->getContext().getDevice().destroy(as.vulkanHandle, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().destroy(as.accelerationBuffer.vulkanHandle, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().destroy(as.indexBuffer.vulkanHandle, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().destroy(as.vertexBuffer.vulkanHandle, nullptr, m_rtxDispatcher);

            m_core->getContext().getDevice().freeMemory(as.accelerationBuffer.deviceMemory, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().freeMemory(as.indexBuffer.deviceMemory, nullptr, m_rtxDispatcher);
            m_core->getContext().getDevice().freeMemory(as.vertexBuffer.deviceMemory, nullptr, m_rtxDispatcher);
        }
    }

    void ASManager::buildTLAS() {
        // TODO
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
        RTXBuffer accelerationBuffer;
        accelerationBuffer.bufferType = RTXBufferType::ACCELERATION;
        accelerationBuffer.deviceSize = asBuildSizesInfo.accelerationStructureSize;
        accelerationBuffer.data = nullptr;
        accelerationBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR
                | vk::BufferUsageFlagBits::eShaderDeviceAddress
                | vk::BufferUsageFlagBits::eStorageBuffer;
        accelerationBuffer.memoryPropertyFlagBits = {};

        createBuffer(accelerationBuffer);   // TODO: is this on GPU?

        // Create an empty AS object
        vk::AccelerationStructureCreateInfoKHR asCreateInfo(
                {}, // creation flags
                accelerationBuffer.vulkanHandle, // allocated AS buffer.
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
            vkcv_log(vkcv::LogLevel::ERROR, "The Bottom Level Acceleration Structure could not be builded!");
        }
        asBuildInfo.setDstAccelerationStructure(blasKHR);

        AccelerationStructure blas = {
                vertexBuffer,
                indexBuffer,
                accelerationBuffer,
                blasKHR
        };
        m_accelerationStructures.push_back(blas);
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
        gpuBuffer.data = data.data();
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

        // only fill data in case of CPU buffer TODO: what about acceleration structure buffer?
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

        if (m_core->getContext().getDevice().createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "ASManager: command pool could not be created.");
        }

        vk::CommandBufferAllocateInfo bufferAllocateInfo;
        bufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        bufferAllocateInfo.commandPool = commandPool;
        bufferAllocateInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        if (m_core->getContext().getDevice().allocateCommandBuffers(&bufferAllocateInfo, &commandBuffer) != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "ASManager: command buffer could not be allocated.");
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