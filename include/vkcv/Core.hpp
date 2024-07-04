#pragma once
/**
 * @authors Alexander Gauggel, Tobias Frisch, Sebastian Gaida, Artur Wasmut, Lars Hoerttrich,
 *          Mara Vogt, Mark Mints, Simeon Hermann, Alex Laptop, Katharina Krämer, Vanessa Karolek
 * @file vkcv/Core.hpp
 * @brief Handling of global states regarding dependencies.
 */

#include <memory>
#include <string>
#include <vulkan/vulkan.hpp>

#include "BlitDownsampler.hpp"
#include "BufferTypes.hpp"
#include "ComputePipelineConfig.hpp"
#include "Container.hpp"
#include "Context.hpp"
#include "DescriptorWrites.hpp"
#include "DispatchSize.hpp"
#include "Drawcall.hpp"
#include "Event.hpp"
#include "EventFunctionTypes.hpp"
#include "GeometryData.hpp"
#include "GraphicsPipelineConfig.hpp"
#include "Handles.hpp"
#include "ImageConfig.hpp"
#include "PassConfig.hpp"
#include "PushConstants.hpp"
#include "RayTracingPipelineConfig.hpp"
#include "Result.hpp"
#include "SamplerTypes.hpp"
#include "Window.hpp"

#define VKCV_FRAMEWORK_NAME "VkCV"
#define VKCV_FRAMEWORK_VERSION (VK_MAKE_VERSION(0, 2, 0))

namespace vkcv {

	// forward declarations
	class PassManager;
	class GraphicsPipelineManager;
	class ComputePipelineManager;
	class RayTracingPipelineManager;
	class DescriptorSetLayoutManager;
	class DescriptorSetManager;
	class BufferManager;
	class SamplerManager;
	class ImageManager;
	class AccelerationStructureManager;
	class CommandStreamManager;
	class WindowManager;
	class SwapchainManager;

	/**
	 * @brief Class to handle the core functionality of the framework.
	 *
	 * The class handles the core functionality of the framework with most
	 * calls addressing resource management via more simplified abstraction.
	 */
	class Core final {
	private:
		/**
		 * Constructor of #Core requires an @p context.
		 *
		 * @param context encapsulates various Vulkan objects
		 */
		explicit Core(Context &&context) noexcept;

		// explicit destruction of default constructor
		Core() = delete;

		Result acquireSwapchainImage(const SwapchainHandle &swapchainHandle);

		Context m_Context;

		std::unique_ptr<DescriptorSetLayoutManager> m_DescriptorSetLayoutManager;
		std::unique_ptr<DescriptorSetManager> m_DescriptorSetManager;
		std::unique_ptr<BufferManager> m_BufferManager;
		std::unique_ptr<SamplerManager> m_SamplerManager;
		std::unique_ptr<ImageManager> m_ImageManager;
		std::unique_ptr<AccelerationStructureManager> m_AccelerationStructureManager;
		std::unique_ptr<CommandStreamManager> m_CommandStreamManager;
		std::unique_ptr<WindowManager> m_WindowManager;
		std::unique_ptr<SwapchainManager> m_SwapchainManager;
		std::unique_ptr<PassManager> m_PassManager;
		std::unique_ptr<GraphicsPipelineManager> m_GraphicsPipelineManager;
		std::unique_ptr<ComputePipelineManager> m_ComputePipelineManager;
		std::unique_ptr<RayTracingPipelineManager> m_RayTracingPipelineManager;
		
		Vector<vk::CommandPool> m_CommandPools;
		vk::Semaphore m_RenderFinished;
		vk::Semaphore m_SwapchainImageAcquired;
		uint32_t m_currentSwapchainImageIndex;

		std::unique_ptr<Downsampler> m_downsampler;

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
		Core(const Core &other) = delete;

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
		Core &operator=(const Core &other) = delete;

		/**
		 * Move assignment operator of #Core uses default behavior!
		 *
		 * @param other Other instance of Context
		 * @return Reference to itself
		 */
		Core &operator=(Core &&other) = delete;

		/**
		 * Returns the context of a Core instance.
		 *
		 * @return Current Context
		 */
		[[nodiscard]] const Context &getContext() const;

