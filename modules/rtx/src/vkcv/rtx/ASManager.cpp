#include "vkcv/rtx/ASManager.hpp"

namespace vkcv::rtx {

    ASManager::ASManager(vkcv::Core *core) :
    m_core(core) {

        // SUGGESTION: recursive call of buildBLAS etc.



    }

    void ASManager::buildBLAS(std::vector<uint8_t> &vertices, std::vector<uint8_t> &indices) {
        uint32_t vertexCount = vertices.size();
        uint32_t indexCount = indices.size();
        vk::Buffer vertexBuffer = makeBuffer(vertices);
        vk::Buffer indexBuffer = makeBuffer(indices);

        // INFO: It seems that we need a dynamic dispatch loader because Vulkan is an ASs ...
        vk::DispatchLoaderDynamic dld( (PFN_vkGetInstanceProcAddr) m_core->getContext().getInstance().getProcAddr("vkGetInstanceProcAddr") );
        dld.init(m_core->getContext().getInstance());

        vk::BufferDeviceAddressInfo vertexBufferDeviceAddressInfo(vertexBuffer);
        vk::DeviceAddress vertexBufferAddress = m_core->getContext().getDevice().getBufferAddressKHR(vertexBufferDeviceAddressInfo, dld);
        vk::DeviceOrHostAddressConstKHR vertexDeviceOrHostAddressConst(vertexBufferAddress);

        vk::BufferDeviceAddressInfo indexBufferDeviceAddressInfo(indexBuffer);
        vk::DeviceAddress indexBufferAddress = m_core->getContext().getDevice().getBufferAddressKHR(indexBufferDeviceAddressInfo, dld);
        vk::DeviceOrHostAddressConstKHR indexDeviceOrHostAddressConst(indexBufferAddress);

        // Specify triangle mesh data // TODO: Check if valid entries ...
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
                dld
                );

        // Allocate the AS TODO: which type do we need for the buffer?? !!!
        Buffer<vk::AccelerationStructureKHR> asBuffer = m_core->createBuffer<vk::AccelerationStructureKHR>(BufferType::RT_ACCELERATION_VERTEX, asBuildSizesInfo.accelerationStructureSize, BufferMemoryType::DEVICE_LOCAL);
        //        core->createBuffer<>()

        // Create an empty AS object
        vk::AccelerationStructureCreateInfoKHR asCreateInfo(
                {}, // creation flags
                asBuffer.getVulkanHandle(), // allocated AS buffer.
                0,
                asBuildSizesInfo.accelerationStructureSize, // size of the AS
                asBuildInfo.type // type of the AS
        );

        // Create the intended AS object
        vk::AccelerationStructureKHR blas;
        vk::Result res = m_core->getContext().getDevice().createAccelerationStructureKHR(
                &asCreateInfo, // AS create info
                nullptr, // allocator callbacks
                &blas, // the AS
                dld
        );
        if(res != vk::Result::eSuccess) {
            vkcv_log(vkcv::LogLevel::ERROR, "The Bottom Level Acceleration Structure could not be builded!");
        }
        asBuildInfo.setDstAccelerationStructure(blas);

