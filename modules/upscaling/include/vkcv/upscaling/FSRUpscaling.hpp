#pragma once

#include "Upscaling.hpp"

#include <vkcv/ShaderProgram.hpp>

namespace vkcv::upscaling {
	
	enum class FSRQualityMode : int {
		NONE = 0,
		ULTRA_QUALITY = 1,
		QUALITY = 2,
		BALANCED = 3,
		PERFORMANCE = 4
	};
	
	void getFSRResolution(FSRQualityMode mode,
						  uint32_t outputWidth, uint32_t outputHeight,
						  uint32_t &inputWidth, uint32_t &inputHeight);
	
	float getFSRLodBias(FSRQualityMode mode);
	
	struct FSRConstants {
		uint32_t Const0 [4];
		uint32_t Const1 [4];
		uint32_t Const2 [4];
		uint32_t Const3 [4];
		uint32_t Sample [4];
	};
	
	class FSRUpscaling : public Upscaling {
	private:
		ComputePipelineHandle m_easuPipeline;
		ComputePipelineHandle m_rcasPipeline;
		
		DescriptorSetHandle m_easuDescriptorSet;
		DescriptorSetHandle m_rcasDescriptorSet;
		
		Buffer<FSRConstants> m_easuConstants;
		Buffer<FSRConstants> m_rcasConstants;
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
		
		void recordUpscaling(const CommandStreamHandle& cmdStream,
							 const ImageHandle& input,
							 const ImageHandle& output) override;
		
		[[nodiscard]]
		bool isHdrEnabled() const;
		
		void setHdrEnabled(bool enabled);
		
		[[nodiscard]]
		float getSharpness() const;
		
		void setSharpness(float sharpness);
		
	};

}