		/**
		 * Creates a #Core with given @p applicationName and @p applicationVersion for your
		 * application.
		 *
		 * It is also possible to require a specific amount of queues, ask for specific queue-flags
		 * or extensions. This function will take care of the required arguments as best as
		 * possible.
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
		static Core create(const std::string &applicationName, uint32_t applicationVersion,
						   const Vector<vk::QueueFlagBits> &queueFlags = {},
						   const Features &features = {},
						   const Vector<const char*> &instanceExtensions = {});

		/**
		 * Creates a basic vulkan graphics pipeline using @p config from the pipeline config class
		 * and returns it using the @p handle. Fixed Functions for pipeline are set with standard
		 * values.
		 *
		 * @param config a pipeline config object from the pipeline config class
		 * @param handle a handle to return the created vulkan handle
		 * @return True if pipeline creation was successful, False if not
		 */
		[[nodiscard]] GraphicsPipelineHandle
		createGraphicsPipeline(const GraphicsPipelineConfig &config);

		/**
		 * Creates a basic vulkan compute pipeline using @p shader program and returns it using the
		 * @p handle. Fixed Functions for pipeline are set with standard values.
		 *
		 * @param config Contains the compiles compute shader and the corresponding descriptor set
		 * layout
		 * @return True if pipeline creation was successful, False if not
		 */
		[[nodiscard]] ComputePipelineHandle
		createComputePipeline(const ComputePipelineConfig &config);
		
		/**
		 * Creates a basic vulkan ray tracing pipeline using @p shader program and returns it using
		 * the @p handle. Fixed Functions for pipeline are set with standard values.
		 *
		 * @param config a pipeline config object from the pipeline config class
		 * layout
		 * @return True if pipeline creation was successful, False if not
		 */
		[[nodiscard]] RayTracingPipelineHandle
		createRayTracingPipeline(const RayTracingPipelineConfig &config);

		/**
		 * Creates a basic vulkan render pass using @p config from the render pass config class and
		 * returns it. Fixed Functions for pipeline are set with standard values.
		 *
		 * @param[in] config a render pass config object from the render pass config class
		 * @return A handle to represent the created pass
		 */
		[[nodiscard]] PassHandle createPass(const PassConfig &config);

		/**
		 * Returns the used configuration for a created render pass which is
		 * represented by the given handle.
		 *
		 * @param[in] pass Pass handle
		 * @return Pass configuration
		 */
		[[nodiscard]] const PassConfig &getPassConfiguration(const PassHandle &pass);

		/**
		 * @brief Creates a buffer with given parameters and returns its handle.
		 *
		 * @param[in] type Type of buffer created
		 * @param[in] typeGuard Type guard for the buffer
		 * @param[in] count Count of elements of its guarded type
		 * @param[in] memoryType Type of buffers memory
		 * @param[in] readable Flag whether the buffer supports reading from it
		 * @return A handle to represent the created buffer
		 */
		BufferHandle createBuffer(BufferType type, const TypeGuard &typeGuard, size_t count,
								  BufferMemoryType memoryType = BufferMemoryType::DEVICE_LOCAL,
								  bool readable = false);

		/**
		 * @brief Creates a buffer with given parameters and returns its handle.
		 *
		 * @param[in] type Type of buffer created
		 * @param[in] size Size of the buffer
		 * @param[in] memoryType Type of buffers memory
		 * @param[in] readable Flag whether the buffer supports reading from it
		 * @return A handle to represent the created buffer
		 */
		BufferHandle createBuffer(BufferType type, size_t size,
								  BufferMemoryType memoryType = BufferMemoryType::DEVICE_LOCAL,
								  bool readable = false);

		/**
		 * @brief Returns the vulkan buffer of a given buffer handle.
		 *
		 * @param[in] buffer Buffer handle
		 * @return Vulkan buffer
		 */
		vk::Buffer getBuffer(const BufferHandle &buffer) const;

		/**
		 * @brief Returns the buffer type of a buffer represented
		 * by a given buffer handle.
		 *
		 * @param[in] buffer Buffer handle
		 * @return Buffer type
		 */
		[[nodiscard]] BufferType getBufferType(const BufferHandle &buffer) const;

		/**
		 * @brief Returns the buffer memory type of a buffer
		 * represented by a given buffer handle.
		 *
		 * @param[in] buffer Buffer handle
		 * @return Buffer memory type
		 */
		[[nodiscard]] BufferMemoryType getBufferMemoryType(const BufferHandle &buffer) const;

