#pragma once

#include "Downsampler.hpp"

namespace vkcv {
	
	class ImageManager;
	
	/**
	 * @brief A class to handle downsampling via blit from a graphics queue.
	 */
	class BlitDownsampler : public Downsampler {
		friend class Core;
	private:
		/**
         * Reference to the image manager.
         */
		ImageManager& m_imageManager;
		
		/**
		 * @brief Constructor to create a blit downsampler instance.
		 *
		 * @param[in,out] core Reference to a Core instance
		 * @param[in,out] imageManager Reference to an image manager
		 */
		BlitDownsampler(Core& core,
						ImageManager& imageManager);
	
	public:
		/**
		 * @brief Record the commands of the downsampling instance to
		 * generate all mip levels of an input image via a
		 * command stream.
		 *
		 * @param[in] cmdStream Command stream handle
		 * @param[in] image Image handle
		 */
		void recordDownsampling(const CommandStreamHandle& cmdStream,
								const ImageHandle& image) override;
	};
	
}
