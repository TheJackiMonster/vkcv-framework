#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/ImageConfig.hpp
 * @brief Structure for image configuration.
 */

#include "Multisampling.hpp"

namespace vkcv {
	
	/**
	 * @brief Structure to configure image before its creation
	 */
	struct ImageConfig {
	private:
		uint32_t m_width;
		uint32_t m_height;
		uint32_t m_depth;
		
		bool m_supportStorage;
		bool m_supportColorAttachment;
		bool m_cubeMapImage;
		
		Multisampling m_msaa;
		
	public:
		/**
		 * Constructor of the image configuration by
		 * a given resolution.
		 *
		 * @param[in] width Image width
		 * @param[in] height Image height
		 * @param[in] depth Image depth
		 */
		ImageConfig(uint32_t width,
					uint32_t height,
					uint32_t depth = 1);
	
		ImageConfig(const ImageConfig &other) = default;
		ImageConfig(ImageConfig&& other) = default;

		~ImageConfig() = default;

		ImageConfig& operator=(const ImageConfig &other) = default;
		ImageConfig& operator=(ImageConfig&& other) = default;
		
		/**
		 * Return the configured width of the image.
		 *
		 * @return Image width
		 */
		[[nodiscard]]
		uint32_t getWidth() const;
		
		/**
		 * Set configured width of the image.
		 *
		 * @param[in] width Image width
		 */
		void setWidth(uint32_t width);
		
		/**
		 * Return the configured height of the image.
		 *
		 * @return Image height
		 */
		[[nodiscard]]
		uint32_t getHeight() const;
		
		/**
		 * Set configured height of the image.
		 *
		 * @param[in] height Image height
		 */
		void setHeight(uint32_t height);
		
		/**
		 * Return the configured depth of the image.
		 *
		 * @return Image depth
		 */
		[[nodiscard]]
		uint32_t getDepth() const;
		
		/**
		 * Set configured depth of the image.
		 *
		 * @param[in] depth Image depth
		 */
		void setDepth(uint32_t depth);
		
		/**
		 * Return whether the image is configured to
		 * support storage operations.
		 *
		 * @return True, if it supports storage, otherwise false
		 */
		[[nodiscard]]
		bool isSupportingStorage() const;
		
		/**
		 * Set whether the image is configured to
		 * support storage operations.
		 *
		 * @param[in] supportStorage Support storage
		 */
		void setSupportingStorage(bool supportStorage);
		
		/**
		 * Return whether the image is configured to
		 * support being used as color attachment.
		 *
		 * @return True, if it supports color attachment, otherwise false
		 */
		[[nodiscard]]
		bool isSupportingColorAttachment() const;
		
		/**
		 * Set whether the image is configured to
		 * support being used as color attachment.
		 *
		 * @param[in] supportColorAttachment Support color attachment
		 */
		void setSupportingColorAttachment(bool supportColorAttachment);
		
		/**
		 * Return whether the image is configured to
		 * be a cube map.
		 *
		 * @return True, if the image is a cube map, otherwise false
		 */
		[[nodiscard]]
		bool isCubeMapImage() const;
		
		/**
		 * Set whether the image is configured to
		 * be a cube map.
		 *
		 * @param[in] cubeMapImage Is cube map image
		 */
		void setCubeMapImage(bool cubeMapImage);
		
		/**
		 * Return type of multisampling the image
		 * is configured to use.
		 *
		 * @return Multisampling
		 */
		[[nodiscard]]
		Multisampling getMultisampling() const;
		
		/**
		 * Set the multisampling of the image
		 * configuration.
		 *
		 * @param[in] msaa Multisampling
		 */
		void setMultisampling(Multisampling msaa);
		
	};
	
}
