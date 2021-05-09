#pragma once
/**
 * @file src/vkcv/Core.hpp
 * @brief Handling of global states regarding dependencies
 */

#include <vulkan/vulkan.hpp>
#include "vkcv/Handles.hpp"

namespace vkcv
{
    // TODO:
    class Buffer;
    class Renderpass;
    class Pipeline;

    class Core final
    {
    private:
        class Context
        {
        public:
            /**
             * Constructor of #Context requires an @p instance, a @p physicalDevice and a @p device.
             *
             * @param instance Vulkan-Instance
             * @param physicalDevice Vulkan-PhysicalDevice
             * @param device Vulkan-Device
             */
            Context(vk::Instance instance, vk::PhysicalDevice physicalDevice, vk::Device device) noexcept;
            // explicit destruction of default constructor
            Context() = delete;
            // is never called directly
            ~Context() noexcept;

            Context(const Context &other) = delete; // copy-ctor
            Context(Context &&other) noexcept; // move-ctor

            Context & operator=(const Context &other) = delete; // copy assignment
            Context & operator=(Context &&other) noexcept; // move assignment

            const vk::Instance &getInstance() const;
            const vk::PhysicalDevice &getPhysicalDevice() const;
            const vk::Device &getDevice() const;

        private:
            vk::Instance        m_Instance;
            vk::PhysicalDevice  m_PhysicalDevice;
            vk::Device          m_Device;
        } m_Context;

        /**
         * Constructor of #Core requires an @p context.
         *
         * @param context encapsulates various Vulkan objects
         */
        explicit Core(Context &&context) noexcept;
        // explicit destruction of default constructor
        Core() = delete;

    public:
        /**
         * Destructor of #Core destroys the Vulkan objects contained in the core's context.
         */
        ~Core() noexcept = default;

        /**
         * Copy-constructor of #Core is deleted!
         *
         * @param other Other instance of #Context
         */
        Core(const Core& other) = delete;

        /**
         * Move-constructor of #Core uses default behavior!
         *
         * @param other Other instance of #Context
         */
        Core(Core &&other) = delete; // move-ctor

        /**
         * Copy assignment operator of #Core is deleted!
         *
         * @param other Other instance of #Context
         * @return Reference to itself
         */
        Core & operator=(const Core &other) = delete;

        /**
         * Move assignment operator of #Core uses default behavior!
         *
         * @param other Other instance of #Context
         * @return Reference to itself
         */
        Core & operator=(Core &&other) = delete;

        [[nodiscard]]
        const Context &getContext() const;

        /**
             * Creates a #Core with given @p applicationName and @p applicationVersion for your application.
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
        static Core create(const char *applicationName,
                           uint32_t applicationVersion,
                           uint32_t queueCount,
                           std::vector<vk::QueueFlagBits> queueFlags    = {},
                           std::vector<const char*> instanceExtensions  = {},
                           std::vector<const char*> deviceExtensions    = {});

        // TODO:
        BufferHandle createBuffer(const Buffer &buf);
        PassHandle createRenderPass(const Renderpass &pass) ;
        PipelineHandle createPipeline(const Pipeline &pipeline);

    };
}
