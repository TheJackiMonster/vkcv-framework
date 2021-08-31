#pragma once
#include "vulkan/vulkan.hpp"
#include "Context.hpp"
#include "vkcv/Window.hpp"

#include <atomic>

namespace vkcv
{
	
	const uint32_t MIN_SWAPCHAIN_SIZE = 2;
	
    class Swapchain final {
    private:
    	friend class Core;
    	friend class Window;
    	friend class SwapchainManager;

        struct Surface
        {
            vk::SurfaceKHR handle;
            std::vector<vk::SurfaceFormatKHR> formats;
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::PresentModeKHR> presentModes;
        };
        
        Surface m_Surface;

        vk::SwapchainKHR m_Swapchain;
        vk::Format m_Format;
        vk::ColorSpaceKHR m_ColorSpace;
        vk::PresentModeKHR m_PresentMode;
		uint32_t m_ImageCount;
	
		vk::Extent2D m_Extent;
	
		std::atomic<bool> m_RecreationRequired;

		/**
		 * Constructor of a SwapChain object
		 * glfw is not initialized in this class because ist must be sure that there exists a context first
		 * glfw is already initialized by the window class
		 * @param surface used by the swapchain
		 * @param swapchain to show images in the window
		 * @param format of the swapchain
		 * @param colorSpace of the swapchain
		 * @param presentMode of the swapchain
		 * @param imageCount of the swapchain
		 * @param extent of the swapchain
		 */
        Swapchain(const Surface &surface,
                  vk::SwapchainKHR swapchain,
                  vk::Format format,
                  vk::ColorSpaceKHR colorSpace,
                  vk::PresentModeKHR presentMode,
                  uint32_t imageCount,
				  vk::Extent2D extent) noexcept;
	
		/**
		 * checks if the update flag is true
		 * @return if an update is needed
		 */
		bool shouldUpdateSwapchain() const;
	
		/**
		 * recreates the swapchain
		 * context
		 * window
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
        vk::SurfaceKHR getSurface() const;

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
    
    };
    
}
