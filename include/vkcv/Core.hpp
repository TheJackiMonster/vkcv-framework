#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch, Sebastian Gaida, Artur Wasmut, Lars Hoerttrich,
 *          Mara Vogt, Mark Mints, Simeon Hermann, Alex Laptop, Katharina Krämer, Vanessa Karolek
 * @file vkcv/Core.hpp
 * @brief Handling of global states regarding dependencies.
 */

#include <memory>
#include <vulkan/vulkan.hpp>

#include "Context.hpp"
#include "Swapchain.hpp"
#include "Window.hpp"
#include "PassConfig.hpp"
#include "Handles.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "GraphicsPipelineConfig.hpp"
#include "ComputePipelineConfig.hpp"
#include "CommandResources.hpp"
#include "SyncResources.hpp"
#include "Result.hpp"
#include "vkcv/DescriptorConfig.hpp"
#include "Sampler.hpp"
#include "DescriptorWrites.hpp"
#include "Event.hpp"
#include "DrawcallRecording.hpp"
#include "CommandRecordingFunctionTypes.hpp"

#define VKCV_FRAMEWORK_NAME "VkCV"
#define VKCV_FRAMEWORK_VERSION (VK_MAKE_VERSION(0, 1, 0))

namespace vkcv
{

    // forward declarations
    class PassManager;
    class GraphicsPipelineManager;
    class ComputePipelineManager;
    class DescriptorManager;
    class BufferManager;
    class SamplerManager;
    class ImageManager;
	class CommandStreamManager;
	class WindowManager;
	class SwapchainManager;

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
        Core(Context &&context, const CommandResources& commandResources, const SyncResources& syncResources) noexcept;
        // explicit destruction of default constructor
        Core() = delete;

		Result acquireSwapchainImage(const SwapchainHandle &swapchainHandle);

        Context m_Context;

        std::unique_ptr<PassManager>             m_PassManager;
        std::unique_ptr<GraphicsPipelineManager> m_PipelineManager;
        std::unique_ptr<ComputePipelineManager>  m_ComputePipelineManager;
        std::unique_ptr<DescriptorManager>       m_DescriptorManager;
        std::unique_ptr<BufferManager>           m_BufferManager;
        std::unique_ptr<SamplerManager>          m_SamplerManager;
        std::unique_ptr<ImageManager>            m_ImageManager;
        std::unique_ptr<CommandStreamManager>    m_CommandStreamManager;
        std::unique_ptr<WindowManager>           m_WindowManager;
        std::unique_ptr<SwapchainManager>        m_SwapchainManager;

		CommandResources    m_CommandResources;
		SyncResources       m_SyncResources;
		uint32_t            m_currentSwapchainImageIndex;

		/**
		 * Sets up swapchain images
		 * @param handle Handle of swapchain
		 */
		void setSwapchainImages(SwapchainHandle handle);

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
         * @param other Other instance of Context
         * @return Reference to itself
         */
        Core & operator=(const Core &other) = delete;

        /**
         * Move assignment operator of #Core uses default behavior!
         *
         * @param other Other instance of Context
         * @return Reference to itself
         */
        Core & operator=(Core &&other) = delete;

		/**
		 * Returns the context of a Core instance.
		 *
		 * @return Current Context
		 */
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
             * @param[in] queueFlags (optional) Requested flags of queues
             * @param[in] instanceExtensions (optional) Requested instance extensions
             * @param[in] deviceExtensions (optional) Requested device extensions
             * @return New instance of #Context
             */
        static Core create(const char *applicationName,
                           uint32_t applicationVersion,
                           const std::vector<vk::QueueFlagBits>& queueFlags    = {},
						   const Features& features = {},
						   const std::vector<const char *>& instanceExtensions = {});

