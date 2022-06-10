#pragma once

#include <vector>

#include <vkcv/Core.hpp>
#include <vkcv/Downsampler.hpp>
#include <vkcv/ShaderProgram.hpp>

namespace vkcv::algorithm {

#define SPD_MAX_MIP_LEVELS 12
	
	struct SPDConstants {
		int mips;
		int numWorkGroupsPerSlice;
		int workGroupOffset[2];
	};
	
	struct SPDConstantsSampler {
		int mips;
		int numWorkGroupsPerSlice;
		int workGroupOffset[2];
		float invInputSize[2];
		float padding[2];
	};

	class SinglePassDownsampler : public vkcv::Downsampler {
	private:
		ComputePipelineHandle m_pipeline;
		
		DescriptorSetLayoutHandle m_descriptorSetLayout;
		std::vector<DescriptorSetHandle> m_descriptorSets;
		
		Buffer<uint32_t> m_globalCounter;
		
		SamplerHandle m_sampler;
		
	public:
		explicit SinglePassDownsampler(Core& core,
									   const SamplerHandle &sampler = SamplerHandle());
		
		void recordDownsampling(const CommandStreamHandle& cmdStream,
								const ImageHandle& image) override;
	
	};

}
