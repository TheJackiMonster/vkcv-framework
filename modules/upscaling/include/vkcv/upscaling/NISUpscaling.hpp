#pragma once

#include "Upscaling.hpp"

#include <vkcv/Buffer.hpp>
#include <vkcv/ShaderProgram.hpp>

namespace vkcv::upscaling {
	
	/**
     * @addtogroup vkcv_upscaling
     * @{
     */
	
	/**
	 * A class to handle upscaling via NVIDIA Image Scaling.
	 * https://github.com/NVIDIAGameWorks/NVIDIAImageScaling
	 */
	class NISUpscaling : public Upscaling {
	private:
		/**
         * The compute pipeline of the NIS upscaling.
         */
		ComputePipelineHandle m_scalerPipeline;
		
		/**
		 * The descriptor set layout of the upscaling pipeline.
		 */
		DescriptorSetLayoutHandle m_scalerDescriptorSetLayout;
		
		/**
		 * The descriptor set for the upscaling pipeline.
		 */
		DescriptorSetHandle m_scalerDescriptorSet;
		
		/**
		 * The buffer template to handle NIS constants for
         * the upscaling pipeline.
		 */
		Buffer<uint8_t> m_scalerConstants;
		
		/**
		 * The sampler handle to use for accessing the images
         * in the NIS upscaling process.
		 */
		SamplerHandle m_sampler;
		
		/**
		 * The image handle to store the upscaling coefficients.
		 */
		ImageHandle m_coefScaleImage;
		
		/**
		 * The image handle to store the USM coefficients.
		 */
		ImageHandle m_coefUsmImage;
		
		/**
		 * The amount of pixels per block width.
		 */
		uint32_t m_blockWidth;
		
		/**
		 *  The amount of pixels per block height.
		 */
		uint32_t m_blockHeight;
		
		/**
		 * Current state of HDR support.
		 */
		bool m_hdr;
		
		/**
		 * The current value of sharpness.
		 */
		float m_sharpness;
		
	public:
		/**
		 * Constructor to create instance for NIS upscaling.
		 *
		 * @param[in,out] core Reference to a Core instance
		 */
		explicit NISUpscaling(Core &core);
		
		/**
         * Record the commands of the NIS upscaling instance to
         * scale the image of the input handle to the resolution of
         * the output image handle via NVIDIA Image Scaling.
         *
         * @param[in] cmdStream Command stream handle to record commands
         * @param[in] input Input image handle
         * @param[in] output Output image handle
         */
		void recordUpscaling(const CommandStreamHandle &cmdStream,
							 const ImageHandle &input,
							 const ImageHandle &output) override;
		
		/**
         * Checks if HDR support is enabled and returns the status as boolean.
         *
         * @return true if HDR is supported, otherwise false
         */
		[[nodiscard]]
		bool isHdrEnabled() const;
		
		/**
         * Changes the status of HDR support of the NIS upscaling instance.
         *
         * @param[in] enabled New status of HDR support
         */
		void setHdrEnabled(bool enabled);
		
		/**
         * Returns the amount of sharpness the NIS upscaling instance is using.
         *
         * @return The amount of sharpness
         */
		[[nodiscard]]
		float getSharpness() const;
		
		/**
         * Changes the amount of sharpness of the NIS upscaling instance.
         * The new sharpness value is restricted by 0.0f as lower and 1.0f
         * as upper boundary.
         *
         * @param[in] sharpness New sharpness value
         */
		void setSharpness(float sharpness);
		
	};
	
	/** @} */
	
}
