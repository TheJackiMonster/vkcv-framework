#include "vkcv/rtx/RTX.hpp"
#include "vkcv/rtx/ASManager.hpp"

namespace vkcv::rtx {

    RTXModule::RTXModule() {

        // prepare needed raytracing extensions
        m_instanceExtensions = {
                "VK_KHR_get_physical_device_properties2"
        };
        m_deviceExtensions = {
                "VK_KHR_maintenance3",
                "VK_EXT_descriptor_indexing",
                "VK_KHR_buffer_device_address",
                "VK_KHR_deferred_host_operations",
                "VK_KHR_acceleration_structure",
                "VK_KHR_spirv_1_4",
                "VK_KHR_ray_tracing_pipeline",
                "VK_KHR_ray_query",
                "VK_KHR_pipeline_library"
        };
    }

    void RTXModule::init(Core* core, Buffer<uint8_t> &vertexBuffer, Buffer<uint8_t> &indexBuffer) {

        // build acceleration structures BLAS then TLAS --> see ASManager
//        ASManager asManager(core);

    }

    std::vector<const char*> RTXModule::getInstanceExtensions() {
        return m_instanceExtensions;
    }

    std::vector<const char*> RTXModule::getDeviceExtensions() {
        return m_deviceExtensions;
    }

}