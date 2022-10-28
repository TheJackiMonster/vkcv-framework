#pragma once

#include <vector>
#include <vkcv/camera/Camera.hpp>

#include "Effect.hpp"

namespace vkcv::effects {
	
	class BloomAndFlaresEffect : public Effect {
	private:
		bool m_advanced;
		
		ComputePipelineHandle m_downsamplePipeline;
		ComputePipelineHandle m_upsamplePipeline;
		ComputePipelineHandle m_lensFlaresPipeline;
		ComputePipelineHandle m_compositePipeline;
		
		DescriptorSetLayoutHandle m_downsampleDescriptorSetLayout;
		std::vector<DescriptorSetHandle> m_downsampleDescriptorSets;
		
		DescriptorSetLayoutHandle m_upsampleDescriptorSetLayout;
		std::vector<DescriptorSetHandle> m_upsampleDescriptorSets;
		std::vector<DescriptorSetHandle> m_flaresDescriptorSets;
		
		DescriptorSetLayoutHandle m_lensFlaresDescriptorSetLayout;
		DescriptorSetHandle m_lensFlaresDescriptorSet;
		
		DescriptorSetLayoutHandle m_compositeDescriptorSetLayout;
		DescriptorSetHandle m_compositeDescriptorSet;
		
		ImageHandle m_blurImage;
		ImageHandle m_flaresImage;
		
		SamplerHandle m_linearSampler;
		SamplerHandle m_radialLutSampler;
		
		ImageHandle m_radialLut;
		ImageHandle m_lensDirt;
		
		glm::vec3 m_cameraDirection;
		uint32_t m_upsampleLimit;
		
		void recordDownsampling(const CommandStreamHandle &cmdStream,
								const ImageHandle &input,
								const ImageHandle &sample,
								const std::vector<DescriptorSetHandle> &mipDescriptorSets);
		
		void recordUpsampling(const CommandStreamHandle &cmdStream,
							  const ImageHandle &sample,
							  const std::vector<DescriptorSetHandle> &mipDescriptorSets);
		
		void recordLensFlares(const CommandStreamHandle &cmdStream,
							  uint32_t mipLevel);
		
		void recordComposition(const CommandStreamHandle &cmdStream,
							   const ImageHandle &output);
		
	public:
		BloomAndFlaresEffect(Core& core,
							 bool advanced = false);
		
		void recordEffect(const CommandStreamHandle &cmdStream,
						  const ImageHandle &input,
						  const ImageHandle &output) override;
		
		void updateCameraDirection(const camera::Camera &camera);
		
		void setUpsamplingLimit(uint32_t limit);
		
	};
	
}
