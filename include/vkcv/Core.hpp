#pragma once
/**
 * @file src/vkcv/Core.hpp
 * @brief Handling of global states regarding dependencies
 */

#include <memory>
#include <vulkan/vulkan.hpp>

#include "vkcv/Context.hpp"
#include "vkcv/Swapchain.hpp"
#include "vkcv/Window.hpp"
#include "vkcv/PassConfig.hpp"
#include "vkcv/Handles.hpp"
#include "vkcv/Buffer.hpp"
#include "vkcv/Image.hpp"
#include "vkcv/PipelineConfig.hpp"
#include "CommandResources.hpp"
#include "SyncResources.hpp"
#include "Result.hpp"
#include "vkcv/DescriptorConfig.hpp"
#include "Sampler.hpp"
#include "DescriptorWrites.hpp"
#include "Event.hpp"
#include "DrawcallRecording.hpp"
#include "CommandRecordingFunctionTypes.hpp"

namespace vkcv
{

    // forward declarations
    class PassManager;
    class PipelineManager;
    class DescriptorManager;
    class BufferManager;
    class SamplerManager;
    class ImageManager;
	class CommandStreamManager;

	struct SubmitInfo {
		QueueType queueType;
		std::vector<vk::Semaphore> waitSemaphores;
		std::vector<vk::Semaphore> signalSemaphores;
	};

    class Core final
    {
    private:

        /**
         * Constructor of #Core requires an @p context.
         *
         * @param context encapsulates various Vulkan objects
         */
        Core(Context &&context, Window &window, const Swapchain& swapChain,  std::vector<vk::ImageView> imageViews,
			const CommandResources& commandResources, const SyncResources& syncResources) noexcept;
        // explicit destruction of default constructor
        Core() = delete;

		Result acquireSwapchainImage();

        Context m_Context;

        Swapchain                       m_swapchain;
        Window&                   		m_window;

        std::unique_ptr<PassManager>            m_PassManager;
        std::unique_ptr<PipelineManager>        m_PipelineManager;
        std::unique_ptr<DescriptorManager>      m_DescriptorManager;
        std::unique_ptr<BufferManager>          m_BufferManager;
        std::unique_ptr<SamplerManager>         m_SamplerManager;
        std::unique_ptr<ImageManager>           m_ImageManager;
        std::unique_ptr<CommandStreamManager>   m_CommandStreamManager;

		CommandResources    m_CommandResources;
		SyncResources       m_SyncResources;
		uint32_t            m_currentSwapchainImageIndex;

		event_handle<int,int> e_resizeHandle;

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
        
        [[nodiscard]]
        const Swapchain& getSwapchain() const;

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
             * @param[in] queueFlags (optional) Requested flags of queues
             * @param[in] instanceExtensions (optional) Requested instance extensions
             * @param[in] deviceExtensions (optional) Requested device extensions
             * @return New instance of #Context
             */
        static Core create(Window &window,
                           const char *applicationName,
                           uint32_t applicationVersion,
                           const std::vector<vk::QueueFlagBits>& queueFlags    = {},
                           const std::vector<const char*>& instanceExtensions  = {},
                           const std::vector<const char*>& deviceExtensions    = {});

        /**
         * Creates a basic vulkan graphics pipeline using @p config from the pipeline config class and returns it using the @p handle.
         * Fixed Functions for pipeline are set with standard values.
         *
         * @param config a pipeline config object from the pipeline config class
         * @param handle a handle to return the created vulkan handle
         * @return True if pipeline creation was successful, False if not
         */
        [[nodiscard]]
        PipelineHandle createGraphicsPipeline(const PipelineConfig &config);

        /**
         * Creates a basic vulkan compute pipeline using @p shader program and returns it using the @p handle.
         * Fixed Functions for pipeline are set with standard values.
         *
         * @param shader program that hold the compiles compute shader
         * @param handle a handle to return the created vulkan handle
         * @return True if pipeline creation was successful, False if not
         */
        [[nodiscard]]
        PipelineHandle createComputePipeline(
            const ShaderProgram &shaderProgram,
            const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts);

        /**
         * Creates a basic vulkan render pass using @p config from the render pass config class and returns it using the @p handle.
         * Fixed Functions for pipeline are set with standard values.
         *
         * @param config a render pass config object from the render pass config class
         * @param handle a handle to return the created vulkan handle
         * @return True if render pass creation was successful, False if not
         */
        [[nodiscard]]
        PassHandle createPass(const PassConfig &config);

        /**
            * Creates a #Buffer with data-type T and @p bufferType
            * @param type Type of Buffer created
            * @param count Count of elements of type T
            * @param memoryType Type of Buffers memory
            * return Buffer-Object
            */
        template<typename T>
        Buffer<T> createBuffer(vkcv::BufferType type, size_t count, BufferMemoryType memoryType = BufferMemoryType::DEVICE_LOCAL) {
        	return Buffer<T>::create(m_BufferManager.get(), type, count, memoryType);
        }
        
