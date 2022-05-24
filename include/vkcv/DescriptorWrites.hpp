#pragma once
/**
 * @authors Artur Wasmut, Tobias Frisch, Alexander Gauggel, Vanessa Karolek
 * @file vkcv/DescriptorWrites.hpp
 * @brief Structures to handle descriptor writes.
 */

#include <vector>

#include "Handles.hpp"

namespace vkcv {
	
	struct SampledImageDescriptorWrite {
		uint32_t binding;
		ImageHandle image;
		uint32_t mipLevel;
		bool useGeneralLayout;
		uint32_t arrayIndex;
	};

	struct StorageImageDescriptorWrite {
		uint32_t binding;
		ImageHandle image;
		uint32_t mipLevel;
	};

	struct BufferDescriptorWrite {
		uint32_t binding;
		BufferHandle buffer;
		bool dynamic;
		uint32_t offset;
		uint32_t size;
	};

	struct SamplerDescriptorWrite {
		uint32_t binding;
		SamplerHandle sampler;
	};
	
	struct AccelerationDescriptorWrite {
	    uint32_t binding;
	};

	/**
	 * @brief Class to store details about writing to
	 * a descriptor set and its bindings.
	 */
	class DescriptorWrites {
	private:
		std::vector<SampledImageDescriptorWrite> m_sampledImageWrites;
		std::vector<StorageImageDescriptorWrite> m_storageImageWrites;
		std::vector<BufferDescriptorWrite> m_uniformBufferWrites;
		std::vector<BufferDescriptorWrite> m_storageBufferWrites;
		std::vector<SamplerDescriptorWrite> m_samplerWrites;
		std::vector<AccelerationDescriptorWrite> m_accelerationWrites;
		
	public:
		/**
		 * @brief Adds an entry to write an image to a given binding
		 * of a descriptor set to sample from it using specific details.
		 *
		 * @param[in] binding Binding index
		 * @param[in] image Image handle
		 * @param[in] mipLevel Mip level index
		 * @param[in] useGeneralLayout Flag to use a general layout
		 * @param[in] arrayIndex Image array index
		 * @return Instance of descriptor writes
		 */
		DescriptorWrites& writeSampledImage(uint32_t binding,
											ImageHandle image,
											uint32_t mipLevel = 0,
											bool useGeneralLayout = false,
											uint32_t arrayIndex = 0);
		
		/**
		 * @brief Adds an entry to write an image to a given binding
		 * of a descriptor set to store into it using specific details.
		 *
		 * @param[in] binding Binding index
		 * @param[in,out] image Image handle
		 * @param[in] mipLevel Mip level index
		 * @return Instance of descriptor writes
		 */
		DescriptorWrites& writeStorageImage(uint32_t binding,
											ImageHandle image,
											uint32_t mipLevel = 0);
		
		/**
		 * @brief Adds an entry to write a buffer to a given binding
		 * of a descriptor set as uniform buffer using specific details.
		 *
		 * @param[in] binding Binding index
		 * @param[in] buffer Buffer handle
		 * @param[in] dynamic Flag to use dynamic access
		 * @param[in] offset Offset for buffer access range
		 * @param[in] size Size of the buffer access range
		 * @return Instance of descriptor writes
		 */
		DescriptorWrites& writeUniformBuffer(uint32_t binding,
											 BufferHandle buffer,
											 bool dynamic = false,
											 uint32_t offset = 0,
											 uint32_t size = 0);
		
		/**
		 * @brief Adds an entry to write a buffer to a given binding
		 * of a descriptor set as storage buffer using specific details.
		 *
		 * @param[in] binding Binding index
		 * @param[in] buffer Buffer handle
		 * @param[in,out] dynamic Flag to use dynamic access
		 * @param[in] offset Offset for buffer access range
		 * @param[in] size Size of the buffer access range
		 * @return Instance of descriptor writes
		 */
		DescriptorWrites& writeStorageBuffer(uint32_t binding,
											 BufferHandle buffer,
											 bool dynamic = false,
											 uint32_t offset = 0,
											 uint32_t size = 0);
		
		/**
		 * @brief Adds an entry to write a sampler to a given binding
		 * of a descriptor set.
		 *
		 * @param[in] binding Binding index
		 * @param[in] sampler Sampler handle
		 * @return Instance of descriptor writes
		 */
		DescriptorWrites& writeSampler(uint32_t binding,
									   SamplerHandle sampler);
		
		/**
		 * @brief Adds an entry for acceleration to a given binding
		 * of a descriptor set.
		 *
		 * @param[in] binding Binding index
		 * @return Instance of descriptor writes
		 */
		DescriptorWrites& writeAcceleration(uint32_t binding);
		
		/**
		 * @brief Returns the list of stored write entries for sampled images.
		 *
		 * @return Sampled image write details
		 */
		[[nodiscard]]
		const std::vector<SampledImageDescriptorWrite>& getSampledImageWrites() const;
		
		/**
		 * @brief Returns the list of stored write entries for storage images.
		 *
		 * @return Storage image write details
		 */
		[[nodiscard]]
		const std::vector<StorageImageDescriptorWrite>& getStorageImageWrites() const;
		
		/**
		 * @brief Returns the list of stored write entries for uniform buffers.
		 *
		 * @return Uniform buffers write details
		 */
		[[nodiscard]]
		const std::vector<BufferDescriptorWrite>& getUniformBufferWrites() const;
		
		/**
		 * @brief Returns the list of stored write entries for storage buffers.
		 *
		 * @return Storage buffers write details
		 */
		[[nodiscard]]
		const std::vector<BufferDescriptorWrite>& getStorageBufferWrites() const;
		
		/**
		 * @brief Returns the list of stored write entries for samplers.
		 *
		 * @return Samplers write details
		 */
		[[nodiscard]]
		const std::vector<SamplerDescriptorWrite>& getSamplerWrites() const;
		
		/**
		 * @brief Returns the list of stored write entries for accelerations.
		 *
		 * @return Accelerations write details
		 */
		[[nodiscard]]
		const std::vector<AccelerationDescriptorWrite>& getAccelerationWrites() const;
		
	};
	
}