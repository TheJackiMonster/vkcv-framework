#pragma once
/**
 * @authors Sebastian Gaida, Tobias Frisch
 * @file vkcv/Swapchain.hpp
 * @brief Class to manage the state of a swapchain and its transitions.
 */

#include <atomic>
#include <vulkan/vulkan.hpp>

#include "Context.hpp"
#include "Surface.hpp"
#include "Window.hpp"

namespace vkcv
{
	
    class Swapchain final {
    private:
    	friend class Core;
    	friend class Window;
    	friend class SwapchainManager;
     
		const Context *m_Context;
        Surface m_Surface;
        vk::SwapchainKHR m_Swapchain;
		std::atomic<bool> m_RecreationRequired;

		/**
		 * @brief Constructor of the swapchain with the current context,
		 * a surface and a given vulkan swapchain object.
		 *
		 * @param[in,out] context Current context
		 * @param[in] surface used by the swapchain
		 * @param[in,out] swapchain to show images in the window
		 */
        Swapchain(const Context &context,
				  const Surface &surface,
                  vk::SwapchainKHR swapchain) noexcept;
	
		/**
		 * @brief Checks whether the swapchain needs to be recreated.
		 *
		 * @return True, if the swapchain should be updated,
		 * otherwise false.
		 */
		bool shouldUpdateSwapchain() const;
	
		/**
		 * @brief Updates and recreates the swapchain.
		 *
		 * @param[in,out] context that holds the device to recreate the swapchain
		 * @param[in] window that the new swapchain gets bound to
		 */
		void updateSwapchain(const Context &context, const Window &window);
	
		/**
		 * @brief Signals the swapchain to be recreated.
		 */
		void signalSwapchainRecreation();

    public:
    	Swapchain(const Swapchain& other);

        /**
         * @brief Returns the vulkan swapchain object of the swapchain.
         *
         * @return The swapchain linked with the #SwapChain class
         * @note The reference to our Swapchain variable is needed for the recreation step
         */
        [[nodiscard]]
        const vk::SwapchainKHR& getSwapchain() const;

        /**
         * @brief Returns the current surface of the swapchain.
         *
         * @return Current surface
         */
        [[nodiscard]]
        const Surface& getSurface() const;

        /**
         * @brief Returns the image format for the current surface
         * of the swapchain.
         *
         * @return Swapchain image format
         */
        [[nodiscard]]
        vk::Format getFormat() const;

        /**
         * @brief Creates a swapchain for a specific window and
         * a given context.
         *
         * @param[in,out] window Window
         * @param[in,out] context Context
         * @return New created swapchain
         */
        static Swapchain create(const Window &window, const Context &context);

        /**
         * Destructor of thw swapchain.
         */
        virtual ~Swapchain();

		/**
		 * @brief Returns the amount of images for the swapchain.
		 *
		 * @return Number of images
		*/
		uint32_t getImageCount() const;
	
        /**
         * @brief Returns the extent from the current surface of
         * the swapchain.
         *
         * @return Extent of the swapchains surface
         */
        [[nodiscard]]
		const vk::Extent2D& getExtent() const;

		/**
		 * @brief Returns the present queue index to be used with
		 * the swapchain and its current surface.
		 *
		 * @return Present queue family index
		 */
		[[nodiscard]]
		uint32_t getPresentQueueIndex() const;

	};
    
}