        /**
         * Creates a basic vulkan graphics pipeline using @p config from the pipeline config class and returns it using the @p handle.
         * Fixed Functions for pipeline are set with standard values.
         *
         * @param config a pipeline config object from the pipeline config class
         * @param handle a handle to return the created vulkan handle
         * @return True if pipeline creation was successful, False if not
         */
        [[nodiscard]]
		GraphicsPipelineHandle createGraphicsPipeline(const GraphicsPipelineConfig &config);

        /**
         * Creates a basic vulkan compute pipeline using @p shader program and returns it using the @p handle.
         * Fixed Functions for pipeline are set with standard values.
         *
         * @param config Contains the compiles compute shader and the corresponding descriptor set layout
         * @return True if pipeline creation was successful, False if not
         */
        [[nodiscard]]
        ComputePipelineHandle createComputePipeline(const ComputePipelineConfig &config);

        /**
         * Creates a basic vulkan render pass using @p config from the render pass config class and returns it.
         * Fixed Functions for pipeline are set with standard values.
         *
         * @param config a render pass config object from the render pass config class
         * @return A handle to represent the created pass
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
        Buffer<T> createBuffer(vkcv::BufferType type, size_t count, BufferMemoryType memoryType = BufferMemoryType::DEVICE_LOCAL, bool supportIndirect = false) {
        	return Buffer<T>::create(m_BufferManager.get(), type, count, memoryType, supportIndirect);
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
									float mipLodBias = 0.0f, SamplerBorderColor borderColor = SamplerBorderColor::INT_ZERO_OPAQUE);

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

        /**
         * Creates a new window and returns it's handle
         * @param applicationName window name
         * @param windowWidth
         * @param windowHeight
         * @param resizeable resizeability bool
         * @return windowHandle
         */
		[[nodiscard]]
		WindowHandle createWindow(
				const char *applicationName,
				uint32_t windowWidth,
				uint32_t windowHeight,
				bool resizeable);

		/**
		 * Getter for window reference
		 * @param handle of the window
		 * @return the window
		 */
		[[nodiscard]]
		Window& getWindow(const WindowHandle& handle );

		/**
		 * Gets the swapchain of the current focused window
		 * @return swapchain
		 */
		[[nodiscard]]
		Swapchain& getSwapchainOfCurrentWindow();

		/**
		 * Returns the swapchain reference
		 * @param handle of the swapchain
		 * @return swapchain
		 */
		[[nodiscard]]
		Swapchain& getSwapchain(const SwapchainHandle &handle);

		/**
		 * Gets the swapchain handle from the window
		 * @param handle of the window
		 * @return the swapchain from getSwapchain( SwapchainHandle )
		 */
		[[nodiscard]]
		Swapchain& getSwapchain(const WindowHandle &handle);

		/**
		 * Returns the image width
		 * @param image handle
		 * @return imageWidth
		 */
        [[nodiscard]]
        uint32_t getImageWidth(const ImageHandle &image);

        /**
         * Returns the image height
         * @param image handle
         * @return imageHeight
         */
        [[nodiscard]]
        uint32_t getImageHeight(const ImageHandle &image);

        /**
         * Returns the image format of the image
         * @param image handle
         * @return imageFormat
         */
		[[nodiscard]]
		vk::Format getImageFormat(const ImageHandle &image);
		
		/**
		 * @brief Returns the images amount of mip levels.
		 *
		 * @param image Image handle
		 * @return Amount of mip levels
		 */
		[[nodiscard]]
		uint32_t getImageMipLevels(const ImageHandle &image);

		/**
		 * @brief Creates a descriptor set layout handle by a set of descriptor bindings.
		 *
		 * @param bindings Descriptor bindings
		 * @return Descriptor set layout handle
		 */
		[[nodiscard]]
		DescriptorSetLayoutHandle createDescriptorSetLayout(const DescriptorBindings &bindings);
		
		/**
		 * @brief Returns the descriptor set layout of a descriptor set layout handle.
		 *
		 * @param handle Descriptor set layout handle
		 * @return Descriptor set layout
		 */
		DescriptorSetLayout getDescriptorSetLayout(const DescriptorSetLayoutHandle handle) const;

