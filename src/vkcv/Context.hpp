#pragma once

#include <vulkan/vulkan.hpp>

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
		
		static Context create(const char* applicationName, uint32_t applicationVersion);
		
	};

}
