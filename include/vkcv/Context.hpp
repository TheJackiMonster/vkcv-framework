#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.hpp>

#include "QueueManager.hpp"

namespace vkcv
{
    class Context
    {
        friend class Core;
    public:
        // explicit destruction of default constructor
        Context() = delete;
        // is never called directly
        ~Context() noexcept;

        Context(const Context &other) = delete; // copy-ctor
        Context(Context &&other) noexcept; // move-ctor

        Context & operator=(const Context &other) = delete; // copy assignment
        Context & operator=(Context &&other) noexcept; // move assignment

        [[nodiscard]]
        const vk::Instance &getInstance() const;
        
        [[nodiscard]]
        const vk::PhysicalDevice &getPhysicalDevice() const;
        
        [[nodiscard]]
        const vk::Device &getDevice() const;
        
        [[nodiscard]]
        const QueueManager& getQueueManager() const;
	
        [[nodiscard]]
		const vma::Allocator& getAllocator() const;
        
        static Context create(const char *applicationName,
							  uint32_t applicationVersion,
							  std::vector<vk::QueueFlagBits> queueFlags,
							  std::vector<const char *> instanceExtensions,
							  std::vector<const char *> deviceExtensions);

    private:
        /**
         * Constructor of #Context requires an @p instance, a @p physicalDevice and a @p device.
         *
         * @param instance Vulkan-Instance
         * @param physicalDevice Vulkan-PhysicalDevice
         * @param device Vulkan-Device
         */
        Context(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device,
				QueueManager&& queueManager, vma::Allocator&& allocator) noexcept;
        
        vk::Instance        m_Instance;
        vk::PhysicalDevice  m_PhysicalDevice;
        vk::Device          m_Device;
		QueueManager		m_QueueManager;
		vma::Allocator 		m_Allocator;
		
    };
}
