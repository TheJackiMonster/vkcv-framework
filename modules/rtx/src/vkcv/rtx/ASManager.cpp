#include "vkcv/rtx/ASManager.hpp"

namespace vkcv::rtx {

    ASManager::ASManager(vkcv::Core *core) :
    m_core(core) {

        // SUGGESTION: recursive call of buildBLAS etc.



    }

    void ASManager::buildBLAS(Buffer<uint16_t> &vertexBuffer, Buffer<uint16_t> &indexBuffer) {
        // INFO: It seems that we need a dynamic dispatch loader because Vulkan is an ASs ...
        vk::DispatchLoaderDynamic dld( (PFN_vkGetInstanceProcAddr) m_core->getContext().getInstance().getProcAddr("vkGetInstanceProcAddr") );
        dld.init(m_core->getContext().getInstance());

        vk::BufferDeviceAddressInfo vertexBufferDeviceAddressInfo(vertexBuffer.getVulkanHandle());
        vk::DeviceAddress vertexBufferAddress = m_core->getContext().getDevice().getBufferAddressKHR(vertexBufferDeviceAddressInfo, dld);
        vk::DeviceOrHostAddressConstKHR vertexDeviceOrHostAddressConst(vertexBufferAddress);

        vk::BufferDeviceAddressInfo indexBufferDeviceAddressInfo(indexBuffer.getVulkanHandle());
        vk::DeviceAddress indexBufferAddress = m_core->getContext().getDevice().getBufferAddressKHR(indexBufferDeviceAddressInfo, dld);
        vk::DeviceOrHostAddressConstKHR indexDeviceOrHostAddressConst(indexBufferAddress);

        // Specify triangle mesh data // TODO: Check if valid entries ...
        vk::AccelerationStructureGeometryTrianglesDataKHR asTriangles(
                vk::Format::eR32G32B32Sfloat,   // vertex format
                vertexDeviceOrHostAddressConst, // vertex buffer address (vk::DeviceOrHostAddressConstKHR)
                3 * sizeof(float), // vertex stride (vk::DeviceSize)
                uint32_t(vertexBuffer.getCount() - 1), // maxVertex (uint32_t)
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
                uint32_t(indexBuffer.getCount() / 3), // the primitiveCount (uint32_t)
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

        // Allocate the AS TODO: which type do we need for the buffer??
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

}