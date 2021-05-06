#include "SwapChain.hpp"
#include "CoreManager.hpp"
#include <iostream>

namespace vkcv {

    SwapChain::SwapChain(vk::SurfaceKHR surface)
        : m_surface(surface)
        {}

    SwapChain SwapChain::create(GLFWwindow* window, const vk::Instance& instance, const vk::PhysicalDevice& physicalDevice, const vk::Device& device){
        vk::SurfaceKHR surface = VK_NULL_HANDLE;
        createSurface(window,surface,instance,physicalDevice);
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        if(physicalDevice.getSurfaceCapabilitiesKHR(surface,&surfaceCapabilities) != vk::Result::eSuccess){
                throw std::runtime_error("cannot get surface capabilites. There is an issue with the surface.");
        }




        return SwapChain(surface);

    }

    void SwapChain::createSurface(GLFWwindow *window, vk::SurfaceKHR &surface, const vk::Instance &instance, const vk::PhysicalDevice& physicalDevice) {
         //create surface
         auto newSurface = VkSurfaceKHR(surface);
         // 0 means VK_SUCCESS
         //std::cout << "FAIL:     " << glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &newSurface) << std::endl;
         if(glfwCreateWindowSurface(VkInstance(instance), window, nullptr, &newSurface) != VK_SUCCESS) {
             throw std::runtime_error("failed to create a window surface!");
         }
         vk::Bool32 surfaceSupport = false;
         // ToDo: hierfuer brauchen wir jetzt den queuefamiliy Index -> siehe ToDo in Context.cpp
         //if(physicalDevice.getSurfaceSupportKHR())

    }

}