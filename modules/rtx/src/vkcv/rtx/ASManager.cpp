#include "vkcv/rtx/ASManager.hpp"

namespace vkcv::rtx {

    ASManager::ASManager(vkcv::Core *core) {
        // RaytracingBuilder : build BLAS
        // BLAS is a vector of BLASEntries which take entries of BLASInput vector

        // we need to prepare build information for acceleration build command
//        std::vector<vk::AccelerationStructureBuildGeometryInfoKHR> buildInfos(0 /* size of BLAS vector as uint32_t */);


        // list of BLAS -> for each BLAS take AccelKHR -> createAcceleration(VkAccelerationStructureCreateInfoKHR)
//        vk::AccelerationStructureCreateInfoKHR asCreateInfo = {
//                {},
//                /* hier muss ein buffer hin */
//        };
//        asCreateInfo.

//        m_buffer = core->createBuffer<void*>(vkcv::BufferType::RT_ACCELERATION, )   // we need the size for the buffer :c
    }

}