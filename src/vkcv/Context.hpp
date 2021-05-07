#pragma once
/**
 * @authors Tobias Frisch, Vanessa Karolek, Katharina Krämer, Sebastian Gaida
 * @file src/vkcv/Context.hpp
 * @brief Context class to handle instance, physical-device and device
 */

#include <vulkan/vulkan.hpp>

namespace vkcv {

	class Context final {
	private:
		vk::Instance m_instance;
		vk::PhysicalDevice m_physicalDevice;
		vk::Device m_device;
		vk::Queue m_graphicsqueue;
		vk::Queue m_computequeue;
		vk::Queue m_transferqueue;

		/**
		 * Constructor of #Context requires an @p instance, a @p physicalDevice and a @p device.
		 *
		 * @param instance Vulkan-Instance
		 * @param physicalDevice Vulkan-PhysicalDevice
		 * @param device Vulkan-Device
		 */
		Context(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device, vk::Queue graphicsqueue, vk::Queue computequeue, vk::Queue transferqueue);

	public:
		/**
		 * Copy-constructor of #Context is deleted!
		 *
		 * @param other Other instance of #Context
		 */
		Context(const Context &other) = delete;
		
		/**
		 * Move-constructor of #Context uses default behavior!
		 *
		 * @param other Other instance of #Context
		 */
		Context(Context &&other) = default;

		/**
		 * Get the Vulkan handle for the instance.
		 *
		 * @return Vulkan-Instance
		 */
		[[nodiscard]]
		const vk::Instance& getInstance() const;
		
		/**
		 * Get the Vulkan handle for the physical-device.
		 *
		 * @return Vulkan-PhysicalDevice
		 */
		[[nodiscard]]
		const vk::PhysicalDevice& getPhysicalDevice() const;
		
		/**
		 * Get the Vulkan handle for the device.
		 *
		 * @return Vulkan-Device
		 */
		[[nodiscard]]
		const vk::Device& getDevice() const;

		/**
		 * Destructor of #Context
		 */
		virtual ~Context();

		/**
		 * Copy-operator of #Context is deleted!
		 *
		 * @param other Other instance of #Context
		 * @return Reference to itself
		 */
		Context& operator=(const Context &other) = delete;
		
		/**
		 * Move-operator of #Context uses default behavior!
		 *
		 * @param other Other instance of #Context
		 * @return Reference to itself
		 */
		Context& operator=(Context &&other) = default;

		/**
		 * Creates a #Context with given @p applicationName and @p applicationVersion for your application.
		 *
		 * It is also possible to require a specific amount of queues, ask for specific queue-flags or
		 * extensions. This function will take care of the required arguments as best as possible.
		 *
		 * To pass a valid version for your application, you should use #VK_MAKE_VERSION().
		 *
		 * @param[in] applicationName Name of the application
		 * @param[in] applicationVersion Version of the application
		 * @param[in] queueCount (optional) Amount of queues which is requested
		 * @param[in] queueFlags (optional) Requested flags of queues
		 * @param[in] instanceExtensions (optional) Requested instance extensions
		 * @param[in] deviceExtensions (optional) Requested device extensions
		 * @return New instance of #Context
		 */
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

		/**
		 * @brief finds an queue family index that fits with the given queue flags to create a queue handle
		 * @param flag The given flag that specifies as which queue type the accessed queue should be treated
		 * @param createInfos The createInfos of the created queues depending on the logical device
		 * @param device The physical with which the queue families can be accessed
		 * @return a fitting queue family index
		 */
        static int findQueueFamilyIndex(vk::QueueFlagBits flag, std::vector<vk::DeviceQueueCreateInfo> &createInfos, vk::PhysicalDevice &device);

	};

}
