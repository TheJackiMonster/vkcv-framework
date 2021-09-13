#pragma once

#include <vkcv/Core.hpp>

namespace vkcv::rtx {

    /**
     * @brief Used for @#RTXBuffer creation depending on the @#RTXBufferType.
     */
    enum class RTXBufferType {
        CPU,
        GPU,
        ACCELERATION,
        SHADER_BINDING
    };

    /**
     * @brief Used as a container to handle buffer creation and destruction in RTX-specific use cases.
     */
    struct RTXBuffer {
        RTXBufferType bufferType;
        void* data;
        vk::DeviceSize deviceSize;
        vk::DeviceMemory deviceMemory;
        vk::BufferUsageFlags bufferUsageFlagBits;
        vk::MemoryPropertyFlags memoryPropertyFlagBits;
        vk::Buffer vulkanHandle;
    };

    /**
     * @brief Used as a container to handle bottom-level acceleration structure (BLAS) construction and destruction.
     */
    struct BottomLevelAccelerationStructure {
        RTXBuffer vertexBuffer;
        RTXBuffer indexBuffer;
        RTXBuffer accelerationBuffer;
        vk::AccelerationStructureKHR vulkanHandle;
    };

    /**
     * @brief Used as a container to handle top-level acceleration structure (TLAS) construction and destruction.
     */
    struct TopLevelAccelerationStructure {
        RTXBuffer gpuBufferInstances;
        RTXBuffer tlasBuffer;
        RTXBuffer tempBuildDataBuffer;  // scratch buffer
        vk::AccelerationStructureKHR vulkanHandle;
    };

    /**
     * @brief A class for managing acceleration structures (bottom, top).
     */
    class ASManager {
    private:
        Core* m_core;
        std::vector<BottomLevelAccelerationStructure> m_bottomLevelAccelerationStructures;
        TopLevelAccelerationStructure m_topLevelAccelerationStructure;
        vk::DispatchLoaderDynamic m_rtxDispatcher;


        /**
         * @brief Returns a #RTXBuffer object holding data of type uint16_t from given @p data of type uint8_t.
         * @param data The input data of type uint8_t.
         * @return A @#RTXBuffer object holding @p data.
         */
        RTXBuffer makeBufferFromData(std::vector<uint8_t> &data);

        /**
         * @brief Copies @p cpuBuffer data into a @p gpuBuffer. Typical use case is a staging buffer (namely,
         * @p cpuBuffer) used to fill a @p gpuBuffer with @p vk::MemoryPropertyFlagBits::eDeviceLocal flag set.
         * @p cpuBuffer is destroyed and freed after copying.
         * @param cpuBuffer
         * @param gpuBuffer
         */
        void copyFromCPUToGPU(RTXBuffer &cpuBuffer, RTXBuffer &gpuBuffer);

    public:

        /**
         * @brief Constructor of @#ASManager .
         * @param core
         */
        ASManager(vkcv::Core *core);

        /**
         * @brief Default destructor of @#ASManager.
         */
        ~ASManager();

        /**
        * @brief A helper function used by #ASManager::makeBufferFromData. Creates a fully initialized #RTXBuffer object
        * from partially specified @p buffer. All missing data of @p buffer will be completed by this function.
        * @param buffer The partially specified #RTXBuffer holding that part of information which is required for
        * successfully creating a vk::Buffer object.
        */
        void createBuffer(RTXBuffer& buffer);

        /**
         * @brief Build a Bottom Level Acceleration Structure (BLAS) object from given @p vertices and @p indices.
         * @param[in] vertices The vertex data of type uint8_t.
         * @param[in] indices The index data of type uint8_t.
         */
        void buildBLAS(std::vector<uint8_t> &vertices, std::vector<uint8_t> &indices);

        /**
         * @brief Build a Top Level Acceleration Structure (TLAS) object from the created
         * @#ASManager::m_accelerationStructures objects.
         */
        void buildTLAS();

        /**
        * @brief Returns the top-level acceleration structure buffer.
        * @return A @#TopLevelAccelerationStructure object holding the tlas.
        */
        TopLevelAccelerationStructure getTLAS();

        /**
         * @brief Returns the bottom-level acceleration structure at @p id.
         * @param id The ID used for indexing.
         * @return The specified @#BottomLevelAccelerationStructure object.
         */
        BottomLevelAccelerationStructure getBLAS(uint32_t id);

        /**
         * @brief Returns the dispatcher member variable for access in the @#RTXModule.
         * @return The dispatcher member variable.
         */
        const vk::DispatchLoaderDynamic& getDispatcher();
    };
}