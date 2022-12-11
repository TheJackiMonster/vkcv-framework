#pragma once

#include "Denoiser.hpp"

namespace vkcv::denoising {
	
	class ShadowDenoiser : public Denoiser {
	private:
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