		/**
		 * @brief Returns the size of a buffer represented
		 * by a given buffer handle.
		 *
		 * @param[in] buffer Buffer handle
		 * @return Size of the buffer
		 */
		[[nodiscard]] size_t getBufferSize(const BufferHandle &buffer) const;
		
		/**
		 * @brief Returns the device address of a buffer represented
		 * by a given buffer handle.
		 *
		 * @param[in] buffer Buffer handle
		 * @return Device address of the buffer
		 */
		[[nodiscard]] vk::DeviceAddress getBufferDeviceAddress(const BufferHandle &buffer) const;

		/**
		 * @brief Fills a buffer represented by a given buffer
		 * handle with custom data.
		 *
		 * @param[in] buffer Buffer handle
		 * @param[in] data Pointer to data
		 * @param[in] size Size of data in bytes
		 * @param[in] offset Offset to fill in data in bytes
		 */
		void fillBuffer(const BufferHandle &buffer, const void* data, size_t size, size_t offset);

		/**
		 * @brief Reads from a buffer represented by a given
		 * buffer handle to some data pointer.
		 *
		 * @param[in] buffer Buffer handle
		 * @param[in] data Pointer to data
		 * @param[in] size Size of data to read in bytes
		 * @param[in] offset Offset to read from buffer in bytes
		 */
		void readBuffer(const BufferHandle &buffer, void* data, size_t size, size_t offset);

		/**
		 * @brief Maps memory to a buffer represented by a given
		 * buffer handle and returns it.
		 *
		 * @param[in] buffer Buffer handle
		 * @param[in] offset Offset of mapping in bytes
		 * @param[in] size Size of mapping in bytes
		 * @return Pointer to mapped memory
		 */
		void* mapBuffer(const BufferHandle &buffer, size_t offset, size_t size);

		/**
		 * @brief Unmaps memory from a buffer represented by a given
		 * buffer handle.
		 *
		 * @param[in] buffer Buffer handle
		 */
		void unmapBuffer(const BufferHandle &buffer);

		/**
		 * Creates a Sampler with given attributes.
		 *
		 * @param[in] magFilter Magnifying filter
		 * @param[in] minFilter Minimizing filter
		 * @param[in] mipmapMode Mipmapping filter
		 * @param[in] addressMode Address mode
		 * @param[in] mipLodBias Mip level of detail bias
		 * @return Sampler handle
		 */
		[[nodiscard]] SamplerHandle
		createSampler(SamplerFilterType magFilter, SamplerFilterType minFilter,
					  SamplerMipmapMode mipmapMode, SamplerAddressMode addressMode,
					  float mipLodBias = 0.0f,
					  SamplerBorderColor borderColor = SamplerBorderColor::INT_ZERO_OPAQUE);

		/**
		 * Creates an #Image with a given format, configuration
		 * and whether a mipchain should be created.
		 *
		 * @param[in] format Image format
		 * @param[in] config Image configuration
		 * @param[in] createMipChain Flag to create a mip chain
		 * @return Image handle
		 */
		[[nodiscard]] ImageHandle createImage(vk::Format format,
											  const ImageConfig& config,
											  bool createMipChain = false);

		/**
		 * @brief Fills the image with given data of a specified size
		 * in bytes.
		 *
		 * @param[in] image Image handle
		 * @param[in] data Image data pointer
		 * @param[in] size Size of data
		 * @param[in] firstLayer First image layer
		 * @param[in] layerCount Image layer count
		 */
		void fillImage(const ImageHandle &image,
					   const void* data,
					   size_t size,
					   uint32_t firstLayer,
					   uint32_t layerCount);

		/**
		 * @brief Switches the images layout synchronously if possible.
		 *
		 * @param[in] image Image handle
		 * @param[in] layout New image layout
		 */
		void switchImageLayout(const ImageHandle &image, vk::ImageLayout layout);

		/**
		 * @brief Returns the default blit-downsampler.
		 *
		 * @return Blit-downsampler
		 */
		[[nodiscard]] Downsampler &getDownsampler();

		/**
		 * Creates a new window and returns it's handle
		 * @param[in] applicationName Window title
		 * @param[in] windowWidth Window width
		 * @param[in] windowHeight Window height
		 * @param[in] resizeable resizeability bool
		 * @return windowHandle
		 */
		[[nodiscard]] WindowHandle createWindow(const std::string &applicationName,
												uint32_t windowWidth, uint32_t windowHeight,
												bool resizeable);

