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
		 * Constructor of a SwapChain object
		 * glfw is not initialized in this class because ist must be sure that there exists a context first
		 * glfw is already initialized by the window class
		 *
		 * @param context Current context
		 * @param surface used by the swapchain
		 * @param swapchain to show images in the window
		 */
        Swapchain(const Context &context,
				  const Surface &surface,
                  vk::SwapchainKHR swapchain) noexcept;
	
		/**
		 * checks if the update flag is true
		 * @return if an update is needed
		 */
		bool shouldUpdateSwapchain() const;
	
		/**
		 * recreates the swapchain
		 * @param context that holds the device to recreate the swapchain
		 * @param window that the new swapchain gets bound to
		 */
		void updateSwapchain(const Context &context, const Window &window);
	
		/**
		 * signal that the swapchain needs to be recreated
		 */
		void signalSwapchainRecreation();

    public:
    	Swapchain(const Swapchain& other);

        /**
         * @return The swapchain linked with the #SwapChain class
         * @note The reference to our Swapchain variable is needed for the recreation step
         */
        [[nodiscard]]
        const vk::SwapchainKHR& getSwapchain() const;

        /**
         * gets the current surface object
         * @return current surface
         */
        [[nodiscard]]
        const Surface& getSurface() const;

        /**
         * gets the chosen swapchain format
         * @return gets the chosen swapchain format
         */
        [[nodiscard]]
        vk::Format getFormat() const;

        /**
         * creates a swap chain object out of the given window and the given context
         * @param window a wrapper that represents a glfw window
         * @param context of the application
         * @return returns an object of swapChain
         */
        static Swapchain create(const Window &window, const Context &context);

        /**
         * Destructor of SwapChain
         */
        virtual ~Swapchain();

		/**
		 * @return number of images in swapchain
		*/
		uint32_t getImageCount() const;
	
        /**
         * @return the 2d extent of the swapchain
         */
        [[nodiscard]]
		const vk::Extent2D& getExtent() const;

		/**
		 * @return the familyQueueIndex for the surface
		 */
		[[nodiscard]]
		uint32_t getPresentQueueIndex() const;

	};
    
}
