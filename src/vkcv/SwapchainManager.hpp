#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/SwapchainManager.hpp
 * @brief Class to manage the swapchains and their surfaces.
 */

#include <atomic>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "vkcv/Window.hpp"

#include "HandleManager.hpp"

namespace vkcv {
	
	const uint32_t MIN_SURFACE_SIZE = 2;
	
	/**
     * @brief Structure to handle swapchains.
     */
	struct SwapchainEntry {
		vk::SwapchainKHR m_Swapchain;
		bool m_RecreationRequired;
		
		vk::SurfaceKHR m_Surface;
		uint32_t m_PresentQueueIndex;
		vk::Extent2D m_Extent;
		vk::Format m_Format;
		vk::ColorSpaceKHR m_ColorSpace;
	};
	
	/**
	 * @brief Class to manage the creation, destruction and
	 * allocation of swapchains.
	 */
	class SwapchainManager : public HandleManager<SwapchainEntry, SwapchainHandle> {
		friend class Core;
	private:
		[[nodiscard]]
		uint64_t getIdFrom(const SwapchainHandle& handle) const override;
		
		[[nodiscard]]
		SwapchainHandle createById(uint64_t id, const HandleDestroyFunction& destroy) override;
		
		/**
		 * @brief Destroys a specific swapchain by a given id
		 *
		 * @param[in] id ID of the swapchain to be destroyed
		 */
		void destroyById(uint64_t id) override;

	public:
		SwapchainManager() noexcept;

		/**
		 * destroys every swapchain
		 */
		~SwapchainManager() noexcept override;

		/**
		 * creates a swapchain and returns the handle
		 * @param window of the to  creatable window
		 * @return the swapchainHandle of the created swapchain
		 */
		SwapchainHandle createSwapchain(Window &window);

		/**
		 * @param handle of the swapchain to get
		 * @return the reference of the swapchain
		 */
		[[nodiscard]]
		SwapchainEntry& getSwapchain(const SwapchainHandle& handle);
		
		/**
		 * @brief Checks whether the swapchain needs to be recreated.
		 *
		 * @param[in] handle Swapchain handle
		 * @return True, if the swapchain should be updated,
		 * otherwise false.
		 */
		bool shouldUpdateSwapchain(const SwapchainHandle& handle) const;
		
		/**
		 * @brief Updates and recreates the swapchain.
		 *
		 * @param[in] handle Swapchain handle
		 * @param[in] window that the new swapchain gets bound to
		 */
		void updateSwapchain(const SwapchainHandle& handle, const Window &window);
		
		/**
		 * @brief Signals the swapchain to be recreated.
		 *
		 * @param[in] handle Swapchain handle
		 */
		void signalRecreation(const SwapchainHandle& handle);
		
		/**
         * @brief Returns the image format for the current surface
         * of the swapchain.
         *
         * @param[in] handle Swapchain handle
         * @return Swapchain image format
         */
		[[nodiscard]]
		vk::Format getFormat(const SwapchainHandle& handle) const;
		
		/**
		 * @brief Returns the amount of images for the swapchain.
		 *
		 * @param[in] handle Swapchain handle
		 * @return Number of images
		*/
		uint32_t getImageCount(const SwapchainHandle& handle) const;
		
		/**
         * @brief Returns the extent from the current surface of
         * the swapchain.
         *
         * @param[in] handle Swapchain handle
         * @return Extent of the swapchains surface
         */
		[[nodiscard]]
		const vk::Extent2D& getExtent(const SwapchainHandle& handle) const;
		
		/**
		 * @brief Returns the present queue index to be used with
		 * the swapchain and its current surface.
		 *
		 * @param[in] handle Swapchain handle
		 * @return Present queue family index
		 */
		[[nodiscard]]
		uint32_t getPresentQueueIndex(const SwapchainHandle& handle) const;
		
		/**
		 * @brief Returns the color space of the surface from
		 * a swapchain.
		 *
		 * @param[in] handle Swapchain handle
		 * @return Color space
		 */
		[[nodiscard]]
		vk::ColorSpaceKHR getSurfaceColorSpace(const SwapchainHandle& handle) const;

		/**
		 * gets the swapchain images
		 * @param handle of the swapchain
		 * @return a vector of the swapchain images
		 */
		[[nodiscard]]
		std::vector<vk::Image> getSwapchainImages(const SwapchainHandle& handle) const;

		/**
		 * creates the swapchain imageViews for the swapchain
		 * @param handle of the swapchain which ImageViews should be created
		 * @return a ov ImageViews of the swapchain
		 */
		[[nodiscard]]
		std::vector<vk::ImageView> createSwapchainImageViews(SwapchainHandle& handle);
		
	};
	
}