#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Surface.hpp
 * @brief Class to manage the surface used by a swapchain.
 */

#include <vulkan/vulkan.hpp>

#include "Context.hpp"
#include "Window.hpp"

namespace vkcv {
	
	const uint32_t MIN_SURFACE_SIZE = 2;

	class Surface {
	private:
		friend class Swapchain;
		friend class SwapchainManager;
		
		const Context *m_Context;
		vk::SurfaceKHR m_Handle;
		uint32_t m_PresentQueueIndex;
		vk::Extent2D m_Extent;
		vk::Format m_Format;
		vk::ColorSpaceKHR m_ColorSpace;
		
		Surface(const Context &context,
				const vk::SurfaceKHR &handle,
				uint32_t presentQueueIndex,
				const vk::Extent2D &extent,
				vk::Format format,
				vk::ColorSpaceKHR colorSpace);
		
		vk::SwapchainKHR createVulkanSwapchain(const Window &window,
											   const vk::SwapchainKHR &oldSwapchain);
		
	public:
		Surface(const Surface& other) = default;
		
		Surface(Surface&& other) noexcept;
		
		Surface& operator=(const Surface& other) = default;
		
		Surface& operator=(Surface&& other) noexcept;
		
		~Surface();
		
		/**
		 * @brief Creates a surface via a window and a current context.
		 *
		 * @param[in] window Window
		 * @param[in] context Context
		 * @return Created surface
		 */
		static Surface create(const Window &window,
							  const Context &context);
		
		/**
         * @brief Returns the Vulkan-Surface of the object.
         *
         * @return Current Vulkan-Surface
         */
		[[nodiscard]]
		vk::SurfaceKHR getSurface() const;
		
		/**
		 * @brief Returns the queue index of the present queue
		 * for the surface.
		 *
		 * @return Present queue index
		 */
		[[nodiscard]]
		uint32_t getPresentQueueIndex() const;
		
		/**
		 * @brief Returns the extent of the surfaces resolution.
		 *
		 * @return Extent of surface
		 */
		[[nodiscard]]
		const vk::Extent2D& getExtent() const;
		
		/**
		 * @brief Returns the image format of the surface to
		 * present an image.
		 *
		 * @return Vulkan image format
		 */
		[[nodiscard]]
		vk::Format getFormat() const;
		
		/**
		 * @brief Returns the color space of the surface.
		 *
		 * @return Color space
		 */
		[[nodiscard]]
		vk::ColorSpaceKHR getColorSpace() const;
		
	};

}
