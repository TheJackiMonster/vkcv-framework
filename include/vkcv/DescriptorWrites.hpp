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

	struct BufferDescriptorWrite {
		inline BufferDescriptorWrite(uint32_t binding, BufferHandle buffer, bool dynamic = false,
									 uint32_t offset = 0, uint32_t size = 0) :
		binding(binding), buffer(buffer), dynamic(dynamic), offset(offset), size(size) {};
		uint32_t		binding;
		BufferHandle	buffer;
		bool 			dynamic;
		uint32_t 		offset;
		uint32_t 		size;
	};

	struct SamplerDescriptorWrite {
		inline SamplerDescriptorWrite(uint32_t binding, SamplerHandle sampler) : binding(binding), sampler(sampler) {};
		uint32_t		binding;
		SamplerHandle	sampler;
	};

	struct DescriptorWrites {
		std::vector<SampledImageDescriptorWrite>		sampledImageWrites;
		std::vector<StorageImageDescriptorWrite>		storageImageWrites;
		std::vector<BufferDescriptorWrite>	    		uniformBufferWrites;
		std::vector<BufferDescriptorWrite>	    		storageBufferWrites;
		std::vector<SamplerDescriptorWrite>			    samplerWrites;
	};
}