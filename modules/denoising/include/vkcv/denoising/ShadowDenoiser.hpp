#pragma once

#include <vkcv/Handles.hpp>

#include "Denoiser.hpp"

namespace vkcv::denoising {
	
	class ShadowDenoiser : public Denoiser {
	private:
		ComputePipelineHandle m_filterPipeline;
		ComputePipelineHandle m_preparePipeline;
		ComputePipelineHandle m_tileClassificationPipeline;
		
		/**
         * The descriptor set layout of the filter pipeline.
         */
		DescriptorSetLayoutHandle m_filterDescriptorSetLayout;
		
		/**
		 * The descriptor set for the filter pipeline.
		 */
		DescriptorSetHandle m_filterDescriptorSet;
		
		/**
         * The descriptor set layout of the prepare pipeline.
         */
		DescriptorSetLayoutHandle m_prepareDescriptorSetLayout;
		
		/**
		 * The descriptor set for the prepare pipeline.
		 */
		DescriptorSetHandle m_prepareDescriptorSet;
		
		/**
         * The descriptor set layout of the tile classification
         * pipeline.
         */
		DescriptorSetLayoutHandle m_tileClassificationDescriptorSetLayout;
		
		/**
		 * The descriptor set for the tile classification
		 * pipeline.
		 */
		DescriptorSetHandle m_tileClassificationDescriptorSet;
	
	public:
		/**
         * Constructor to create a shadow denoiser instance.
         *
         * @param[in,out] core Reference to a Core instance
         */
		explicit ShadowDenoiser(Core& core);
		
		/**
         * Record the commands of the given shadow denoiser instance
         * to reduce the noise from the image of the input handle and
         * pass the important details to the output image handle.
         *
         * @param[in] cmdStream Command stream handle to record commands
         * @param[in] input Input image handle
         * @param[in] output Output image handle
         */
		void recordDenoising(const CommandStreamHandle& cmdStream,
							 const ImageHandle& input,
							 const ImageHandle& output) override;
		
	};
	
}
