#pragma once

#include "Upscaling.hpp"

#include <vkcv/ShaderProgram.hpp>

namespace vkcv::upscaling {
	
	class NISUpscaling : public Upscaling {
	private:
		ComputePipelineHandle m_scalerPipeline;
		
		DescriptorSetLayoutHandle m_scalerDescriptorSetLayout;
		DescriptorSetHandle m_scalerDescriptorSet;
		
		Buffer<uint8_t> m_scalerConstants;
		SamplerHandle m_sampler;
		ImageHandle m_coefScaleImage;
		ImageHandle m_coefUsmImage;
		
		uint32_t m_blockWidth;
		uint32_t m_blockHeight;
		
		bool m_hdr;
		float m_sharpness;
		
	public:
		explicit NISUpscaling(Core &core);
		
		void recordUpscaling(const CommandStreamHandle &cmdStream,
							 const ImageHandle &input,
							 const ImageHandle &output) override;
		
		[[nodiscard]]
		bool isHdrEnabled() const;
		
		void setHdrEnabled(bool enabled);
		
		[[nodiscard]]
		float getSharpness() const;
		
		void setSharpness(float sharpness);
		
	};
	
}