		/**
		 * Getter for window reference
		 * @param[in] handle of the window
		 * @return the window
		 */
		[[nodiscard]] Window &getWindow(const WindowHandle &handle);

		/**
		 * @brief Returns the image format for the current surface
		 * of the swapchain.
		 *
		 * @param[in] handle Swapchain handle
		 * @return Swapchain image format
		 */
		[[nodiscard]] vk::Format getSwapchainFormat(const SwapchainHandle &swapchain) const;

		/**
		 * @brief Returns the amount of images for the swapchain.
		 *
		 * @param[in] handle Swapchain handle
		 * @return Number of images
		 */
		[[nodiscard]] uint32_t getSwapchainImageCount(const SwapchainHandle &swapchain) const;

		/**
		 * @brief Returns the extent from the current surface of
		 * the swapchain.
		 *
		 * @param[in] handle Swapchain handle
		 * @return Extent of the swapchains surface
		 */
		[[nodiscard]] vk::Extent2D getSwapchainExtent(const SwapchainHandle &swapchain) const;

		/**
		 * @brief Returns the image width.
		 *
		 * @param[in] image Image handle
		 * @return imageWidth
		 */
		[[nodiscard]] uint32_t getImageWidth(const ImageHandle &image);

		/**
		 * @brief Returns the image height.
		 *
		 * @param[in] image Image handle
		 * @return imageHeight
		 */
		[[nodiscard]] uint32_t getImageHeight(const ImageHandle &image);

		/**
		 * @brief Returns the image depth.
		 *
		 * @param[in] image Image handle
		 * @return imageDepth
		 */
		[[nodiscard]] uint32_t getImageDepth(const ImageHandle &image);

		/**
		 * @brief Returns the image format of the image.
		 *
		 * @param[in] image Image handle
		 * @return imageFormat
		 */
		[[nodiscard]] vk::Format getImageFormat(const ImageHandle &image);

		/**
		 * @brief Returns whether the image supports storage or not.
		 *
		 * @param[in] image Image handle
		 * @return True, if the image supports storage, otherwise false.
		 */
		[[nodiscard]] bool isImageSupportingStorage(const ImageHandle &image);

		/**
		 * @brief Returns the images amount of mip levels.
		 *
		 * @param[in] image Image handle
		 * @return Amount of mip levels
		 */
		[[nodiscard]] uint32_t getImageMipLevels(const ImageHandle &image);

		/**
		 * @brief Returns the images amount of array layers.
		 *
		 * @param[in] image Image handle
		 * @return Amount of array layers
		 */
		[[nodiscard]] uint32_t getImageArrayLayers(const ImageHandle &image);

		/**
		 * @brief Creates a descriptor set layout handle by a set of descriptor bindings.
		 *
		 * @param[in] bindings Descriptor bindings
		 * @return Descriptor set layout handle
		 */
		[[nodiscard]] DescriptorSetLayoutHandle
		createDescriptorSetLayout(const DescriptorBindings &bindings);

		/**
		 * @brief Creates a new descriptor set
		 *
		 * @param[in] layout Handle to the layout that the descriptor set will use
		 * @return Handle that represents the descriptor set
		 */
		[[nodiscard]] DescriptorSetHandle
		createDescriptorSet(const DescriptorSetLayoutHandle &layout);

		/**
		 * @brief Writes resources bindings to a descriptor set
		 *
		 * @param handle Handle of the descriptor set
		 * @param writes Struct containing the resource bindings to be written
		 * must be compatible with the descriptor set's layout
		 */
		void writeDescriptorSet(DescriptorSetHandle handle, const DescriptorWrites &writes);

		/**
		 * @brief Start recording command buffers and increment frame index
		 */
		bool beginFrame(uint32_t &width, uint32_t &height, const WindowHandle &windowHandle);