		/**
		 * @brief Creates a new descriptor set
		 * 
		 * @param layoutHandle Handle to the layout that the descriptor set will use
		 * @return Handle that represents the descriptor set
		 */
        [[nodiscard]]
        DescriptorSetHandle createDescriptorSet(const DescriptorSetLayoutHandle &layoutHandle);

		/**
		 * @brief Writes resources bindings to a descriptor set
		 * 
		 * @param handle Handle of the descriptor set
		 * @param writes Struct containing the resource bindings to be written
		 * must be compatible with the descriptor set's layout
		*/
		void writeDescriptorSet(DescriptorSetHandle handle, const DescriptorWrites& writes);

		/**
		 * @brief Returns information about a descriptor set
		 * 
		 * @param handle Handle of the descriptor set
		 * @return Struct containing the descriptor set's vulkan handle, layout handle and descriptor pool index
		*/
		DescriptorSet getDescriptorSet(const DescriptorSetHandle handle) const;


		/**
		 * @brief Start recording command buffers and increment frame index
		*/
		bool beginFrame(uint32_t& width, uint32_t& height, const WindowHandle &windowHandle);

		/**
		 * @brief Records drawcalls to a command stream
		 * 
		 * @param cmdStreamHandle Handle of the command stream that the drawcalls are recorded into
		 * @param renderpassHandle Handle of the renderpass that is used for the drawcalls
		 * @param pipelineHandle Handle of the pipeline that is used for the drawcalls
		 * @param pushConstants Push constants that are used for the drawcalls, ignored if constant size is set to 0
		 * @param drawcalls Information about each drawcall, consisting of mesh handle, descriptor set bindings and instance count
		 * @param renderTargets Image handles that are used as render targets
		 * @param windowHandle Window handle that is used to retrieve the corresponding swapchain
		*/
		void recordDrawcallsToCmdStream(
			const CommandStreamHandle&      cmdStreamHandle,
			const PassHandle&               renderpassHandle,
			const GraphicsPipelineHandle    &pipelineHandle,
			const PushConstants             &pushConstants,
			const std::vector<DrawcallInfo> &drawcalls,
			const std::vector<ImageHandle>  &renderTargets,
			const WindowHandle              &windowHandle);
	
		/**
		 * @brief Records indirect drawcalls to a command stream
		 *
		 * @param cmdStreamHandle Handle of the command stream that the drawcalls are recorded into
		 * @param renderpassHandle Handle of the renderpass that is used for the drawcalls
		 * @param pipelineHandle Handle of the pipeline that is used for the drawcalls
		 * @param pushConstantData Push constants that are used for the drawcalls, ignored if constant size is set to 0
		 * @param compiledDescriptorSet TODO
		 * @param compiledMesh TODO
		 * @param drawcalls Information about each drawcall, consisting of mesh handle, descriptor set bindings and instance count
		 * @param renderTargets Image handles that are used as render targets
		 * @param indirectBuffer TODO
		 * @param drawCount TODO
		 * @param windowHandle Window handle that is used to retrieve the corresponding swapchain
		*/
		void recordIndexedIndirectDrawcallsToCmdStream(
				const CommandStreamHandle                           cmdStreamHandle,
				const PassHandle                                    renderpassHandle,
				const GraphicsPipelineHandle                        &pipelineHandle,
				const PushConstants                                 &pushConstantData,
                const vkcv::DescriptorSetHandle                     &compiledDescriptorSet,
				const vkcv::Mesh                                    &compiledMesh,
				const std::vector<ImageHandle>                      &renderTargets,
				const vkcv::Buffer<vk::DrawIndexedIndirectCommand>  &indirectBuffer,
				const uint32_t                                      drawCount,
				const WindowHandle                                  &windowHandle);
		
