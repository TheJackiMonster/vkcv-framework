#pragma once
/**
 * @authors Tobias Frisch, Artur Wasmut, Sebastian Gaida, Alexander Gauggel
 * @file vkcv/Context.hpp
 * @brief Class to handle the instance, device, allocator and features of the current context.
 */

#include <string>
#include <vk_mem_alloc.hpp>
#include <vulkan/vulkan.hpp>

#include "Features.hpp"
#include "Handles.hpp"
#include "QueueManager.hpp"

namespace vkcv {

	/**
	 * @brief Class to manage core resources for vulkan callbacks.
	 *
	 * The class to manage the vulkan resources as an instance,
	 * a device, a physical device and a memory allocator. Additionally
	 * instances of this class will hold the feature manager and the
	 * queue manager.
	 */
	class Context {
		friend class Core;

	public:
		// explicit destruction of default constructor
		Context() = delete;
		// is never called directly
		~Context() noexcept;

		Context(const Context &other) = delete; // copy-ctor
		Context(Context &&other) noexcept;      // move-ctor

		Context &operator=(const Context &other) = delete; // copy assignment
		Context &operator=(Context &&other) noexcept;      // move assignment

		/**
		 * @brief Returns the vulkan instance of the context.
		 *
		 * @return Vulkan instance
		 */
		[[nodiscard]] const vk::Instance &getInstance() const;

		/**
		 * @brief Returns the vulkan physical device of the context.
		 *
		 * @return Vulkan physical device
		 */
		[[nodiscard]] const vk::PhysicalDevice &getPhysicalDevice() const;

		/**
		 * @brief Returns the vulkan device of the context.
		 *
		 * @return Vulkan device
		 */
		[[nodiscard]] const vk::Device &getDevice() const;
		
		/**
		 * @brief Returns a dynamic dispatch loader of the context.
		 *
		 * @return Dynamic dispatch loader
		 */
		[[nodiscard]] const vk::DispatchLoaderDynamic &getDispatchLoaderDynamic() const;

		/**
		 * @brief Returns the feature manager of the context.
		 *
		 * @return Feature manager
		 */
		[[nodiscard]] const FeatureManager &getFeatureManager() const;

		/**
		 * @brief Returns the queue manager of the context.
		 *
		 * @return Queue manager
		 */
		[[nodiscard]] const QueueManager &getQueueManager() const;

		/**
		 * @brief Returns the VMA allocator of the context.
		 *
		 * @return VMA allocator
		 */
		[[nodiscard]] const vma::Allocator &getAllocator() const;

		/**
		 * @brief Creates a context for a given application with
		 * a specific name, version, queue requirements, features and
		 * required instance extensions.
		 *
		 * @param applicationName Application name
		 * @param applicationVersion  Application version
		 * @param queueFlags Queue flags
		 * @param features Features
		 * @param instanceExtensions Instance extensions
		 * @return New context
		 */
		static Context create(const std::string &applicationName, uint32_t applicationVersion,
							  const std::vector<vk::QueueFlagBits> &queueFlags,
							  const Features &features,
							  const std::vector<const char*> &instanceExtensions = {});

	private:
		/**
		 * @brief Constructor of #Context requires an @p instance,
		 * a @p physicalDevice and a @p device.
		 *
		 * @param instance Vulkan-Instance
		 * @param physicalDevice Vulkan-PhysicalDevice
		 * @param device Vulkan-Device
		 */
		Context(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device,
				FeatureManager &&featureManager, QueueManager &&queueManager,
				vma::Allocator &&allocator) noexcept;

		vk::Instance m_Instance;
		vk::PhysicalDevice m_PhysicalDevice;
		vk::Device m_Device;
		vk::DispatchLoaderDynamic m_DispatchDynamic;
		FeatureManager m_FeatureManager;
		QueueManager m_QueueManager;
		vma::Allocator m_Allocator;
	};

} // namespace vkcv
