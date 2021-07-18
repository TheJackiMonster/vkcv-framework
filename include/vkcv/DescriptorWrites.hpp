#pragma once
#include "Handles.hpp"
#include <vector>

namespace vkcv {
	struct SampledImageDescriptorWrite {
		inline SampledImageDescriptorWrite(uint32_t binding, ImageHandle image, uint32_t mipLevel = 0, bool useGeneralLayout = false)
		    : binding(binding), image(image), mipLevel(mipLevel), useGeneralLayout(useGeneralLayout) {};
		uint32_t	binding;
		ImageHandle	image;
		uint32_t    mipLevel;
		bool        useGeneralLayout;
	};

	struct StorageImageDescriptorWrite {
		inline StorageImageDescriptorWrite(uint32_t binding, ImageHandle image, uint32_t mipLevel = 0) 
			: binding(binding), image(image), mipLevel(mipLevel) {};
		uint32_t	binding;
		ImageHandle	image;
		uint32_t	mipLevel;
	};

	struct UniformBufferDescriptorWrite {
		inline UniformBufferDescriptorWrite(uint32_t binding, BufferHandle buffer, bool dynamic = false) :
		binding(binding), buffer(buffer), dynamic(dynamic) {};
		uint32_t		binding;
		BufferHandle	buffer;
		bool 			dynamic;
	};

	struct StorageBufferDescriptorWrite {
		inline StorageBufferDescriptorWrite(uint32_t binding, BufferHandle buffer, bool dynamic = false) :
		binding(binding), buffer(buffer), dynamic(dynamic) {};
		uint32_t		binding;
		BufferHandle	buffer;
		bool			dynamic;
	};

	struct SamplerDescriptorWrite {
		inline SamplerDescriptorWrite(uint32_t binding, SamplerHandle sampler) : binding(binding), sampler(sampler) {};
		uint32_t		binding;
		SamplerHandle	sampler;
	};

	struct DescriptorWrites {
		std::vector<SampledImageDescriptorWrite>		sampledImageWrites;
		std::vector<StorageImageDescriptorWrite>		storageImageWrites;
		std::vector<UniformBufferDescriptorWrite>	    uniformBufferWrites;
		std::vector<StorageBufferDescriptorWrite>	    storageBufferWrites;
		std::vector<SamplerDescriptorWrite>			    samplerWrites;
	};
}