		/**
		 * @brief Records mesh shader drawcalls to a command stream
		 *
		 * @param cmdStreamHandle Handle of the command stream that the drawcalls are recorded into
		 * @param renderpassHandle Handle of the renderpass that is used for the drawcalls
		 * @param pipelineHandle Handle of the pipeline that is used for the drawcalls
		 * @param pushConstantData Push constants that are used for the drawcalls, ignored if constant size is set to 0
		 * @param drawcalls Information about each drawcall, consisting of descriptor set bindings and task shader dispatch count
		 * @param renderTargets Image handles that are used as render targets
		 * @param windowHandle Window handle that is used to retrieve the corresponding swapchain
		*/
		void recordMeshShaderDrawcalls(
			const CommandStreamHandle&              cmdStreamHandle,
			const PassHandle&                       renderpassHandle,
			const GraphicsPipelineHandle            &pipelineHandle,
			const PushConstants&                    pushConstantData,
            const std::vector<MeshShaderDrawcall>&  drawcalls,
			const std::vector<ImageHandle>&         renderTargets,
			const WindowHandle&                     windowHandle);
		
        /**
         * Records the rtx ray generation to the @p cmdStreamHandle.
         * Currently only supports @p closestHit, @p rayGen and @c miss shaderstages @c.
         *
         * @param cmdStreamHandle The command stream handle which receives relevant commands for drawing.
         * @param rtxPipeline The raytracing pipeline from the RTXModule.
         * @param rtxPipelineLayout The raytracing pipeline layout from the RTXModule.
         * @param rgenRegion The shader binding table region for ray generation shaders.
         * @param rmissRegion The shader binding table region for ray miss shaders.
         * @param rchitRegion The shader binding table region for ray closest hit shaders.
         * @param rcallRegion The shader binding table region for callable shaders.
         * @param descriptorSetUsages The descriptor set usages.
         * @param pushConstants The push constants.
         * @param windowHandle The window handle defining in which window to render.
         */
        void recordRayGenerationToCmdStream(
            CommandStreamHandle cmdStreamHandle,
            vk::Pipeline rtxPipeline,
            vk::PipelineLayout rtxPipelineLayout,
            vk::StridedDeviceAddressRegionKHR rgenRegion,
            vk::StridedDeviceAddressRegionKHR rmissRegion,
            vk::StridedDeviceAddressRegionKHR rchitRegion,
            vk::StridedDeviceAddressRegionKHR rcallRegion,
            const std::vector<DescriptorSetUsage>& descriptorSetUsages,
            const PushConstants& pushConstants,
            const WindowHandle windowHandle);

		/**
		 * @brief Record a compute shader dispatch into a command stream
		 * 
		 * @param cmdStream Handle of the command stream that the dispatch is recorded into
		 * @param computePipeline Handle of the pipeline that is used for the dispatch
		 * @param dispatchCount How many work groups are dispatched
		 * @param descriptorSetUsages Descriptor set bindings of the dispatch
		 * @param pushConstants Push constant data for the dispatch
		 */
		void recordComputeDispatchToCmdStream(
			CommandStreamHandle cmdStream,
            ComputePipelineHandle computePipeline,
			const uint32_t dispatchCount[3],
			const std::vector<DescriptorSetUsage> &descriptorSetUsages,
			const PushConstants& pushConstants);
		
		/**
		 * @brief Record the start of a debug label into a command stream. 
		 * Debug labels are displayed in GPU debuggers, such as RenderDoc
		 * 
		 * @param cmdStream Handle of the command stream that the label start is recorded into
		 * @param label Label name, which is displayed in a debugger
		 * @param color Display color for the label in a debugger
		*/
		void recordBeginDebugLabel(const CommandStreamHandle &cmdStream,
								   const std::string& label,
								   const std::array<float, 4>& color);
		
		/**
		 * @brief Record the end of a debug label into a command stream
		 * @param cmdStream Handle of the command stream that the label end is recorded into
		*/
		void recordEndDebugLabel(const CommandStreamHandle &cmdStream);

