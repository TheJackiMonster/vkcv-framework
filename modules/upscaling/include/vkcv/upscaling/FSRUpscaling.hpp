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
		
		/**
		 * Sharpness will calculate the rcasAttenuation value
		 * which should be between 0.0f and 2.0f (default: 0.25f).
		 *
		 * rcasAttenuation = (1.0f - sharpness) * 2.0f
		 *
		 * So the default value for sharpness should be 0.875f.
		 *
		 * Beware that 0.0f or any negative value of sharpness will
		 * disable the rcas pass completely.
		 */
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
