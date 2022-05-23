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

	class DescriptorWrites {
	private:
		std::vector<SampledImageDescriptorWrite> m_sampledImageWrites;
		std::vector<StorageImageDescriptorWrite> m_storageImageWrites;
		std::vector<BufferDescriptorWrite> m_uniformBufferWrites;
		std::vector<BufferDescriptorWrite> m_storageBufferWrites;
		std::vector<SamplerDescriptorWrite> m_samplerWrites;
		std::vector<AccelerationDescriptorWrite> m_accelerationWrites;
		
	public:
		DescriptorWrites& writeSampledImage(uint32_t binding,
											ImageHandle image,
											uint32_t mipLevel = 0,
											bool useGeneralLayout = false,
											uint32_t arrayIndex = 0);
		
		DescriptorWrites& writeStorageImage(uint32_t binding,
											ImageHandle image,
											uint32_t mipLevel = 0);
		
		DescriptorWrites& writeUniformBuffer(uint32_t binding,
											 BufferHandle buffer,
											 bool dynamic = false,
											 uint32_t offset = 0,
											 uint32_t size = 0);
		
		DescriptorWrites& writeStorageBuffer(uint32_t binding,
											 BufferHandle buffer,
											 bool dynamic = false,
											 uint32_t offset = 0,
											 uint32_t size = 0);
		
		DescriptorWrites& writeSampler(uint32_t binding,
									   SamplerHandle sampler);
		
		DescriptorWrites& writeAcceleration(uint32_t binding);
		
		const std::vector<SampledImageDescriptorWrite>& getSampledImageWrites() const;
		
		const std::vector<StorageImageDescriptorWrite>& getStorageImageWrites() const;
		
		const std::vector<BufferDescriptorWrite>& getUniformBufferWrites() const;
		
		const std::vector<BufferDescriptorWrite>& getStorageBufferWrites() const;
		
		const std::vector<SamplerDescriptorWrite>& getSamplerWrites() const;
		
		const std::vector<AccelerationDescriptorWrite>& getAccelerationWrites() const;
		
	};
	
}