		/**
		 * @brief Record an indirect compute shader dispatch into a command stream
		 *
		 * @param cmdStream Handle of the command stream that the indirect dispatch is recorded into
		 * @param computePipeline Handle of the pipeline that is used for the indirect dispatch
		 * @param buffer GPU Buffer from which the dispatch counts are read
		 * @param bufferArgOffset Offset into the GPU Buffer from where the dispatch counts are read
		 * @param descriptorSetUsages Descriptor set bindings of the indirect dispatch
		 * @param pushConstants Push constant data for the indirect dispatch
		 */
		void recordComputeIndirectDispatchToCmdStream(
			const CommandStreamHandle               cmdStream,
			const ComputePipelineHandle             computePipeline,
			const vkcv::BufferHandle                buffer,
			const size_t                            bufferArgOffset,
			const std::vector<DescriptorSetUsage>&  descriptorSetUsages,
			const PushConstants&                    pushConstants);

		/**
		 * @brief End recording and present image
		 */
		void endFrame( const WindowHandle& windowHandle );

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

		/**
		 * @brief Create a new command stream
		 * 
		 * @param queueType The type of queue to which the command stream will be submitted to
		 * @return Handle which represents the command stream
		 */
		CommandStreamHandle createCommandStream(QueueType queueType);

		/**
		 * @brief Record commands to a command stream by providing a function
		 * 
		 * @param cmdStreamHandle Handle of the command stream to record to
		 * @param record Recording function
		 * @param finish Finish function, called after execution of commands is finished
		 */
		void recordCommandsToStream(
			const CommandStreamHandle   cmdStreamHandle,
			const RecordCommandFunction &record,
			const FinishCommandFunction &finish);

		/**
		 * @brief Submit command stream to GPU for actual execution
		 * 
		 * @param handle command stream to submit
		 */
		void submitCommandStream(const CommandStreamHandle& handle);

		/**
		 * @brief Prepare swapchain image for presentation to screen.
		 * Handles internal state such as image format, also acts as a memory barrier
		 * 
		 * @param handle Handle of the command stream to record the preparation commands to
		 */
		void prepareSwapchainImageForPresent(const CommandStreamHandle& handle);

		/**
		 * @brief Prepare image for use as a sampled image.
		 * Handles internal state such as image format, also acts as a memory barrier
		 * 
		 * @param cmdStream Handle of the command stream to record the preparation commands to
		 * @param image Handle of the image to prepare
		 */
		void prepareImageForSampling(const CommandStreamHandle& cmdStream, const ImageHandle& image);

		/**
		 * @brief Prepare image for use as a storage image.
		 * Handles internal state such as image format, also acts as a memory barrier
		 *
		 * @param cmdStream Handle of the command stream to record the preparation commands to
		 * @param image Handle of the image to prepare
		 */
		void prepareImageForStorage(const CommandStreamHandle& cmdStream, const ImageHandle& image);
		
		/**
		 * @brief Manual trigger to record commands to prepare an image for use as an attachment
		 *
		 * normally layout transitions for attachments are handled by the core
		 * however for manual vulkan use, e.g. ImGui integration, this function is exposed
		 * this is also why the command buffer is passed directly, instead of the command stream handle
		 * 
		 * @param cmdBuffer The vulkan command buffer to record to
		 * @param image Handle of the image to prepare
		 */
		void prepareImageForAttachmentManually(const vk::CommandBuffer& cmdBuffer, const ImageHandle& image);

		/**
		 * @brief Indicate an external change of an image's layout
		 * 
		 * if manual vulkan work, e.g. ImGui integration, changes an image layout this function must be used
		 * to update the internal image state
		 * 
		 * @param image Handle of the image whose layout was changed
		 * @param layout The current layout of the image
		*/
		void updateImageLayoutManual(const vkcv::ImageHandle& image, const vk::ImageLayout layout);

