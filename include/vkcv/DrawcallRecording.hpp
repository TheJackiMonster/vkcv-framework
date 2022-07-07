#pragma once
/**
 * @authors Sebastian Gaida, Alexander Gauggel, Artur Wasmut, Tobias Frisch
 * @file vkcv/DrawcallRecording.hpp
 * @brief Structures and functions to record drawcalls.
 */

#include <vector>
#include <vulkan/vulkan.hpp>

#include "Handles.hpp"
#include "PushConstants.hpp"

namespace vkcv {
	
	/**
	 * @brief Structure to store details about a vertex buffer binding.
	 */
    struct VertexBufferBinding {
        vk::DeviceSize offset;
        vk::Buffer buffer;
    };

	/**
	 * @brief Enum class to specify the size of indexes.
	 */
    enum class IndexBitCount {
		Bit8,
        Bit16,
        Bit32
    };
	
	/**
	 * @brief Structure to configure a descriptor set usage.
	 */
    struct DescriptorSetUsage {
        inline DescriptorSetUsage(uint32_t setLocation, DescriptorSetHandle descriptorSet,
								  const std::vector<uint32_t>& dynamicOffsets = {}) noexcept :
			setLocation(setLocation),
			descriptorSet(descriptorSet),
			dynamicOffsets(dynamicOffsets) {}

        const uint32_t          	setLocation;
        const DescriptorSetHandle 	descriptorSet;
        const std::vector<uint32_t> dynamicOffsets;
    };
	
	/**
	 * @brief Structure to store details of a mesh to draw.
	 */
    struct Mesh {
        inline Mesh() {}

        inline Mesh(std::vector<VertexBufferBinding> vertexBufferBindings,
					vk::Buffer indexBuffer,
					size_t indexCount,
					IndexBitCount indexBitCount = IndexBitCount::Bit16) noexcept :
			vertexBufferBindings(vertexBufferBindings),
			indexBuffer(indexBuffer),
            indexCount(indexCount),
            indexBitCount(indexBitCount) {}

        std::vector<VertexBufferBinding> vertexBufferBindings;
        vk::Buffer indexBuffer;
        size_t indexCount;
        IndexBitCount indexBitCount;

    };
	
	/**
	 * @brief Structure to store details for a drawcall.
	 */
    struct DrawcallInfo {
        inline DrawcallInfo(const Mesh& mesh,
							const std::vector<DescriptorSetUsage>& descriptorSets,
							const uint32_t instanceCount = 1) :
			mesh(mesh),
			descriptorSets(descriptorSets),
			instanceCount(instanceCount){}

        Mesh mesh;
        std::vector<DescriptorSetUsage> descriptorSets;
        uint32_t instanceCount;
    };
	
	/**
	 * @brief Structure to store details for a mesh shader drawcall.
	 */
    struct MeshShaderDrawcall {
        std::vector<DescriptorSetUsage> descriptorSets;
        uint32_t taskCount;
    };
	
}
