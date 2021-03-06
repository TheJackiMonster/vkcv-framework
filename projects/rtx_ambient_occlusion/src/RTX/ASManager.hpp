#pragma once

#include <vkcv/Core.hpp>

namespace vkcv::rtx {

    /**
     * @brief Used for @#RTXBuffer creation depending on the @#RTXBufferType.
     */
    enum class RTXBufferType {
        STAGING,
        GPU,
        ACCELERATION,
        SHADER_BINDING,
        SCRATCH
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
        const vk::Device* m_device;
        std::vector<BottomLevelAccelerationStructure> m_bottomLevelAccelerationStructures;
        TopLevelAccelerationStructure m_topLevelAccelerationStructure;
        vk::DispatchLoaderDynamic m_rtxDispatcher;
        
        /**
         * Creates a command pool.
         */
        vk::CommandPool createCommandPool();

        /**
         * @brief Takes a @p cmdPool, allocates a command buffer and starts recording it.
         * @param cmdPool The command pool.
         * @return The allocated command buffer.
         */
        vk::CommandBuffer createAndBeginCommandBuffer( vk::CommandPool cmdPool);

        /**
         * @brief Ends the @p commandBuffer,submits it and waits. Afterwards frees the @p commandBuffer.
         * @param commandPool The command pool.
         * @param commandBuffer The command buffer.
         */
        void submitCommandBuffer(vk::CommandPool commandPool, vk::CommandBuffer& commandBuffer);

        /**
         * @brief Gets the device address of a @p buffer.
         * @param buffer The buffer.
         * @return The device address of the @p buffer.
         */
        vk::DeviceAddress getBufferDeviceAddress(vk::Buffer buffer);

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
         * @brief Returns a @#RTXBuffer object holding data of type @p T.
         * @param data The input data of type @p T.
         * @return A @#RTXBuffer object holding @p data of type @p T.
         */
        template<class T>
        RTXBuffer makeBufferFromData(std::vector<T>& data) {

            // first: Staging Buffer creation
            RTXBuffer stagingBuffer;
            stagingBuffer.bufferType = RTXBufferType::STAGING;
            stagingBuffer.deviceSize = sizeof(T) * data.size();
            stagingBuffer.data = data.data();
            stagingBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eTransferSrc;
            stagingBuffer.memoryPropertyFlagBits = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;

            createBuffer(stagingBuffer);

            // second: create AS Buffer
            RTXBuffer targetBuffer;
            targetBuffer.bufferType = RTXBufferType::GPU;
            targetBuffer.deviceSize = sizeof(T) * data.size();
            targetBuffer.bufferUsageFlagBits = vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
                | vk::BufferUsageFlagBits::eTransferDst
                | vk::BufferUsageFlagBits::eStorageBuffer
                | vk::BufferUsageFlagBits::eShaderDeviceAddress;
            targetBuffer.memoryPropertyFlagBits = vk::MemoryPropertyFlagBits::eDeviceLocal;

            createBuffer(targetBuffer);

            // copy from CPU to GPU
            copyFromCPUToGPU(stagingBuffer, targetBuffer);

            return targetBuffer;
        }

        /**
        * @brief A helper function used by @#ASManager::makeBufferFromData. Creates a fully initialized @#RTXBuffer object
        * from partially specified @p buffer. All missing data of @p buffer will be completed by this function.
        * @param buffer The partially specified @#RTXBuffer holding that part of information which is required for
        * successfully creating a @p vk::Buffer object.
        */
        void createBuffer(RTXBuffer& buffer);

        /**
         * @brief Build a Bottom Level Acceleration Structure (BLAS) object from given @p vertexBuffer and @p indexBuffer.
         * @param[in] vertexBuffer The vertex data.
         * @param[in] vertexCount The amount of vertices in @p vertexBuffer.
         * @param[in] indexBuffer The index data.
         * @param[in] indexCount The amount of indices in @p indexBuffer.
         */
        void buildBLAS(RTXBuffer vertexBuffer, uint32_t vertexCount, RTXBuffer indexBuffer, uint32_t indexCount);

        /**
         * @brief Build a Top Level Acceleration Structure (TLAS) object from the created
         * @#ASManager::m_accelerationStructures objects.
         */
        void buildTLAS();

        /**
        * @brief Returns the top-level acceleration structure (TLAS) buffer.
        * @return A @#TopLevelAccelerationStructure object holding the TLAS.
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