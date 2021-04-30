#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace vkcv {

	class Context final {
	private:
		vk::Instance m_instance;
		vk::PhysicalDevice m_physicalDevice;
		vk::Device m_device;

		Context(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device);


	public:
		Context(const Context &other) = delete;
		Context(Context &&other) = default;

		[[nodiscard]]
		const vk::Instance& getInstance() const;
		
		[[nodiscard]]
		const vk::PhysicalDevice& getPhysicalDevice() const;
		
		[[nodiscard]]
		const vk::Device& getDevice() const;

		virtual ~Context();

		Context& operator=(const Context &other) = delete;
		Context& operator=(Context &&other) = default;

		static Context create(const char* applicationName, uint32_t applicationVersion, uint32_t queueCount = 1, const std::vector<vk::QueueFlagBits> queueFlags = {}, std::vector<const char*> instanceExtensions = {}, std::vector<const char*> deviceExtensions = {});
		static bool checkSupport(std::vector<const char*> &supported, std::vector<const char*> &check);
		static std::vector<const char*> getRequiredExtensions();
		static vk::PhysicalDevice Context::pickPhysicalDevice(vk::Instance& instance);
		static int deviceScore(const vk::PhysicalDevice &physicalDevice);
		static std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos(vk::PhysicalDevice& physicalDevice, uint32_t queueCount, std::vector<vk::QueueFlagBits> &queueFlags);
	};

}
