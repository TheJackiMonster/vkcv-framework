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

		static Context create(const char* applicationName, uint32_t applicationVersion, uint32_t queueCount = 1, std::vector<vk::QueueFlagBits> queueFlags = {}, std::vector<const char*> instanceExtensions = {}, std::vector<const char*> deviceExtensions = {});

		/**
		 * @brief With the help of the reference "supported" all elements in "check" checked,
		 * if they are supported by the physical device.
		 * @param supported The reference that can be used to check "check"
		 * @param check The elements to be checked
		 * @return True, if all elements in "check" are supported
		*/
		static bool checkSupport(std::vector<const char*> &supported, std::vector<const char*> &check);

		/**
		 * @brief Gets all extensions required, i.e. GLFW and advanced debug extensions.
		 * @return The required extensions
		*/
		static std::vector<const char*> getRequiredExtensions();

		/**
		 * @brief All existing physical devices will be evaluated by deviceScore.
		 * @param instance The instance
		 * @return The optimal physical device
		 * @see Context.deviceScore
		*/
		static vk::PhysicalDevice pickPhysicalDevice(vk::Instance& instance);

		/**
		 * @brief The physical device is evaluated by three categories:
		 * discrete GPU vs. integrated GPU, amount of queues and its abilities, and VRAM.physicalDevice.
		 * @param physicalDevice The physical device
		 * @return Device score as integer
		*/
		static int deviceScore(const vk::PhysicalDevice &physicalDevice);

		/**
		 * @brief Creates a candidate list of queues that all meet the desired flags and then creates the maximum possible number
		 * of queues. If the number of desired queues is not sufficient, the remaining queues are created from the next
		 * candidate from the list.
		 * @param physicalDevice The physical device
		 * @param queueCount The amount of queues to be created
		 * @param qPriorities 
		 * @param queueFlags The abilities which have to be supported by any created queue
		 * @return 
		*/
		static std::vector<vk::DeviceQueueCreateInfo> getQueueCreateInfos(vk::PhysicalDevice& physicalDevice, uint32_t queueCount, std::vector<float>& qPriorities, std::vector<vk::QueueFlagBits> &queueFlags);
	};

}
