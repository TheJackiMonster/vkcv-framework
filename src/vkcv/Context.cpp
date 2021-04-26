
#include "Context.hpp"

namespace vkcv {

	Context::Context(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device)
	: m_instance(instance), m_physicalDevice(physicalDevice), m_device(device)
	{}

	Context::~Context() {
		m_device.destroy();
		m_instance.destroy();
	}
	
	Context Context::create(const char* applicationName, uint32_t applicationVersion) {
		const vk::ApplicationInfo applicationInfo (
				applicationName,
				applicationVersion,
				"vkCV",
				VK_MAKE_VERSION(0, 0, 1),
				VK_HEADER_VERSION_COMPLETE
		);
		
		// TODO: enable validation layers in debug build and add required extensions
		const vk::InstanceCreateInfo instanceCreateInfo (
				vk::InstanceCreateFlags(),
				&applicationInfo,
				0,
				nullptr,
				0,
				nullptr
		);
		
		vk::Instance instance = vk::createInstance(instanceCreateInfo);
		
		// TODO: search for the best physical device (discrete GPU)
		vk::PhysicalDevice physicalDevice = instance.enumeratePhysicalDevices()[0];
		
		// TODO: create required queues, add validation layers and required extensions
		const vk::DeviceCreateInfo deviceCreateInfo (
				vk::DeviceCreateFlags(),
				0,
				nullptr,
				0,
				nullptr,
				0,
				nullptr,
				nullptr
		);
		
		vk::Device device = physicalDevice.createDevice(deviceCreateInfo);
		
		return Context(instance, physicalDevice, device);
	}

	vk::Instance Context::getInstance() {
		return m_instance;
	}

	vk::PhysicalDevice Context::getPhysicalDevice() {
		return m_physicalDevice;
	}

	vk::Device Context::getDevice() {
		return m_device;
	}
}
