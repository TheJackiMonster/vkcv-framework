#pragma once

#include <vkcv/Core.hpp>
#include <vkcv/Handles.hpp>
#include <vkcv/ShaderProgram.hpp>

namespace vkcv::upscaling {
	
	struct FSRConstants {
		uint32_t Const0 [4];
		uint32_t Const1 [4];
		uint32_t Const2 [4];
		uint32_t Const3 [4];
		uint32_t Sample [4];
	};
	
	class FSRUpscaling {
	private:
		Core& m_core;
		
		PipelineHandle m_easuPipeline;
		PipelineHandle m_rcasPipeline;
		
		DescriptorSetHandle m_easuDescriptorSet;
		DescriptorSetHandle m_rcasDescriptorSet;
		
		Buffer<FSRConstants> m_constants;
		ImageHandle m_intermediateImage;
		SamplerHandle m_sampler;
		
		bool m_hdr;
		float m_sharpness;
	
	public:
		explicit FSRUpscaling(Core& core);
		
		~FSRUpscaling() = default;
		
		void recordUpscaling(const CommandStreamHandle& cmdStream,
							 const ImageHandle& input,
							 const ImageHandle& output);
		
		[[nodiscard]]
		bool isHdrEnabled() const;
		
		void setHdrEnabled(bool enabled);
		
		[[nodiscard]]
		float getSharpness() const;
		
		void setSharpness(float sharpness);
		
	};

}
