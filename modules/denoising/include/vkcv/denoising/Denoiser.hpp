#pragma once

#include <vkcv/Core.hpp>

namespace vkcv::denoising {
	
	class Denoiser {
	protected:
		/**
         * Reference to the current Core instance.
         */
		Core& m_core;
		
	public:
		/**
         * Constructor to create a denoiser instance.
         *
         * @param[in,out] core Reference to a Core instance
         */
		explicit Denoiser(Core& core);
	
		~Denoiser() = default;
		
		/**
         * Record the commands of the given denoiser instance to
         * reduce the noise from the image of the input handle and
         * pass the important details to the output image handle.
         *
         * @param[in] cmdStream Command stream handle to record commands
         * @param[in] input Input image handle
         * @param[in] output Output image handle
         */
		virtual void recordDenoising(const CommandStreamHandle& cmdStream,
									 const ImageHandle& input,
									 const ImageHandle& output) = 0;
		
	};
	
}
