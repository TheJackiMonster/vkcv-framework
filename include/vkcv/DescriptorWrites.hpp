#pragma once
#include "Handles.hpp"
#include <vector>

namespace vkcv {
	struct SampledImageDescriptorWrite {
		inline SampledImageDescriptorWrite(uint32_t binding, ImageHandle image) : binding(binding), image(image) {};
		uint32_t	binding;
		ImageHandle	image;
	};

	struct StorageImageDescriptorWrite {
		inline StorageImageDescriptorWrite(uint32_t binding, ImageHandle image) : binding(binding), image(image) {};
		uint32_t	binding;
		ImageHandle	image;
	};

	struct UniformBufferDescriptorWrite {
		inline UniformBufferDescriptorWrite(uint32_t binding, BufferHandle buffer) : binding(binding), buffer(buffer) {};
		uint32_t		binding;
		BufferHandle	buffer;
	};

	struct StorageBufferDescriptorWrite {
		inline StorageBufferDescriptorWrite(uint32_t binding, BufferHandle buffer) : binding(binding), buffer(buffer) {};
		uint32_t		binding;
		BufferHandle	buffer;
	};

	struct SamplerDescriptorWrite {
		inline SamplerDescriptorWrite(uint32_t binding, SamplerHandle sampler) : binding(binding), sampler(sampler) {};
		uint32_t		binding;
		SamplerHandle	sampler;
	};

	struct DescriptorWrites {
		std::vector<SampledImageDescriptorWrite>		sampledImageWrites;
		std::vector<StorageImageDescriptorWrite>		storageImageWrites;
		std::vector<UniformBufferDescriptorWrite>	uniformBufferWrites;
		std::vector<StorageBufferDescriptorWrite>	storageBufferWrites;
		std::vector<SamplerDescriptorWrite>			samplerWrites;
	};
}