		/**
		 * @brief Records drawcalls to a command stream
		 *
		 * @param cmdStreamHandle Handle of the command stream that the drawcalls are recorded into
		 * @param pipelineHandle Handle of the pipeline that is used for the drawcalls
		 * @param pushConstants Push constants that are used for the drawcalls, ignored if constant
		 * size is set to 0
		 * @param drawcalls Information about each drawcall, consisting of mesh handle, descriptor
		 * set bindings and instance count
		 * @param renderTargets Image handles that are used as render targets
		 * @param windowHandle Window handle that is used to retrieve the corresponding swapchain
		 */
		void recordDrawcallsToCmdStream(const CommandStreamHandle &cmdStreamHandle,
										const GraphicsPipelineHandle &pipelineHandle,
										const PushConstants &pushConstants,
										const Vector<InstanceDrawcall> &drawcalls,
										const Vector<ImageHandle> &renderTargets,
										const WindowHandle &windowHandle);

		/**
		 * @brief Records indirect drawcalls to a command stream
		 *
		 * @param cmdStreamHandle Handle of the command stream that the drawcalls are recorded into
		 * @param pipelineHandle Handle of the pipeline that is used for the drawcalls
		 * @param pushConstantData Push constants that are used for the drawcalls, ignored if
		 * constant size is set to 0
		 * @param drawcalls Information about each drawcall, consisting of mesh handle, descriptor
		 * set bindings and draw count
		 * @param renderTargets Image handles that are used as render targets
		 * @param windowHandle Window handle that is used to retrieve the corresponding swapchain
		 */
		void recordIndirectDrawcallsToCmdStream(const CommandStreamHandle cmdStreamHandle,
												const GraphicsPipelineHandle &pipelineHandle,
												const PushConstants &pushConstantData,
												const Vector<IndirectDrawcall> &drawcalls,
												const Vector<ImageHandle> &renderTargets,
												const WindowHandle &windowHandle);

		/**
		 * @brief Records mesh shader drawcalls to a command stream
		 *
		 * @param cmdStreamHandle Handle of the command stream that the drawcalls are recorded into
		 * @param pipelineHandle Handle of the pipeline that is used for the drawcalls
		 * @param pushConstantData Push constants that are used for the drawcalls, ignored if
		 * constant size is set to 0
		 * @param drawcalls Information about each drawcall, consisting of descriptor set bindings
		 * and task shader task count
		 * @param renderTargets Image handles that are used as render targets
		 * @param windowHandle Window handle that is used to retrieve the corresponding swapchain
		 */
		void recordMeshShaderDrawcalls(const CommandStreamHandle &cmdStreamHandle,
									   const GraphicsPipelineHandle &pipelineHandle,
									   const PushConstants &pushConstantData,
									   const Vector<TaskDrawcall> &drawcalls,
									   const Vector<ImageHandle> &renderTargets,
									   const WindowHandle &windowHandle);

		/**
		 * Records the ray generation via ray tracing pipeline to the @p cmdStreamHandle.
		 *
		 * @param cmdStreamHandle The command stream handle which receives relevant commands for
		 * drawing.
		 * @param rayTracingPipeline The raytracing pipeline
		 * @param dispatchSize How many work groups are dispatched
		 * @param descriptorSetUsages The descriptor set usages.
		 * @param pushConstants The push constants.
		 * @param windowHandle The window handle defining in which window to render.
		 */
		void recordRayGenerationToCmdStream(const CommandStreamHandle &cmdStreamHandle,
											const RayTracingPipelineHandle &rayTracingPipeline,
											const DispatchSize &dispatchSize,
											const Vector<DescriptorSetUsage>
											        &descriptorSetUsages,
											const PushConstants &pushConstants,
											const vkcv::WindowHandle &windowHandle);

		/**
		 * @brief Record a compute shader dispatch into a command stream
		 *
		 * @param cmdStream Handle of the command stream that the dispatch is recorded into
		 * @param computePipeline Handle of the pipeline that is used for the dispatch
		 * @param dispatchSize How many work groups are dispatched
		 * @param descriptorSetUsages Descriptor set usages of the dispatch
		 * @param pushConstants Push constant data for the dispatch
		 */
		void
		recordComputeDispatchToCmdStream(const CommandStreamHandle &cmdStream,
										 const ComputePipelineHandle &computePipeline,
										 const DispatchSize &dispatchSize,
										 const Vector<DescriptorSetUsage> &descriptorSetUsages,
										 const PushConstants &pushConstants);

