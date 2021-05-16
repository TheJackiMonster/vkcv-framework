#pragma once
/**
 * @file src/vkcv/Core.hpp
 * @brief Handling of global states regarding dependencies
 */

#include <vulkan/vulkan.hpp>
#include "vkcv/Context.hpp"
#include "vkcv/SwapChain.hpp"
#include "vkcv/Window.hpp"
#include "vkcv/Renderpass.hpp"
#include "vkcv/Handles.hpp"
#include "vkcv/Pipeline.hpp"

namespace vkcv
{
    // TODO:
    class Buffer;
    class Renderpass;

    class Core final
    {
    private:

        /**
         * Constructor of #Core requires an @p context.
         *
         * @param context encapsulates various Vulkan objects
         */
        Core(Context &&context, const Window &window, SwapChain swapChain,  std::vector<vk::ImageView> imageViews) noexcept;
        // explicit destruction of default constructor
        Core() = delete;

        Context m_Context;

        SwapChain m_swapchain;
        std::vector<vk::ImageView> m_swapchainImageViews;
        const Window& m_window;

        uint64_t m_NextPipelineId;
        std::vector<vk::Pipeline> m_Pipelines;
        std::vector<vk::PipelineLayout> m_PipelineLayouts;

        uint64_t m_NextRenderpassId;

        uint64_t m_NextPassId;
        std::vector<vk::RenderPass> m_Renderpasses;

    public:
        /**
         * Destructor of #Core destroys the Vulkan objects contained in the core's context.
         */
        ~Core() noexcept;

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
        static Core create(const Window &window,
                           const char *applicationName,
                           uint32_t applicationVersion,
                           uint32_t queueCount,
                           std::vector<vk::QueueFlagBits> queueFlags    = {},
                           std::vector<const char*> instanceExtensions  = {},
                           std::vector<const char*> deviceExtensions    = {});

        /**
         * Creates a basic vulkan graphics pipeline using @p pipeline from the pipeline class and returns it using the @p handle.
         * Fixed Functions for pipeline are set with standard values.
         *
         * @param pipeline a pipeline object from the pipeline class
         * @param handle a handle to return the created vulkan handle
         * @return True if Pipeline creation was successfull, False if not
         */
        bool createGraphicsPipeline(const Pipeline &pipeline, PipelineHandle &handle);

        // TODO:
        BufferHandle createBuffer(const Buffer &buf);

        [[nodiscard]]
        bool createRenderpass(const Renderpass &pass, RenderpassHandle &handle);
    };
}