        // TODO: destroy accelerationstructure when closing app
    }

    vk::Buffer ASManager::makeBuffer(std::vector<uint8_t> &data) {
        // convert uint8_t data into unit16_t
        std::vector<uint16_t> data_converted;
        for (size_t i=0; i<data.size(); i++) {
            data_converted.push_back((uint16_t)data[i]);
        }

        // now create a vk::Buffer of type uint16_t for the data
        vk::Device device = m_core->getContext().getDevice();

        // first: Staging Buffer creation
        vk::DeviceSize deviceSize = sizeof(data_converted[0]) * data_converted.size();
        vk::BufferUsageFlags bufferUsageFlagBits = vk::BufferUsageFlagBits::eTransferSrc;
        vk::BufferCreateInfo bufferCreateInfo(
                vk::BufferCreateFlags(),  // vk::BufferCreateFlags
                deviceSize, // vk::DeviceSize
                bufferUsageFlagBits,    // vk::BufferUsageFlags
                vk::SharingMode::eExclusive,    // vk::SharingMode
                {}, // uint32_t queueFamilyIndexCount
                {}  // uint32_t* queueFamilyIndices
                );
        vk::Buffer stagingBuffer = device.createBuffer(bufferCreateInfo, nullptr);
        vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(stagingBuffer);

        vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties = m_core->getContext().getPhysicalDevice().getMemoryProperties();

        vk::MemoryPropertyFlags memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;

        uint32_t memoryTypeIndex = -1;
        for (int x = 0; x < physicalDeviceMemoryProperties.memoryTypeCount; x++) {
            if ((memoryRequirements.memoryTypeBits & (1 << x)) && (physicalDeviceMemoryProperties.memoryTypes[x].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags) {
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
        vk::DeviceMemory deviceMemory = device.allocateMemory(memoryAllocateInfo);

        uint32_t memoryOffset = 0;
        device.bindBufferMemory(stagingBuffer, deviceMemory, memoryOffset);

        // fill staging buffer
        void* mapped = device.mapMemory(deviceMemory, memoryOffset, deviceSize);
        std::memcpy(mapped, data_converted.data(), deviceSize);
        device.unmapMemory(deviceMemory);

        // second: GPU Buffer creation
        vk::BufferUsageFlags bufferUsageFlagBits2 = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
                | vk::BufferUsageFlagBits::eTransferDst
                | vk::BufferUsageFlagBits::eStorageBuffer
                | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
        vk::BufferCreateInfo bufferCreateInfo2(
                vk::BufferCreateFlags(),  // vk::BufferCreateFlags
                deviceSize, // vk::DeviceSize
                bufferUsageFlagBits2,    // vk::BufferUsageFlags
                vk::SharingMode::eExclusive,    // vk::SharingMode
                {}, // uint32_t queueFamilyIndexCount
                {}  // uint32_t* queueFamilyIndices
                );
        vk::Buffer dataBuffer = device.createBuffer(bufferCreateInfo2, nullptr);
        vk::MemoryRequirements memoryRequirements2 = device.getBufferMemoryRequirements(dataBuffer);

        vk::PhysicalDeviceMemoryProperties physicalDeviceMemoryProperties2 = m_core->getContext().getPhysicalDevice().getMemoryProperties();

        vk::MemoryPropertyFlags memoryPropertyFlags2 = vk::MemoryPropertyFlagBits::eDeviceLocal;

        uint32_t memoryTypeIndex2 = -1;
        for (int x = 0; x < physicalDeviceMemoryProperties.memoryTypeCount; x++) {
            if ((memoryRequirements2.memoryTypeBits & (1 << x)) && (physicalDeviceMemoryProperties2.memoryTypes[x].propertyFlags & memoryPropertyFlags2) == memoryPropertyFlags2) {
                memoryTypeIndex2 = x;
                break;
            }
        }

        vk::MemoryAllocateInfo memoryAllocateInfo2(
                memoryRequirements2.size,  // size of allocation in bytes
                memoryTypeIndex2 // index identifying a memory type from the memoryTypes array of the vk::PhysicalDeviceMemoryProperties structure.
                );
        vk::MemoryAllocateFlagsInfo allocateFlagsInfo2(
                vk::MemoryAllocateFlagBitsKHR::eDeviceAddress  // vk::MemoryAllocateFlags
        );
        memoryAllocateInfo2.setPNext(&allocateFlagsInfo2);  // extend memory allocate info with allocate flag info
        vk::DeviceMemory deviceMemory2 = device.allocateMemory(memoryAllocateInfo2);

        uint32_t memoryOffset2 = 0;
        device.bindBufferMemory(dataBuffer, deviceMemory2, memoryOffset2);

        // copy data from stagingBuffer to dataBuffer
        vk::CommandPool commandPool;

        vk::CommandPoolCreateInfo commandPoolCreateInfo;
        commandPoolCreateInfo.queueFamilyIndex = m_core->getContext().getQueueManager().getGraphicsQueues()[0].familyIndex;

        if (device.createCommandPool(&commandPoolCreateInfo, nullptr, &commandPool) != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "ASManager: command pool could not be created.");
        }

        vk::CommandBufferAllocateInfo bufferAllocateInfo;
        bufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
        bufferAllocateInfo.commandPool = commandPool;
        bufferAllocateInfo.commandBufferCount = 1;

        vk::CommandBuffer commandBuffer;
        if (device.allocateCommandBuffers(&bufferAllocateInfo, &commandBuffer) != vk::Result::eSuccess) {
            vkcv_log(LogLevel::ERROR, "ASManager: command buffer could not be allocated.");
        }

        beginCommandBuffer(commandBuffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
        vk::BufferCopy bufferCopy;
        bufferCopy.size = deviceSize;
        commandBuffer.copyBuffer(stagingBuffer, dataBuffer, 1, &bufferCopy);
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        Queue graphicsQueue = m_core->getContext().getQueueManager().getGraphicsQueues()[0];

        graphicsQueue.handle.submit(submitInfo);
        graphicsQueue.handle.waitIdle();

        device.freeCommandBuffers(commandPool, 1, &commandBuffer);
        device.destroyBuffer(stagingBuffer);
        device.freeMemory(deviceMemory);

        return dataBuffer;
    }

}