		/**
		 * @brief Record the start of a debug label into a command stream.
		 * Debug labels are displayed in GPU debuggers, such as RenderDoc
		 *
		 * @param cmdStream Handle of the command stream that the label start is recorded into
		 * @param label Label name, which is displayed in a debugger
		 * @param color Display color for the label in a debugger
		 */
		void recordBeginDebugLabel(const CommandStreamHandle &cmdStream, const std::string &label,
								   const std::array<float, 4> &color);

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
			const CommandStreamHandle cmdStream, const ComputePipelineHandle computePipeline,
			const vkcv::BufferHandle buffer, const size_t bufferArgOffset,
			const Vector<DescriptorSetUsage> &descriptorSetUsages,
			const PushConstants &pushConstants);

		/**
		 * @brief End recording and present image
		 */
		void endFrame(const WindowHandle &windowHandle);

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
		void recordCommandsToStream(const CommandStreamHandle &stream,
									const RecordCommandFunction &record,
									const FinishCommandFunction &finish);

		/**
		 * @brief Submit command stream to GPU for actual execution
		 *
		 * @param[in] handle Command stream to submit
		 * @param[in] signalRendering Flag to specify if the command stream finishes rendering
		 */
		void submitCommandStream(const CommandStreamHandle &stream, bool signalRendering = true);

		/**
		 * @brief Prepare swapchain image for presentation to screen.
		 * Handles internal state such as image format, also acts as a memory barrier
		 *
		 * @param handle Handle of the command stream to record the preparation commands to
		 */
		void prepareSwapchainImageForPresent(const CommandStreamHandle &handle);

		/**
		 * @brief Prepare image for use as a sampled image.
		 * Handles internal state such as image format, also acts as a memory barrier
		 *
		 * @param[in] cmdStream Handle of the command stream to record the preparation commands to
		 * @param[in] image Handle of the image to prepare
		 * @param[in] mipLevelCount Count of mip levels to prepare
		 * @param[in] mipLevelOffset Offset to start preparing mip levels
		 */
		void prepareImageForSampling(const CommandStreamHandle &cmdStream, const ImageHandle &image,
									 uint32_t mipLevelCount = 0, uint32_t mipLevelOffset = 0);

		/**
		 * @brief Prepare image for use as a storage image.
		 * Handles internal state such as image format, also acts as a memory barrier
		 *
		 * @param[in] cmdStream Handle of the command stream to record the preparation commands to
		 * @param[in] image Handle of the image to prepare
		 * @param[in] mipLevelCount Count of mip levels to prepare
		 * @param[in] mipLevelOffset Offset to start preparing mip levels
		 */
		void prepareImageForStorage(const CommandStreamHandle &cmdStream, const ImageHandle &image,
									uint32_t mipLevelCount = 0, uint32_t mipLevelOffset = 0);

		/**
		 * @brief Manual trigger to record commands to prepare an image for use as an attachment
		 *
		 * normally layout transitions for attachments are handled by the core
		 * however for manual vulkan use, e.g. ImGui integration, this function is exposed
		 * this is also why the command buffer is passed directly, instead of the command stream
		 * handle
		 *
		 * @param cmdBuffer The vulkan command buffer to record to
		 * @param image Handle of the image to prepare
		 */
		void prepareImageForAttachmentManually(const vk::CommandBuffer &cmdBuffer,
											   const ImageHandle &image);

		/**
		 * @brief Indicate an external change of an image's layout
		 *
		 * if manual vulkan work, e.g. ImGui integration, changes an image layout this function must
		 * be used to update the internal image state
		 *
		 * @param image Handle of the image whose layout was changed
		 * @param layout The current layout of the image
		 */
		void updateImageLayoutManual(const vkcv::ImageHandle &image, const vk::ImageLayout layout);

		/**
		 * @brief Records a memory barrier to synchronize subsequent accesses to the image's data
		 *
		 * @param cmdStream Handle of the command stream to record the barrier to
		 * @param image Handle of the image the barrier belongs to
		 */
		void recordImageMemoryBarrier(const CommandStreamHandle &cmdStream,
									  const ImageHandle &image);

		/**
		 * @brief Records a buffer barrier to synchronize subsequent accesses to the buffer's data
		 *
		 * @param cmdStream Handle of the command stream to record the barrier to
		 * @param buffer Handle of the buffer the barrier belongs to
		 */
		void recordBufferMemoryBarrier(const CommandStreamHandle &cmdStream,
									   const BufferHandle &buffer);