		/**
		 * @brief Records a memory barrier to synchronize subsequent accesses to the image's data
		 * 
		 * @param cmdStream Handle of the command stream to record the barrier to
		 * @param image Handle of the image the barrier belongs to
		 */
		void recordImageMemoryBarrier(const CommandStreamHandle& cmdStream, const ImageHandle& image);

		/**
		 * @brief Records a buffer barrier to synchronize subsequent accesses to the buffer's data
		 * 
		 * @param cmdStream Handle of the command stream to record the barrier to
		 * @param buffer Handle of the buffer the barrier belongs to
		 */
		void recordBufferMemoryBarrier(const CommandStreamHandle& cmdStream, const BufferHandle& buffer);

		/**
		 * @brief Resolve a source MSAA image into a destination image for further use
		 * 
		 * @param cmdStream Handle of the command stream to record the resolve to
		 * @param src The MSAA image that is resolved
		 * @param dst The target non-MSAA image that is resolved into
		 */
		void resolveMSAAImage(const CommandStreamHandle& cmdStream, const ImageHandle& src, const ImageHandle& dst);

		/**
		 * @return Vulkan image view of the current swapchain image
		 */
		[[nodiscard]]
		vk::ImageView getSwapchainImageView() const;
	
		/**
		 * @brief Records a generic memory barrier to a command stream
		 * 
		 * @param cmdStream Handle of the command stream the barrier is recorded to
		 */
		void recordMemoryBarrier(const CommandStreamHandle& cmdStream);
		
		/**
		 * @brief Record a blit (bit block image transfer) of a source image into a destination image, 
		 * mip 0 is used for both
		 * 
		 * @param cmdStream Handle of the command stream the blit operation is recorded into
		 * @param src The source image that is read from
		 * @param dst The destination image that is written into
		 * @param filterType The type of interpolation that is used
		 */
		void recordBlitImage(const CommandStreamHandle& cmdStream, const ImageHandle& src, const ImageHandle& dst,
							 SamplerFilterType filterType);
	
		/**
		 * @brief Sets a debug label to a buffer handle.
		 *
		 * @param handle Buffer handle
		 * @param label Debug label
		 */
		void setDebugLabel(const BufferHandle &handle, const std::string &label);
		
		/**
		 * @brief Sets a debug label to a pass handle.
		 *
		 * @param handle Pass handle
		 * @param label Debug label
		 */
		void setDebugLabel(const PassHandle &handle, const std::string &label);
		
		/**
		 * @brief Sets a debug label to a graphics pipeline handle.
		 *
		 * @param handle Graphics pipeline handle
		 * @param label Debug label
		 */
		void setDebugLabel(const GraphicsPipelineHandle &handle, const std::string &label);
		
		/**
		 * @brief Sets a debug label to a compute pipeline handle.
		 *
		 * @param handle Compute pipeline handle
		 * @param label Debug label
		 */
		void setDebugLabel(const ComputePipelineHandle &handle, const std::string &label);
		
		/**
		 * @brief Sets a debug label to a descriptor set handle.
		 *
		 * @param handle Descriptor set handle
		 * @param label Debug label
		 */
		void setDebugLabel(const DescriptorSetHandle &handle, const std::string &label);
		
		/**
		 * @brief Sets a debug label to a sampler handle.
		 *
		 * @param handle Sampler handle
		 * @param label Debug label
		 */
		void setDebugLabel(const SamplerHandle &handle, const std::string &label);
		
		/**
		 * @brief Sets a debug label to an image handle.
		 *
		 * @param handle Image handle
		 * @param label Debug label
		 */
		void setDebugLabel(const ImageHandle &handle, const std::string &label);
		
		/**
		 * @brief Sets a debug label to a command stream handle.
		 *
		 * @param handle Command stream handle
		 * @param label Debug label
		 */
		void setDebugLabel(const CommandStreamHandle &handle, const std::string &label);
		
    };
}
