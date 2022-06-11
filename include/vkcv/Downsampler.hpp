#pragma once

#include "Handles.hpp"

namespace vkcv {
	
	class Core;
	
	/**
	 * An abstract class to handle downsampling of images for mip generation.
	 */
	class Downsampler {
	protected:
		/**
         * Reference to the current Core instance.
         */
		Core& m_core;
		
	public:
		/**
         * Constructor to create a downsampler instance.
         * @param[in,out] core Reference to a Core instance
         */
		explicit Downsampler(Core& core);
		
		~Downsampler() = default;
		
		/**
		 * Record the commands of the given downsampler instance to
         * scale the image down on its own mip levels.
		 *
		 * @param[in] cmdStream
		 * @param[in] image
		 */
		virtual void recordDownsampling(const CommandStreamHandle& cmdStream,
										const ImageHandle& image) = 0;
		
	};
	
}