		/**
		 * @brief Resolve a source MSAA image into a destination image for further use
		 *
		 * @param cmdStream Handle of the command stream to record the resolve to
		 * @param src The MSAA image that is resolved
		 * @param dst The target non-MSAA image that is resolved into
		 */
		void resolveMSAAImage(const CommandStreamHandle &cmdStream, const ImageHandle &src,
							  const ImageHandle &dst);

		/**
		 * @return Vulkan image view of the current swapchain image
		 */
		[[nodiscard]] vk::ImageView getSwapchainImageView() const;

		/**
		 * @brief Records a generic memory barrier to a command stream
		 *
		 * @param cmdStream Handle of the command stream the barrier is recorded to
		 */
		void recordMemoryBarrier(const CommandStreamHandle &cmdStream);

		/**
		 * @brief Record a blit (bit block image transfer) of a source image into a destination
		 * image, mip 0 is used for both
		 *
		 * @param cmdStream Handle of the command stream the blit operation is recorded into
		 * @param src The source image that is read from
		 * @param dst The destination image that is written into
		 * @param filterType The type of interpolation that is used
		 */
		void recordBlitImage(const CommandStreamHandle &cmdStream, const ImageHandle &src,
							 const ImageHandle &dst, SamplerFilterType filterType);

		/**
		 * @brief Sets a debug label to a buffer handle.
		 *
		 * @param[in,out] handle Buffer handle
		 * @param[in] label Debug label
		 */
		void setDebugLabel(const BufferHandle &handle, const std::string &label);

		/**
		 * @brief Sets a debug label to a pass handle.
		 *
		 * @param[in,out] handle Pass handle
		 * @param[in] label Debug label
		 */
		void setDebugLabel(const PassHandle &handle, const std::string &label);

		/**
		 * @brief Sets a debug label to a graphics pipeline handle.
		 *
		 * @param[in,out] handle Graphics pipeline handle
		 * @param[in] label Debug label
		 */
		void setDebugLabel(const GraphicsPipelineHandle &handle, const std::string &label);

		/**
		 * @brief Sets a debug label to a compute pipeline handle.
		 *
		 * @param[in,out] handle Compute pipeline handle
		 * @param[in] label Debug label
		 */
		void setDebugLabel(const ComputePipelineHandle &handle, const std::string &label);

		/**
		 * @brief Sets a debug label to a descriptor set handle.
		 *
		 * @param[in,out] handle Descriptor set handle
		 * @param[in] label Debug label
		 */
		void setDebugLabel(const DescriptorSetHandle &handle, const std::string &label);

		/**
		 * @brief Sets a debug label to a sampler handle.
		 *
		 * @param[in,out] handle Sampler handle
		 * @param[in] label Debug label
		 */
		void setDebugLabel(const SamplerHandle &handle, const std::string &label);

		/**
		 * @brief Sets a debug label to an image handle.
		 *
		 * @param[in,out] handle Image handle
		 * @param[in] label Debug label
		 */
		void setDebugLabel(const ImageHandle &handle, const std::string &label);

		/**
		 * @brief Sets a debug label to a command stream handle.
		 *
		 * @param[in,out] handle Command stream handle
		 * @param[in] label Debug label
		 */
		void setDebugLabel(const CommandStreamHandle &handle, const std::string &label);

		/**
		 * @brief Runs the application in the current until all windows get closed.
		 *
		 * The frame callback will be called for each window every single frame.
		 *
		 * @param[in] frame Frame callback
		 */
		void run(const WindowFrameFunction &frame);

		/**
		 * @brief Return the underlying vulkan handle for a render pass
		 * by its given pass handle.
		 *
		 * @param[in] handle Pass handle
		 * @return Vulkan render pass
		 */
		[[nodiscard]] vk::RenderPass getVulkanRenderPass(const PassHandle &handle) const;

		/**
		 * @brief Return the underlying vulkan handle for a pipeline
		 * by its given graphics pipeline handle.
		 *
		 * @param[in] handle Graphics pipeline handle
		 * @return Vulkan pipeline
		 */
		[[nodiscard]] vk::Pipeline getVulkanPipeline(const GraphicsPipelineHandle &handle) const;

		/**
		 * @brief Return the underlying vulkan handle for a pipeline
		 * by its given compute pipeline handle.
		 *
		 * @param[in] handle Compute pipeline handle
		 * @return Vulkan pipeline
		 */
		[[nodiscard]] vk::Pipeline getVulkanPipeline(const ComputePipelineHandle &handle) const;