        /**
         * Creates a Sampler with given attributes.
         *
         * @param magFilter Magnifying filter
         * @param minFilter Minimizing filter
         * @param mipmapMode Mipmapping filter
         * @param addressMode Address mode
         * @param mipLodBias Mip level of detail bias
         * @return Sampler handle
         */
        [[nodiscard]]
        SamplerHandle createSampler(SamplerFilterType magFilter, SamplerFilterType minFilter,
									SamplerMipmapMode mipmapMode, SamplerAddressMode addressMode,
									float mipLodBias = 0.0f);

        /**
         * Creates an #Image with a given format, width, height and depth.
         *
         * @param format Image format
         * @param width Image width
         * @param height Image height
         * @param depth Image depth
         * @return Image-Object
         */
        [[nodiscard]]
        Image createImage(
			vk::Format      format,
			uint32_t        width,
			uint32_t        height,
			uint32_t        depth = 1,
			bool            createMipChain = false,
			bool            supportStorage = false,
			bool            supportColorAttachment = false,
			Multisampling   multisampling = Multisampling::None);

        [[nodiscard]]
        uint32_t getImageWidth(const ImageHandle& image);
        
        [[nodiscard]]
        uint32_t getImageHeight(const ImageHandle& image);
	
		[[nodiscard]]
		vk::Format getImageFormat(const ImageHandle& image);

        /** TODO:
         *   @param setDescriptions
         *   @return
         */
        [[nodiscard]]
        DescriptorSetHandle createDescriptorSet(const std::vector<DescriptorBinding> &bindings);
		void writeDescriptorSet(DescriptorSetHandle handle, const DescriptorWrites& writes);

		DescriptorSet getDescriptorSet(const DescriptorSetHandle handle) const;

		/**
		 * @brief start recording command buffers and increment frame index
		*/
		bool beginFrame(uint32_t& width, uint32_t& height);

		void recordDrawcallsToCmdStream(
			const CommandStreamHandle       cmdStreamHandle,
			const PassHandle                renderpassHandle, 
			const PipelineHandle            pipelineHandle,
			const PushConstants             &pushConstants,
			const std::vector<DrawcallInfo> &drawcalls,
			const std::vector<ImageHandle>  &renderTargets);

		void recordMeshShaderDrawcalls(
			const CommandStreamHandle               cmdStreamHandle,
			const PassHandle                        renderpassHandle,
			const PipelineHandle                    pipelineHandle,
			const PushConstants&                    pushConstantData,
            const std::vector<MeshShaderDrawcall>&  drawcalls,
			const std::vector<ImageHandle>&         renderTargets);

		void recordComputeDispatchToCmdStream(
			CommandStreamHandle cmdStream,
			PipelineHandle computePipeline,
			const uint32_t dispatchCount[3],
			const std::vector<DescriptorSetUsage> &descriptorSetUsages,
			const PushConstants& pushConstants);
		
		void recordBeginDebugLabel(const CommandStreamHandle &cmdStream,
								   const std::string& label,
								   const std::array<float, 4>& color);
		
		void recordEndDebugLabel(const CommandStreamHandle &cmdStream);

		/**
		 * @brief end recording and present image
		*/
		void endFrame();

		/**
		 * Submit a command buffer to any queue of selected type. The recording can be customized by a
		 * custom record-command-function. If the command submission has finished, an optional finish-function
		 * will be called.
		 *
		 * @param submitInfo Submit information
		 * @param record Record-command-function
		 * @param finish Finish-command-function or nullptr
		 */
		void recordAndSubmitCommandsImmediate(
			const SubmitInfo            &submitInfo, 
			const RecordCommandFunction &record, 
			const FinishCommandFunction &finish);

		CommandStreamHandle createCommandStream(QueueType queueType);

		void recordCommandsToStream(
			const CommandStreamHandle   cmdStreamHandle,
			const RecordCommandFunction &record,
			const FinishCommandFunction &finish);

		void submitCommandStream(const CommandStreamHandle& handle);
		void prepareSwapchainImageForPresent(const CommandStreamHandle& handle);
		void prepareImageForSampling(const CommandStreamHandle& cmdStream, const ImageHandle& image);
		void prepareImageForStorage(const CommandStreamHandle& cmdStream, const ImageHandle& image);
		void recordImageMemoryBarrier(const CommandStreamHandle& cmdStream, const ImageHandle& image);
		void recordBufferMemoryBarrier(const CommandStreamHandle& cmdStream, const BufferHandle& buffer);
		void resolveMSAAImage(const CommandStreamHandle& cmdStream, const ImageHandle& src, const ImageHandle& dst);

		[[nodiscard]]
		vk::ImageView getSwapchainImageView() const;
	
		void recordMemoryBarrier(const CommandStreamHandle& cmdStream);
		
		void recordBlitImage(const CommandStreamHandle& cmdStream, const ImageHandle& src, const ImageHandle& dst,
							 SamplerFilterType filterType);
		
    };
}
