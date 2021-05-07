#include "SwapChain.hpp"
#include "CoreManager.hpp"

namespace vkcv {

    SwapChain::SwapChain(vk::SurfaceKHR surface, const vkcv::Context* context)
        : m_surface(surface), m_context(context)
        {}

    SwapChain SwapChain::create(GLFWwindow* window, const vkcv::Context* context){

        const vk::Instance& instance = context->getInstance();
        const vk::PhysicalDevice& physicalDevice = context->getPhysicalDevice();
        const vk::Device& device = context->getDevice();

        vk::SurfaceKHR surface = createSurface(window,instance,physicalDevice);

        vk::Extent2D extent2D = chooseSwapExtent(physicalDevice, surface, window);
        vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(physicalDevice, surface);
        vk::PresentModeKHR presentMode = choosePresentMode(physicalDevice, surface);


//        vk::SwapchainCreateInfoKHR swapchainCreateInfo(
//                vk::SwapchainCreateFlagBitsKHR()
//                VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // VkStructureType                sType
//                nullptr,                                      // const void                    *pNext
//                0,                                            // VkSwapchainCreateFlagsKHR      flags
//                surface,                   // VkSurfaceKHR                   surface
//                desired_number_of_images,                     // uint32_t                       minImageCount
//                desired_format.format,                        // VkFormat                       imageFormat
//                desired_format.colorSpace,                    // VkColorSpaceKHR                imageColorSpace
//                desired_extent,                               // VkExtent2D                     imageExtent
//                1,                                            // uint32_t                       imageArrayLayers
//                desired_usage,                                // VkImageUsageFlags              imageUsage
//                VK_SHARING_MODE_EXCLUSIVE,                    // VkSharingMode                  imageSharingMode
//                0,                                            // uint32_t                       queueFamilyIndexCount
//                nullptr,                                      // const uint32_t                *pQueueFamilyIndices
//                desired_transform,                            // VkSurfaceTransformFlagBitsKHR  preTransform
//                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,            // VkCompositeAlphaFlagBitsKHR    compositeAlpha
//                desired_present_mode,                         // VkPresentModeKHR               presentMode
//                VK_TRUE,                                      // VkBool32                       clipped
//                old_swap_chain                                // VkSwapchainKHR                 oldSwapchain
//        );

//        vk::SwapchainCreateInfoKHR swapchainCreateInfo(
//                vk::SwapchainCreateFlagsKHR(),  //flags
//                surface, // surface
//                surfaceCapabilities.minImageCount, // minImageCount TODO: how many do we need for our application?? "must be less than or equal to the value returned in maxImageCount"
//                vk::Format::eB8G8R8A8Unorm,         // imageFormat  TODO: what image format should be used?
//                vk::ColorSpaceKHR::eSrgbNonlinear, // imageColorSpace   TODO: which color space should be used?
//                vk::Extent2D(width, height), // imageExtent
//                1, // imageArrayLayers TODO: should we only allow non-stereoscopic applications? yes -> 1, no -> ? "must be greater than 0, less or equal to maxImageArrayLayers"
//                vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eDepthStencilAttachment, // imageUsage   TODO: what attachments? only color? depth?
//                vk::SharingMode::eExclusive, // imageSharingMode TODO: which sharing mode? "VK_SHARING_MODE_EXCLUSIV access exclusive to a single queue family, better performance", "VK_SHARING_MODE_CONCURRENT access from multiple queues"
//                0, // queueFamilyIndexCount, the number of queue families having access to the image(s) of the swapchain when imageSharingMode is VK_SHARING_MODE_CONCURRENT
//                nullptr, // pQueueFamilyIndices, the pointer to an array of queue family indices having access to the images(s) of the swapchain when imageSharingMode is VK_SHARING_MODE_CONCURRENT
//                vk::SurfaceTransformFlagBitsKHR::eIdentity, // preTransform, transformations applied onto the image before display
//                vk::CompositeAlphaFlagBitsKHR::eOpaque, // compositeAlpha, TODO: how to handle transparent pixels? do we need transparency? If no -> opaque
//                vk::PresentModeKHR::eFifo, // presentMode
//                true, // clipped
//                nullptr // oldSwapchain
//        );

        return SwapChain(surface, context);

    }

    vk::SurfaceKHR SwapChain::createSurface(GLFWwindow *window, const vk::Instance &instance, const vk::PhysicalDevice& physicalDevice) {
         //create surface
         VkSurfaceKHR surface;
         // 0 means VK_SUCCESS
         //std::cout << "FAIL:     " << glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &newSurface) << std::endl;
         if(glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &surface) != VK_SUCCESS) {
             throw std::runtime_error("failed to create a window surface!");
         }
         vk::Bool32 surfaceSupport = false;
         // ToDo: hierfuer brauchen wir jetzt den queuefamiliy Index -> siehe ToDo in Context.cpp
         //if(physicalDevice.getSurfaceSupportKHR())
        return vk::SurfaceKHR(surface);
    }

    vk::Extent2D SwapChain::chooseSwapExtent(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, GLFWwindow* window){
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        if(physicalDevice.getSurfaceCapabilitiesKHR(surface,&surfaceCapabilities) != vk::Result::eSuccess){
            throw std::runtime_error("cannot get surface capabilities. There is an issue with the surface.");
        }

        VkExtent2D extent2D = {
                static_cast<uint32_t>(vkcv::getWidth(window)),
                static_cast<uint32_t>(vkcv::getHeight(window))
        };
        extent2D.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent2D.width));
        extent2D.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent2D.height));

        if (extent2D.width > surfaceCapabilities.maxImageExtent.width ||
            extent2D.width < surfaceCapabilities.minImageExtent.width ||
            extent2D.height > surfaceCapabilities.maxImageExtent.height ||
            extent2D.height < surfaceCapabilities.minImageExtent.height) {
            std::printf("Surface size not matching. Resizing to allowed value.");
        }
        return extent2D;
    }

    vk::SurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
        uint32_t formatCount;
        physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, nullptr);
        std::vector<vk::SurfaceFormatKHR> availableFormats(formatCount);
        if (physicalDevice.getSurfaceFormatsKHR(surface, &formatCount, &availableFormats[0]) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to get surface formats");
        }

        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == vk::Format::eB8G8R8A8Unorm  && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    vk::PresentModeKHR SwapChain::choosePresentMode(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface) {
        uint32_t modeCount;
        physicalDevice.getSurfacePresentModesKHR( surface, &modeCount, nullptr );
        std::vector<vk::PresentModeKHR> availablePresentModes(modeCount);
        if (physicalDevice.getSurfacePresentModesKHR(surface, &modeCount, &availablePresentModes[0]) != vk::Result::eSuccess) {
            throw std::runtime_error("Failed to get presentation modes");
        }

        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }
        return vk::PresentModeKHR::eFifo;
    }

    SwapChain::~SwapChain() {
//      m_context->getDevice().destroySwapchainKHR( m_swapChain );
      m_context->getInstance().destroySurfaceKHR( m_surface );
    }

}