		/**
		 * @brief Return the underlying vulkan handle for a descriptor set layout
		 * by its given descriptor set layout handle.
		 *
		 * @param[in] handle Descriptor set layout handle
		 * @return Vulkan descriptor set layout
		 */
		[[nodiscard]] vk::DescriptorSetLayout
		getVulkanDescriptorSetLayout(const DescriptorSetLayoutHandle &handle) const;

		/**
		 * @brief Return the underlying vulkan handle for a descriptor set
		 * by its given descriptor set handle.
		 *
		 * @param[in] handle Descriptor set handle
		 * @return Vulkan descriptor set
		 */
		[[nodiscard]] vk::DescriptorSet
		getVulkanDescriptorSet(const DescriptorSetHandle &handle) const;

		/**
		 * @brief Return the underlying vulkan handle for a buffer
		 * by its given buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 * @return Vulkan buffer
		 */
		[[nodiscard]] vk::Buffer getVulkanBuffer(const BufferHandle &handle) const;

		/**
		 * @brief Return the underlying vulkan handle for a sampler
		 * by its given sampler handle.
		 *
		 * @param[in] handle Sampler handle
		 * @return Vulkan sampler
		 */
		[[nodiscard]] vk::Sampler getVulkanSampler(const SamplerHandle &handle) const;

		/**
		 * @brief Return the underlying vulkan handle for a image
		 * by its given image handle.
		 *
		 * @param[in] handle Image handle
		 * @return Vulkan image
		 */
		[[nodiscard]] vk::Image getVulkanImage(const ImageHandle &handle) const;

		/**
		 * @brief Return the underlying vulkan handle for a image view
		 * by its given image handle.
		 *
		 * @param[in] handle Image handle
		 * @return Vulkan image view
		 */
		[[nodiscard]] vk::ImageView getVulkanImageView(const ImageHandle &handle) const;

		/**
		 * @brief Return the underlying vulkan handle for a device memory
		 * by its given buffer handle.
		 *
		 * @param[in] handle Buffer handle
		 * @return Vulkan device memory
		 */
		[[nodiscard]] vk::DeviceMemory getVulkanDeviceMemory(const BufferHandle &handle) const;

		/**
		 * @brief Return the underlying vulkan handle for a device memory
		 * by its given image handle.
		 *
		 * @param[in] handle Image handle
		 * @return Vulkan device memory
		 */
		[[nodiscard]] vk::DeviceMemory getVulkanDeviceMemory(const ImageHandle &handle) const;
		
		/**
		 * @brief Creates an acceleration structure handle built with a given list of geometry data.
		 *
		 * @param[in] geometryData List of geometry data
		 * @return Acceleration structure handle
		 */
		AccelerationStructureHandle createAccelerationStructure(
				const Vector<GeometryData> &geometryData,
				const BufferHandle &transformBuffer = {},
				bool compaction = false);
		
		/**
		 * @brief Creates an acceleration structure handle built with a given list of
		 * other bottom-level acceleration structures.
		 *
		 * @param[in] handles List of acceleration structure handles
		 * @return Acceleration structure handle
		 */
		AccelerationStructureHandle createAccelerationStructure(
				const Vector<AccelerationStructureHandle> &handles);
		
		/**
		 * @brief the underlying vulkan handle for an acceleration structure
		 * by its given acceleration structure handle.
		 *
		 * @param[in] handle Acceleration structure handle
		 * @return Vulkan acceleration structure
		 */
		[[nodiscard]] vk::AccelerationStructureKHR getVulkanAccelerationStructure(
				const AccelerationStructureHandle &handle) const;
		
		/**
		 * @brief Return the underlying vulkan handle for an acceleration
		 * structure by its given acceleration structure handle.
		 *
		 * @param[in] handle Acceleration structure handle
		 * @return Vulkan buffer
		 */
		[[nodiscard]] vk::Buffer getVulkanBuffer(
				const vkcv::AccelerationStructureHandle &handle) const;
		
		/**
		 * @brief Returns the device address of an acceleration structure represented
		 * by a given acceleration structure handle.
		 *
		 * @param[in] handle Acceleration structure handle
		 * @return Device address of the acceleration structure
		 */
		[[nodiscard]] vk::DeviceAddress getAccelerationStructureDeviceAddress(
				const vkcv::AccelerationStructureHandle &handle) const;
		
	};
} // namespace vkcv
