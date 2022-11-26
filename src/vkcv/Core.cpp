/**
 * @authors Artur Wasmut, Alexander Gauggel, Tobias Frisch
 * @file src/vkcv/Core.cpp
 * @brief Handling of global states regarding dependencies
 */

#include <GLFW/glfw3.h>
#include <cmath>

#include "AccelerationStructureManager.hpp"
#include "BufferManager.hpp"
#include "CommandStreamManager.hpp"
#include "ComputePipelineManager.hpp"
#include "DescriptorSetLayoutManager.hpp"
#include "DescriptorSetManager.hpp"
#include "GraphicsPipelineManager.hpp"
#include "ImageManager.hpp"
#include "PassManager.hpp"
#include "RayTracingPipelineManager.hpp"
#include "SamplerManager.hpp"
#include "WindowManager.hpp"
#include "vkcv/BlitDownsampler.hpp"
#include "vkcv/Core.hpp"
#include "vkcv/Image.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv {

	/**
	 * @brief Generates a set of the family indices for all different kinds of
	 * queues a given queue manager provides.
	 *
	 * @param[in] queueManager Queue manager
	 * @return Set of queue family indices
	 */
	static std::unordered_set<int> generateQueueFamilyIndexSet(const QueueManager &queueManager) {
		std::unordered_set<int> indexSet;

		for (const auto &queue : queueManager.getGraphicsQueues()) {
			indexSet.insert(queue.familyIndex);
		}

		for (const auto &queue : queueManager.getComputeQueues()) {
			indexSet.insert(queue.familyIndex);
		}

		for (const auto &queue : queueManager.getTransferQueues()) {
			indexSet.insert(queue.familyIndex);
		}

		indexSet.insert(queueManager.getPresentQueue().familyIndex);
		return indexSet;
	}

	/**
	 * @brief Creates and returns a vector of newly allocated command pools
	 * for each different queue family index in a given set.
	 *
	 * @param[in,out] device Vulkan device
	 * @param[in] familyIndexSet Set of queue family indices
	 * @return New command pools
	 */
	static std::vector<vk::CommandPool>
	createCommandPools(const vk::Device &device, const std::unordered_set<int> &familyIndexSet) {
		std::vector<vk::CommandPool> commandPoolsPerQueueFamily;
		commandPoolsPerQueueFamily.resize(familyIndexSet.size());

		const vk::CommandPoolCreateFlags poolFlags = vk::CommandPoolCreateFlagBits::eTransient;
		for (const int familyIndex : familyIndexSet) {
			const vk::CommandPoolCreateInfo poolCreateInfo(poolFlags, familyIndex);
			commandPoolsPerQueueFamily [familyIndex] =
				device.createCommandPool(poolCreateInfo, nullptr, {});
		}

		return commandPoolsPerQueueFamily;
	}

	Core Core::create(const std::string &applicationName, uint32_t applicationVersion,
					  const std::vector<vk::QueueFlagBits> &queueFlags, const Features &features,
					  const std::vector<const char*> &instanceExtensions) {
		Context context = Context::create(applicationName, applicationVersion, queueFlags, features,
										  instanceExtensions);

		return Core(std::move(context));
	}

	const Context &Core::getContext() const {
		return m_Context;
	}

	Core::Core(Context &&context) noexcept :
		m_Context(std::move(context)), m_PassManager(std::make_unique<PassManager>()),
		m_GraphicsPipelineManager(std::make_unique<GraphicsPipelineManager>()),
		m_ComputePipelineManager(std::make_unique<ComputePipelineManager>()),
		m_RayTracingPipelineManager(std::make_unique<RayTracingPipelineManager>()),
		m_DescriptorSetLayoutManager(std::make_unique<DescriptorSetLayoutManager>()),
		m_DescriptorSetManager(std::make_unique<DescriptorSetManager>()),
		m_BufferManager(std::make_unique<BufferManager>()),
		m_SamplerManager(std::make_unique<SamplerManager>()),
		m_ImageManager(std::make_unique<ImageManager>()),
		m_AccelerationStructureManager(std::make_unique<AccelerationStructureManager>()),
		m_CommandStreamManager { std::make_unique<CommandStreamManager>() },
		m_WindowManager(std::make_unique<WindowManager>()),
		m_SwapchainManager(std::make_unique<SwapchainManager>()), m_CommandPools(),
		m_RenderFinished(), m_SwapchainImageAcquired(), m_downsampler(nullptr) {
		m_CommandPools = createCommandPools(
			m_Context.getDevice(), generateQueueFamilyIndexSet(m_Context.getQueueManager()));

		m_RenderFinished = m_Context.getDevice().createSemaphore({});
		m_SwapchainImageAcquired = m_Context.getDevice().createSemaphore({});

		m_PassManager->init(*this);
		m_GraphicsPipelineManager->init(*this);
		m_ComputePipelineManager->init(*this);
		m_RayTracingPipelineManager->init(*this);
		m_DescriptorSetLayoutManager->init(*this);
		m_DescriptorSetManager->init(*this, *m_DescriptorSetLayoutManager);
		m_BufferManager->init(*this);
		m_SamplerManager->init(*this);
		m_ImageManager->init(*this, *m_BufferManager);
		m_AccelerationStructureManager->init(*this, *m_BufferManager);
		m_CommandStreamManager->init(*this);
		m_SwapchainManager->init(*this);
		m_downsampler = std::unique_ptr<Downsampler>(new BlitDownsampler(*this, *m_ImageManager));
	}

	Core::~Core() noexcept {
		m_Context.getDevice().waitIdle();

		for (const vk::CommandPool &pool : m_CommandPools) {
			m_Context.getDevice().destroyCommandPool(pool);
		}

		m_Context.getDevice().destroySemaphore(m_RenderFinished);
		m_Context.getDevice().destroySemaphore(m_SwapchainImageAcquired);
	}

	GraphicsPipelineHandle Core::createGraphicsPipeline(const GraphicsPipelineConfig &config) {
		return m_GraphicsPipelineManager->createPipeline(config, *m_PassManager,
														 *m_DescriptorSetLayoutManager);
	}

	ComputePipelineHandle Core::createComputePipeline(const ComputePipelineConfig &config) {
		std::vector<vk::DescriptorSetLayout> layouts;
		layouts.resize(config.getDescriptorSetLayouts().size());

		for (size_t i = 0; i < layouts.size(); i++) {
			layouts [i] = m_DescriptorSetLayoutManager
							  ->getDescriptorSetLayout(config.getDescriptorSetLayouts() [i])
							  .vulkanHandle;
		}

		return m_ComputePipelineManager->createComputePipeline(config.getShaderProgram(), layouts);
	}
	
	RayTracingPipelineHandle Core::createRayTracingPipeline(
			const vkcv::RayTracingPipelineConfig &config) {
		return m_RayTracingPipelineManager->createPipeline(config,
														   *m_DescriptorSetLayoutManager,
														   *m_BufferManager);
	}

	PassHandle Core::createPass(const PassConfig &config) {
		return m_PassManager->createPass(config);
	}

	const PassConfig &Core::getPassConfiguration(const vkcv::PassHandle &pass) {
		return m_PassManager->getPassConfig(pass);
	}

	BufferHandle Core::createBuffer(BufferType type, const TypeGuard &typeGuard, size_t count,
									BufferMemoryType memoryType, bool readable) {
		return m_BufferManager->createBuffer(typeGuard, type, memoryType,
											 count * typeGuard.typeSize(), readable);
	}

	BufferHandle Core::createBuffer(BufferType type, size_t size, BufferMemoryType memoryType,
									bool readable) {
		return m_BufferManager->createBuffer(TypeGuard(1), type, memoryType, size, readable);
	}

	vk::Buffer Core::getBuffer(const BufferHandle &buffer) const {
		return m_BufferManager->getBuffer(buffer);
	}

	BufferType Core::getBufferType(const BufferHandle &handle) const {
		return m_BufferManager->getBufferType(handle);
	}

	BufferMemoryType Core::getBufferMemoryType(const BufferHandle &handle) const {
		return m_BufferManager->getBufferMemoryType(handle);
	}

	size_t Core::getBufferSize(const BufferHandle &handle) const {
		return m_BufferManager->getBufferSize(handle);
	}
	
	vk::DeviceAddress Core::getBufferDeviceAddress(const vkcv::BufferHandle &buffer) const {
		return m_BufferManager->getBufferDeviceAddress(buffer);
	}

	void Core::fillBuffer(const BufferHandle &handle, const void* data, size_t size,
						  size_t offset) {
		m_BufferManager->fillBuffer(handle, data, size, offset);
	}

	void Core::readBuffer(const BufferHandle &handle, void* data, size_t size, size_t offset) {
		m_BufferManager->readBuffer(handle, data, size, offset);
	}

	void* Core::mapBuffer(const BufferHandle &handle, size_t offset, size_t size) {
		return m_BufferManager->mapBuffer(handle, offset, size);
	}

	void Core::unmapBuffer(const BufferHandle &handle) {
		m_BufferManager->unmapBuffer(handle);
	}

	Result Core::acquireSwapchainImage(const SwapchainHandle &swapchainHandle) {
		uint32_t imageIndex;
		vk::Result result;

		try {
			result = m_Context.getDevice().acquireNextImageKHR(
				m_SwapchainManager->getSwapchain(swapchainHandle).m_Swapchain,
				std::numeric_limits<uint64_t>::max(), m_SwapchainImageAcquired, nullptr,
				&imageIndex, {});
		} catch (const vk::OutOfDateKHRError &e) {
			result = vk::Result::eErrorOutOfDateKHR;
		} catch (const vk::DeviceLostError &e) {
			result = vk::Result::eErrorDeviceLost;
		}

		if ((result != vk::Result::eSuccess) && (result != vk::Result::eSuboptimalKHR)) {
			vkcv_log(LogLevel::ERROR, "%s", vk::to_string(result).c_str());
			return Result::ERROR;
		} else if (result == vk::Result::eSuboptimalKHR) {
			vkcv_log(LogLevel::WARNING, "Acquired image is suboptimal");
			m_SwapchainManager->signalRecreation(swapchainHandle);
		}

		m_currentSwapchainImageIndex = imageIndex;
		return Result::SUCCESS;
	}

	bool Core::beginFrame(uint32_t &width, uint32_t &height, const WindowHandle &windowHandle) {
		const Window &window = m_WindowManager->getWindow(windowHandle);
		const SwapchainHandle swapchainHandle = window.getSwapchain();

		if (m_SwapchainManager->shouldUpdateSwapchain(swapchainHandle)) {
			m_Context.getDevice().waitIdle();

			m_SwapchainManager->updateSwapchain(swapchainHandle, window);

			if (!m_SwapchainManager->getSwapchain(swapchainHandle).m_Swapchain) {
				return false;
			}

			setSwapchainImages(swapchainHandle);
		}

		const auto &extent = m_SwapchainManager->getExtent(swapchainHandle);

		width = extent.width;
		height = extent.height;

		if ((width < MIN_SURFACE_SIZE) || (height < MIN_SURFACE_SIZE)) {
			return false;
		}

		if (acquireSwapchainImage(swapchainHandle) != Result::SUCCESS) {
			vkcv_log(LogLevel::ERROR, "Acquire failed");

			m_currentSwapchainImageIndex = std::numeric_limits<uint32_t>::max();
		}

		m_Context.getDevice().waitIdle(); // TODO: this is a sin against graphics programming, but
										  // its getting late - Alex

		m_ImageManager->setCurrentSwapchainImageIndex(m_currentSwapchainImageIndex);

		return (m_currentSwapchainImageIndex != std::numeric_limits<uint32_t>::max());
	}

	static std::array<uint32_t, 2>
	getWidthHeightFromRenderTargets(const std::vector<ImageHandle> &renderTargets,
									const vk::Extent2D &swapchainExtent,
									const ImageManager &imageManager) {

		std::array<uint32_t, 2> widthHeight;

		if (renderTargets.size() > 0) {
			const vkcv::ImageHandle firstImage = renderTargets [0];
			if (firstImage.isSwapchainImage()) {
				widthHeight [0] = swapchainExtent.width;
				widthHeight [1] = swapchainExtent.height;
			} else {
				widthHeight [0] = imageManager.getImageWidth(firstImage);
				widthHeight [1] = imageManager.getImageHeight(firstImage);
			}
		} else {
			widthHeight [0] = 1;
			widthHeight [1] = 1;
		}
		// TODO: validate that width/height match for all attachments
		return widthHeight;
	}

	static vk::Framebuffer createFramebuffer(const std::vector<ImageHandle> &renderTargets,
											 const ImageManager &imageManager,
											 const vk::Extent2D &renderExtent,
											 const vk::RenderPass &renderpass,
											 const vk::Device &device) {

		std::vector<vk::ImageView> attachmentsViews;
		for (const ImageHandle &handle : renderTargets) {
			attachmentsViews.push_back(imageManager.getVulkanImageView(handle));
		}

		const vk::FramebufferCreateInfo createInfo(
			{}, renderpass, static_cast<uint32_t>(attachmentsViews.size()), attachmentsViews.data(),
			renderExtent.width, renderExtent.height, 1);

		return device.createFramebuffer(createInfo);
	}

	void transitionRendertargetsToAttachmentLayout(const std::vector<ImageHandle> &renderTargets,
												   ImageManager &imageManager,
												   const vk::CommandBuffer cmdBuffer) {

		for (const ImageHandle &handle : renderTargets) {
			const bool isDepthImage = isDepthFormat(imageManager.getImageFormat(handle));
			const vk::ImageLayout targetLayout =
				isDepthImage ? vk::ImageLayout::eDepthStencilAttachmentOptimal :
							   vk::ImageLayout::eColorAttachmentOptimal;
			imageManager.recordImageLayoutTransition(handle, 0, 0, targetLayout, cmdBuffer);
		}
	}

	std::vector<vk::ClearValue>
	createAttachmentClearValues(const std::vector<AttachmentDescription> &attachments) {
		std::vector<vk::ClearValue> clearValues;
		for (const auto &attachment : attachments) {
			if (attachment.getLoadOperation() == AttachmentOperation::CLEAR) {
				clearValues.push_back(attachment.getClearValue());
			}
		}
		return clearValues;
	}

	void recordDynamicViewport(vk::CommandBuffer cmdBuffer, uint32_t width, uint32_t height) {
		vk::Viewport dynamicViewport(0.0f, 0.0f, static_cast<float>(width),
									 static_cast<float>(height), 0.0f, 1.0f);

		vk::Rect2D dynamicScissor({ 0, 0 }, { width, height });

		cmdBuffer.setViewport(0, 1, &dynamicViewport);
		cmdBuffer.setScissor(0, 1, &dynamicScissor);
	}

	vk::IndexType getIndexType(IndexBitCount indexByteCount) {
		switch (indexByteCount) {
		case IndexBitCount::Bit8:
			return vk::IndexType::eUint8EXT;
		case IndexBitCount::Bit16:
			return vk::IndexType::eUint16;
		case IndexBitCount::Bit32:
			return vk::IndexType::eUint32;
		default:
			vkcv_log(LogLevel::ERROR, "unknown Enum");
			return vk::IndexType::eNoneKHR;
		}
	}

	static void recordDrawcall(const DescriptorSetManager &descriptorSetManager,
							   const BufferManager &bufferManager, const InstanceDrawcall &drawcall,
							   vk::CommandBuffer cmdBuffer, vk::PipelineLayout pipelineLayout,
							   const PushConstants &pushConstants, size_t drawcallIndex) {

		const auto &vertexData = drawcall.getVertexData();

		for (uint32_t i = 0; i < vertexData.getVertexBufferBindings().size(); i++) {
			const auto &vertexBinding = vertexData.getVertexBufferBindings() [i];

			cmdBuffer.bindVertexBuffers(i, bufferManager.getBuffer(vertexBinding.buffer),
										vertexBinding.offset);
		}

		for (const auto &usage : drawcall.getDescriptorSetUsages()) {
			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, pipelineLayout, usage.location,
				descriptorSetManager.getDescriptorSet(usage.descriptorSet).vulkanHandle,
				usage.dynamicOffsets);
		}

		if (pushConstants.getSizePerDrawcall() > 0) {
			cmdBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eAll, 0,
									pushConstants.getSizePerDrawcall(),
									pushConstants.getDrawcallData(drawcallIndex));
		}

		if (vertexData.getIndexBuffer()) {
			cmdBuffer.bindIndexBuffer(bufferManager.getBuffer(vertexData.getIndexBuffer()), 0,
									  getIndexType(vertexData.getIndexBitCount()));

			cmdBuffer.drawIndexed(vertexData.getCount(), drawcall.getInstanceCount(), 0, 0, {});
		} else {
			cmdBuffer.draw(vertexData.getCount(), drawcall.getInstanceCount(), 0, 0, {});
		}
	}

	static void recordGraphicsPipeline(Core &core, CommandStreamManager &cmdStreamManager,
									   GraphicsPipelineManager &pipelineManager,
									   PassManager &passManager, ImageManager &imageManager,
									   const CommandStreamHandle &cmdStreamHandle,
									   const GraphicsPipelineHandle &pipelineHandle,
									   const PushConstants &pushConstants,
									   const std::vector<ImageHandle> &renderTargets,
									   const WindowHandle &windowHandle,
									   const RecordCommandFunction &record) {

		const SwapchainHandle swapchainHandle = core.getWindow(windowHandle).getSwapchain();

		const std::array<uint32_t, 2> extent = getWidthHeightFromRenderTargets(
			renderTargets, core.getSwapchainExtent(swapchainHandle), imageManager);

		const auto width = extent [0];
		const auto height = extent [1];

		const PassHandle &passHandle = pipelineManager.getPipelineConfig(pipelineHandle).getPass();

		const vk::RenderPass renderPass = passManager.getVkPass(passHandle);
		const PassConfig passConfig = passManager.getPassConfig(passHandle);

		const auto &attachments = passConfig.getAttachments();
		const auto &layouts = passManager.getLayouts(passHandle);

		if (renderTargets.size() != layouts.size()) {
			vkcv_log(LogLevel::ERROR, "Amount of render targets does not match specified pipeline");
			return;
		}

		const vk::Pipeline pipeline = pipelineManager.getVkPipeline(pipelineHandle);
		const vk::Rect2D renderArea(vk::Offset2D(0, 0), vk::Extent2D(width, height));

		vk::CommandBuffer cmdBuffer = cmdStreamManager.getStreamCommandBuffer(cmdStreamHandle);
		transitionRendertargetsToAttachmentLayout(renderTargets, imageManager, cmdBuffer);

		for (size_t i = 0; i < layouts.size(); i++) {
			imageManager.recordImageLayoutTransition(renderTargets [i], 0, 0, layouts [i],
													 cmdBuffer);
		}

		const vk::Framebuffer framebuffer =
			createFramebuffer(renderTargets, imageManager, renderArea.extent, renderPass,
							  core.getContext().getDevice());

		if (!framebuffer) {
			vkcv_log(LogLevel::ERROR, "Failed to create temporary framebuffer");
			return;
		}

		auto submitFunction = [&](const vk::CommandBuffer &cmdBuffer) {
			const std::vector<vk::ClearValue> clearValues =
				createAttachmentClearValues(attachments);

			const vk::RenderPassBeginInfo beginInfo(renderPass, framebuffer, renderArea,
													clearValues.size(), clearValues.data());

			cmdBuffer.beginRenderPass(beginInfo, {}, {});
			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline, {});

			const GraphicsPipelineConfig &pipeConfig =
				pipelineManager.getPipelineConfig(pipelineHandle);

			if (pipeConfig.isViewportDynamic()) {
				recordDynamicViewport(cmdBuffer, width, height);
			}

			if (record) {
				record(cmdBuffer);
			}

			cmdBuffer.endRenderPass();
		};

		auto finishFunction = [framebuffer, &core]() {
			core.getContext().getDevice().destroy(framebuffer);
		};

		core.recordCommandsToStream(cmdStreamHandle, submitFunction, finishFunction);
	}

	void Core::recordDrawcallsToCmdStream(const CommandStreamHandle &cmdStreamHandle,
										  const GraphicsPipelineHandle &pipelineHandle,
										  const PushConstants &pushConstantData,
										  const std::vector<InstanceDrawcall> &drawcalls,
										  const std::vector<ImageHandle> &renderTargets,
										  const WindowHandle &windowHandle) {

		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}

		const vk::PipelineLayout pipelineLayout =
			m_GraphicsPipelineManager->getVkPipelineLayout(pipelineHandle);

		auto recordFunction = [&](const vk::CommandBuffer &cmdBuffer) {
			for (size_t i = 0; i < drawcalls.size(); i++) {
				recordDrawcall(*m_DescriptorSetManager, *m_BufferManager, drawcalls [i], cmdBuffer,
							   pipelineLayout, pushConstantData, i);
			}
		};

		recordGraphicsPipeline(*this, *m_CommandStreamManager, *m_GraphicsPipelineManager,
							   *m_PassManager, *m_ImageManager, cmdStreamHandle, pipelineHandle,
							   pushConstantData, renderTargets, windowHandle, recordFunction);
	}

	static void
	recordIndirectDrawcall(const Core &core, const DescriptorSetManager &descriptorSetManager,
						   const BufferManager &bufferManager, vk::CommandBuffer cmdBuffer,
						   vk::PipelineLayout pipelineLayout, const PushConstants &pushConstantData,
						   size_t drawcallIndex, const IndirectDrawcall &drawcall) {
		for (const auto &usage : drawcall.getDescriptorSetUsages()) {
			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, pipelineLayout, usage.location,
				descriptorSetManager.getDescriptorSet(usage.descriptorSet).vulkanHandle,
				usage.dynamicOffsets);
		}

		const auto &vertexData = drawcall.getVertexData();

		for (uint32_t i = 0; i < vertexData.getVertexBufferBindings().size(); i++) {
			const auto &vertexBinding = vertexData.getVertexBufferBindings() [i];

			cmdBuffer.bindVertexBuffers(i, bufferManager.getBuffer(vertexBinding.buffer),
										vertexBinding.offset);
		}

		if (pushConstantData.getSizePerDrawcall() > 0) {
			cmdBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eAll, 0,
									pushConstantData.getSizePerDrawcall(),
									pushConstantData.getDrawcallData(0));
		}

		if (vertexData.getIndexBuffer()) {
			cmdBuffer.bindIndexBuffer(bufferManager.getBuffer(vertexData.getIndexBuffer()), 0,
									  getIndexType(vertexData.getIndexBitCount()));

			cmdBuffer.drawIndexedIndirect(bufferManager.getBuffer(drawcall.getIndirectDrawBuffer()),
										  0, drawcall.getDrawCount(),
										  sizeof(vk::DrawIndexedIndirectCommand));
		} else {
			cmdBuffer.drawIndirect(bufferManager.getBuffer(drawcall.getIndirectDrawBuffer()), 0,
								   drawcall.getDrawCount(), sizeof(vk::DrawIndirectCommand));
		}
	}

	void Core::recordIndirectDrawcallsToCmdStream(
		const vkcv::CommandStreamHandle cmdStreamHandle,
		const vkcv::GraphicsPipelineHandle &pipelineHandle,
		const vkcv::PushConstants &pushConstantData, const std::vector<IndirectDrawcall> &drawcalls,
		const std::vector<ImageHandle> &renderTargets, const vkcv::WindowHandle &windowHandle) {

		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}

		const vk::PipelineLayout pipelineLayout =
			m_GraphicsPipelineManager->getVkPipelineLayout(pipelineHandle);

		auto recordFunction = [&](const vk::CommandBuffer &cmdBuffer) {
			for (size_t i = 0; i < drawcalls.size(); i++) {
				recordIndirectDrawcall(*this, *m_DescriptorSetManager, *m_BufferManager, cmdBuffer,
									   pipelineLayout, pushConstantData, i, drawcalls [i]);
			}
		};

		recordGraphicsPipeline(*this, *m_CommandStreamManager, *m_GraphicsPipelineManager,
							   *m_PassManager, *m_ImageManager, cmdStreamHandle, pipelineHandle,
							   pushConstantData, renderTargets, windowHandle, recordFunction);
	}

	static void recordMeshShaderDrawcall(const Core &core,
										 const DescriptorSetManager &descriptorSetManager,
										 vk::CommandBuffer cmdBuffer,
										 vk::PipelineLayout pipelineLayout,
										 const PushConstants &pushConstantData,
										 size_t drawcallIndex, const TaskDrawcall &drawcall) {

		static PFN_vkCmdDrawMeshTasksNV cmdDrawMeshTasks =
			reinterpret_cast<PFN_vkCmdDrawMeshTasksNV>(
				core.getContext().getDevice().getProcAddr("vkCmdDrawMeshTasksNV"));

		if (!cmdDrawMeshTasks) {
			vkcv_log(LogLevel::ERROR, "Mesh shader drawcalls are not supported");
			return;
		}

		for (const auto &descriptorUsage : drawcall.getDescriptorSetUsages()) {
			cmdBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics, pipelineLayout, descriptorUsage.location,
				descriptorSetManager.getDescriptorSet(descriptorUsage.descriptorSet).vulkanHandle,
				descriptorUsage.dynamicOffsets);
		}

		if (pushConstantData.getData()) {
			cmdBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eAll, 0,
									pushConstantData.getSizePerDrawcall(),
									pushConstantData.getDrawcallData(drawcallIndex));
		}

		cmdDrawMeshTasks(VkCommandBuffer(cmdBuffer), drawcall.getTaskCount(), 0);
	}

	void Core::recordMeshShaderDrawcalls(const CommandStreamHandle &cmdStreamHandle,
										 const GraphicsPipelineHandle &pipelineHandle,
										 const PushConstants &pushConstantData,
										 const std::vector<TaskDrawcall> &drawcalls,
										 const std::vector<ImageHandle> &renderTargets,
										 const WindowHandle &windowHandle) {

		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}

		const vk::PipelineLayout pipelineLayout =
			m_GraphicsPipelineManager->getVkPipelineLayout(pipelineHandle);

		auto recordFunction = [&](const vk::CommandBuffer &cmdBuffer) {
			for (size_t i = 0; i < drawcalls.size(); i++) {
				recordMeshShaderDrawcall(*this, *m_DescriptorSetManager, cmdBuffer, pipelineLayout,
										 pushConstantData, i, drawcalls [i]);
			}
		};

		recordGraphicsPipeline(*this, *m_CommandStreamManager, *m_GraphicsPipelineManager,
							   *m_PassManager, *m_ImageManager, cmdStreamHandle, pipelineHandle,
							   pushConstantData, renderTargets, windowHandle, recordFunction);
	}

	void Core::recordRayGenerationToCmdStream(
		const CommandStreamHandle &cmdStreamHandle,
		const RayTracingPipelineHandle &rayTracingPipeline,
		const DispatchSize &dispatchSize,
		const std::vector<DescriptorSetUsage> &descriptorSetUsages,
		const PushConstants &pushConstants,
		const vkcv::WindowHandle &windowHandle) {
		
		const SwapchainHandle swapchainHandle = getWindow(windowHandle).getSwapchain();
		
		const vk::Pipeline pipeline = m_RayTracingPipelineManager->getVkPipeline(
				rayTracingPipeline
		);
		
		const vk::PipelineLayout pipelineLayout = m_RayTracingPipelineManager->getVkPipelineLayout(
				rayTracingPipeline
		);
		
		const vk::StridedDeviceAddressRegionKHR *rayGenAddress = (
				m_RayTracingPipelineManager->getRayGenShaderBindingTableAddress(rayTracingPipeline)
		);
		
		const vk::StridedDeviceAddressRegionKHR *rayMissAddress = (
				m_RayTracingPipelineManager->getMissShaderBindingTableAddress(rayTracingPipeline)
		);
		
		const vk::StridedDeviceAddressRegionKHR *rayHitAddress = (
				m_RayTracingPipelineManager->getHitShaderBindingTableAddress(rayTracingPipeline)
		);
		
		const vk::StridedDeviceAddressRegionKHR *rayCallAddress = (
				m_RayTracingPipelineManager->getCallShaderBindingTableAddress(rayTracingPipeline)
		);

		auto submitFunction = [&](const vk::CommandBuffer &cmdBuffer) {
			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline);
			
			for (const auto &usage : descriptorSetUsages) {
				cmdBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eRayTracingKHR,
					pipelineLayout,
					usage.location,
					{ m_DescriptorSetManager->getDescriptorSet(usage.descriptorSet).vulkanHandle },
					usage.dynamicOffsets
				);
			}

			if (pushConstants.getSizePerDrawcall() > 0) {
				cmdBuffer.pushConstants(
					pipelineLayout,
					vk::ShaderStageFlagBits::eAll,
					0,
					pushConstants.getSizePerDrawcall(),
					pushConstants.getData()
				);
			}
			
			cmdBuffer.traceRaysKHR(
					rayGenAddress,
					rayMissAddress,
					rayHitAddress,
					rayCallAddress,
					dispatchSize.x(),
					dispatchSize.y(),
					dispatchSize.z(),
					m_Context.getDispatchLoaderDynamic()
			);
		};
		
		recordCommandsToStream(cmdStreamHandle, submitFunction, nullptr);
	}

	void Core::recordComputeDispatchToCmdStream(
		const CommandStreamHandle &cmdStreamHandle,
		const ComputePipelineHandle &computePipeline,
		const DispatchSize &dispatchSize,
		const std::vector<DescriptorSetUsage> &descriptorSetUsages,
		const PushConstants &pushConstants) {
		auto submitFunction = [&](const vk::CommandBuffer &cmdBuffer) {
			const auto pipelineLayout =
				m_ComputePipelineManager->getVkPipelineLayout(computePipeline);

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute,
								   m_ComputePipelineManager->getVkPipeline(computePipeline));
			for (const auto &usage : descriptorSetUsages) {
				cmdBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eCompute, pipelineLayout, usage.location,
					{ m_DescriptorSetManager->getDescriptorSet(usage.descriptorSet).vulkanHandle },
					usage.dynamicOffsets);
			}
			if (pushConstants.getSizePerDrawcall() > 0) {
				cmdBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0,
										pushConstants.getSizePerDrawcall(),
										pushConstants.getData());
			}

			cmdBuffer.dispatch(dispatchSize.x(), dispatchSize.y(), dispatchSize.z());
		};

		recordCommandsToStream(cmdStreamHandle, submitFunction, nullptr);
	}

	void Core::recordBeginDebugLabel(const CommandStreamHandle &cmdStream, const std::string &label,
									 const std::array<float, 4> &color) {
#ifdef VULKAN_DEBUG_LABELS
		static PFN_vkCmdBeginDebugUtilsLabelEXT beginDebugLabel =
			reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
				m_Context.getDevice().getProcAddr("vkCmdBeginDebugUtilsLabelEXT"));

		if (!beginDebugLabel) {
			return;
		}

		auto submitFunction = [&](const vk::CommandBuffer &cmdBuffer) {
			const vk::DebugUtilsLabelEXT debug(label.c_str(), color);

			beginDebugLabel(static_cast<VkCommandBuffer>(cmdBuffer),
							&(static_cast<const VkDebugUtilsLabelEXT &>(debug)));
		};

		recordCommandsToStream(cmdStream, submitFunction, nullptr);
#endif
	}

	void Core::recordEndDebugLabel(const CommandStreamHandle &cmdStream) {
#ifdef VULKAN_DEBUG_LABELS
		static PFN_vkCmdEndDebugUtilsLabelEXT endDebugLabel =
			reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
				m_Context.getDevice().getProcAddr("vkCmdEndDebugUtilsLabelEXT"));

		if (!endDebugLabel) {
			return;
		}

		auto submitFunction = [&](const vk::CommandBuffer &cmdBuffer) {
			endDebugLabel(static_cast<VkCommandBuffer>(cmdBuffer));
		};

		recordCommandsToStream(cmdStream, submitFunction, nullptr);
#endif
	}

	void Core::recordComputeIndirectDispatchToCmdStream(
		const CommandStreamHandle cmdStream, const ComputePipelineHandle computePipeline,
		const vkcv::BufferHandle buffer, const size_t bufferArgOffset,
		const std::vector<DescriptorSetUsage> &descriptorSetUsages,
		const PushConstants &pushConstants) {

		auto submitFunction = [&](const vk::CommandBuffer &cmdBuffer) {
			const auto pipelineLayout =
				m_ComputePipelineManager->getVkPipelineLayout(computePipeline);

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute,
								   m_ComputePipelineManager->getVkPipeline(computePipeline));
			for (const auto &usage : descriptorSetUsages) {
				cmdBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eCompute, pipelineLayout, usage.location,
					{ m_DescriptorSetManager->getDescriptorSet(usage.descriptorSet).vulkanHandle },
					usage.dynamicOffsets);
			}
			if (pushConstants.getSizePerDrawcall() > 0) {
				cmdBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eCompute, 0,
										pushConstants.getSizePerDrawcall(),
										pushConstants.getData());
			}
			cmdBuffer.dispatchIndirect(m_BufferManager->getBuffer(buffer), bufferArgOffset);
		};

		recordCommandsToStream(cmdStream, submitFunction, nullptr);
	}

	void Core::endFrame(const WindowHandle &windowHandle) {
		SwapchainHandle swapchainHandle = m_WindowManager->getWindow(windowHandle).getSwapchain();

		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}

		const std::array<vk::Semaphore, 2> waitSemaphores { m_RenderFinished,
															m_SwapchainImageAcquired };

		const vk::SwapchainKHR &swapchain =
			m_SwapchainManager->getSwapchain(swapchainHandle).m_Swapchain;
		const vk::PresentInfoKHR presentInfo(waitSemaphores, swapchain,
											 m_currentSwapchainImageIndex);

		vk::Result result;

		try {
			result = m_Context.getDevice()
						 .getQueue(m_SwapchainManager->getPresentQueueIndex(swapchainHandle), 0)
						 .presentKHR(presentInfo);
		} catch (const vk::OutOfDateKHRError &e) {
			result = vk::Result::eErrorOutOfDateKHR;
		} catch (const vk::DeviceLostError &e) {
			result = vk::Result::eErrorDeviceLost;
		}

		if ((result != vk::Result::eSuccess) && (result != vk::Result::eSuboptimalKHR)) {
			vkcv_log(LogLevel::ERROR, "Swapchain presentation failed (%s)",
					 vk::to_string(result).c_str());
		} else if (result == vk::Result::eSuboptimalKHR) {
			vkcv_log(LogLevel::WARNING, "Swapchain presentation is suboptimal");
			m_SwapchainManager->signalRecreation(swapchainHandle);
		}
	}

	/**
	 * @brief Returns a queue of a given type from a queue manager.
	 *
	 * @param[in] type Type of queue
	 * @param[in] queueManager Queue manager
	 * @return Queue of a given type
	 */
	static Queue getQueueForSubmit(QueueType type, const QueueManager &queueManager) {
		switch (type) {
		case QueueType::Graphics:
			return queueManager.getGraphicsQueues().front();
		case QueueType::Compute:
			return queueManager.getComputeQueues().front();
		case QueueType::Transfer:
			return queueManager.getTransferQueues().front();
		case QueueType::Present:
			return queueManager.getPresentQueue();
		default: {
			vkcv_log(LogLevel::ERROR, "Unknown queue type");
			return queueManager.getGraphicsQueues().front(); // graphics is the most general queue
		}
		}
	}

	CommandStreamHandle Core::createCommandStream(QueueType queueType) {
		const vkcv::Queue queue = getQueueForSubmit(queueType, m_Context.getQueueManager());
		const vk::CommandPool cmdPool = m_CommandPools [queue.familyIndex];

		return m_CommandStreamManager->createCommandStream(queue.handle, cmdPool);
	}

	void Core::recordCommandsToStream(const CommandStreamHandle &stream,
									  const RecordCommandFunction &record,
									  const FinishCommandFunction &finish) {
		if (record) {
			m_CommandStreamManager->recordCommandsToStream(stream, record);
		}

		if (finish) {
			m_CommandStreamManager->addFinishCallbackToStream(stream, finish);
		}
	}

	void Core::submitCommandStream(const CommandStreamHandle &stream, bool signalRendering) {
		std::vector<vk::Semaphore> waitSemaphores;

		// FIXME: add proper user controllable sync
		std::vector<vk::Semaphore> signalSemaphores;
		if (signalRendering) {
			signalSemaphores.push_back(m_RenderFinished);
		}

		m_CommandStreamManager->submitCommandStreamSynchronous(stream, waitSemaphores,
															   signalSemaphores);
	}

	SamplerHandle Core::createSampler(SamplerFilterType magFilter, SamplerFilterType minFilter,
									  SamplerMipmapMode mipmapMode, SamplerAddressMode addressMode,
									  float mipLodBias, SamplerBorderColor borderColor) {
		return m_SamplerManager->createSampler(magFilter, minFilter, mipmapMode, addressMode,
											   mipLodBias, borderColor);
	}

	ImageHandle Core::createImage(vk::Format format,
								  const ImageConfig& config,
								  bool createMipChain) {
		uint32_t mipCount = 1;
		if (createMipChain) {
			mipCount = 1 + (uint32_t) std::floor(
					std::log2(std::max(
							config.getWidth(),
							std::max(config.getHeight(), config.getDepth()))
					)
			);
		}

		return m_ImageManager->createImage(
				format,
				mipCount,
				config
		);
	}

	void Core::fillImage(const ImageHandle &image,
						 const void* data,
						 size_t size,
						 uint32_t firstLayer,
						 uint32_t layerCount) {
		m_ImageManager->fillImage(image, data, size, firstLayer, layerCount);
	}

	void Core::switchImageLayout(const ImageHandle &image, vk::ImageLayout layout) {
		m_ImageManager->switchImageLayoutImmediate(image, layout);
	}

	Downsampler &Core::getDownsampler() {
		return *m_downsampler;
	}

	WindowHandle Core::createWindow(const std::string &applicationName, uint32_t windowWidth,
									uint32_t windowHeight, bool resizeable) {
		WindowHandle windowHandle = m_WindowManager->createWindow(
			*m_SwapchainManager, applicationName, windowWidth, windowHeight, resizeable);

		SwapchainHandle swapchainHandle = m_WindowManager->getWindow(windowHandle).getSwapchain();
		setSwapchainImages(swapchainHandle);
		return windowHandle;
	}

	Window &Core::getWindow(const WindowHandle &handle) {
		return m_WindowManager->getWindow(handle);
	}

	vk::Format Core::getSwapchainFormat(const SwapchainHandle &swapchain) const {
		return m_SwapchainManager->getFormat(swapchain);
	}

	uint32_t Core::getSwapchainImageCount(const SwapchainHandle &swapchain) const {
		return m_SwapchainManager->getImageCount(swapchain);
	}

	vk::Extent2D Core::getSwapchainExtent(const SwapchainHandle &swapchain) const {
		return m_SwapchainManager->getExtent(swapchain);
	}

	uint32_t Core::getImageWidth(const ImageHandle &image) {
		return m_ImageManager->getImageWidth(image);
	}

	uint32_t Core::getImageHeight(const ImageHandle &image) {
		return m_ImageManager->getImageHeight(image);
	}

	uint32_t Core::getImageDepth(const ImageHandle &image) {
		return m_ImageManager->getImageDepth(image);
	}

	vk::Format Core::getImageFormat(const ImageHandle &image) {
		return m_ImageManager->getImageFormat(image);
	}

	bool Core::isImageSupportingStorage(const ImageHandle &image) {
		return m_ImageManager->isImageSupportingStorage(image);
	}

	uint32_t Core::getImageMipLevels(const ImageHandle &image) {
		return m_ImageManager->getImageMipCount(image);
	}

	uint32_t Core::getImageArrayLayers(const ImageHandle &image) {
		return m_ImageManager->getImageArrayLayers(image);
	}

	DescriptorSetLayoutHandle Core::createDescriptorSetLayout(const DescriptorBindings &bindings) {
		return m_DescriptorSetLayoutManager->createDescriptorSetLayout(bindings);
	}

	DescriptorSetHandle Core::createDescriptorSet(const DescriptorSetLayoutHandle &layout) {
		return m_DescriptorSetManager->createDescriptorSet(layout);
	}

	void Core::writeDescriptorSet(DescriptorSetHandle handle, const DescriptorWrites &writes) {
		m_DescriptorSetManager->writeDescriptorSet(handle, writes, *m_ImageManager,
												   *m_BufferManager, *m_SamplerManager);
	}

	void Core::prepareSwapchainImageForPresent(const CommandStreamHandle &cmdStream) {
		auto swapchainHandle = ImageHandle::createSwapchainImageHandle();
		recordCommandsToStream(
			cmdStream,
			[swapchainHandle, this](const vk::CommandBuffer cmdBuffer) {
				m_ImageManager->recordImageLayoutTransition(
					swapchainHandle, 0, 0, vk::ImageLayout::ePresentSrcKHR, cmdBuffer);
			},
			nullptr);
	}

	void Core::prepareImageForSampling(const CommandStreamHandle &cmdStream,
									   const ImageHandle &image, uint32_t mipLevelCount,
									   uint32_t mipLevelOffset) {
		recordCommandsToStream(
			cmdStream,
			[image, mipLevelCount, mipLevelOffset, this](const vk::CommandBuffer cmdBuffer) {
				m_ImageManager->recordImageLayoutTransition(image, mipLevelCount, mipLevelOffset,
															vk::ImageLayout::eShaderReadOnlyOptimal,
															cmdBuffer);
			},
			nullptr);
	}

	void Core::prepareImageForStorage(const CommandStreamHandle &cmdStream,
									  const ImageHandle &image, uint32_t mipLevelCount,
									  uint32_t mipLevelOffset) {
		recordCommandsToStream(
			cmdStream,
			[image, mipLevelCount, mipLevelOffset, this](const vk::CommandBuffer cmdBuffer) {
				m_ImageManager->recordImageLayoutTransition(image, mipLevelCount, mipLevelOffset,
															vk::ImageLayout::eGeneral, cmdBuffer);
			},
			nullptr);
	}

	void Core::prepareImageForAttachmentManually(const vk::CommandBuffer &cmdBuffer,
												 const ImageHandle &image) {
		transitionRendertargetsToAttachmentLayout({ image }, *m_ImageManager, cmdBuffer);
	}

	void Core::updateImageLayoutManual(const vkcv::ImageHandle &image,
									   const vk::ImageLayout layout) {
		m_ImageManager->updateImageLayoutManual(image, layout);
	}

	void Core::recordImageMemoryBarrier(const CommandStreamHandle &cmdStream,
										const ImageHandle &image) {
		recordCommandsToStream(
			cmdStream,
			[image, this](const vk::CommandBuffer cmdBuffer) {
				m_ImageManager->recordImageMemoryBarrier(image, cmdBuffer);
			},
			nullptr);
	}

	void Core::recordBufferMemoryBarrier(const CommandStreamHandle &cmdStream,
										 const BufferHandle &buffer) {
		recordCommandsToStream(
			cmdStream,
			[buffer, this](const vk::CommandBuffer cmdBuffer) {
				m_BufferManager->recordBufferMemoryBarrier(buffer, cmdBuffer);
			},
			nullptr);
	}

	void Core::resolveMSAAImage(const CommandStreamHandle &cmdStream, const ImageHandle &src,
								const ImageHandle &dst) {
		recordCommandsToStream(
			cmdStream,
			[src, dst, this](const vk::CommandBuffer cmdBuffer) {
				m_ImageManager->recordMSAAResolve(cmdBuffer, src, dst);
			},
			nullptr);
	}

	vk::ImageView Core::getSwapchainImageView() const {
		return m_ImageManager->getVulkanImageView(vkcv::ImageHandle::createSwapchainImageHandle());
	}

	void Core::recordMemoryBarrier(const CommandStreamHandle &cmdStream) {
		recordCommandsToStream(
			cmdStream,
			[](const vk::CommandBuffer cmdBuffer) {
				vk::MemoryBarrier barrier(
					vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead,
					vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead);

				cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
										  vk::PipelineStageFlagBits::eAllCommands,
										  vk::DependencyFlags(), 1, &barrier, 0, nullptr, 0,
										  nullptr);
			},
			nullptr);
	}

	void Core::recordBlitImage(const CommandStreamHandle &cmdStream, const ImageHandle &src,
							   const ImageHandle &dst, SamplerFilterType filterType) {
		recordCommandsToStream(
			cmdStream,
			[&](const vk::CommandBuffer cmdBuffer) {
				m_ImageManager->recordImageLayoutTransition(
					src, 0, 0, vk::ImageLayout::eTransferSrcOptimal, cmdBuffer);

				m_ImageManager->recordImageLayoutTransition(
					dst, 0, 0, vk::ImageLayout::eTransferDstOptimal, cmdBuffer);

				const std::array<vk::Offset3D, 2> srcOffsets = {
					vk::Offset3D(0, 0, 0), vk::Offset3D(m_ImageManager->getImageWidth(src),
														m_ImageManager->getImageHeight(src), 1)
				};

				const std::array<vk::Offset3D, 2> dstOffsets = {
					vk::Offset3D(0, 0, 0), vk::Offset3D(m_ImageManager->getImageWidth(dst),
														m_ImageManager->getImageHeight(dst), 1)
				};

				const bool srcDepth = isDepthFormat(m_ImageManager->getImageFormat(src));
				const bool dstDepth = isDepthFormat(m_ImageManager->getImageFormat(dst));

				const vk::ImageBlit blit = vk::ImageBlit(
					vk::ImageSubresourceLayers(srcDepth ? vk::ImageAspectFlagBits::eDepth :
														  vk::ImageAspectFlagBits::eColor,
											   0, 0, 1),
					srcOffsets,
					vk::ImageSubresourceLayers(dstDepth ? vk::ImageAspectFlagBits::eDepth :
														  vk::ImageAspectFlagBits::eColor,
											   0, 0, 1),
					dstOffsets);

				cmdBuffer.blitImage(m_ImageManager->getVulkanImage(src),
									vk::ImageLayout::eTransferSrcOptimal,
									m_ImageManager->getVulkanImage(dst),
									vk::ImageLayout::eTransferDstOptimal, 1, &blit,
									filterType == SamplerFilterType::LINEAR ? vk::Filter::eLinear :
																			  vk::Filter::eNearest);
			},
			nullptr);
	}

	void Core::setSwapchainImages(SwapchainHandle handle) {
		const auto &swapchain = m_SwapchainManager->getSwapchain(handle);
		const auto swapchainImages = m_SwapchainManager->getSwapchainImages(handle);
		const auto swapchainImageViews = m_SwapchainManager->createSwapchainImageViews(handle);

		m_ImageManager->setSwapchainImages(swapchainImages, swapchainImageViews,
										   swapchain.m_Extent.width, swapchain.m_Extent.height,
										   swapchain.m_Format);
	}

	static void setDebugObjectLabel(const vk::Device &device, const vk::ObjectType &type,
									uint64_t handle, const std::string &label) {
#ifdef VULKAN_DEBUG_LABELS
		static PFN_vkSetDebugUtilsObjectNameEXT setDebugLabel =
			reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
				device.getProcAddr("vkSetDebugUtilsObjectNameEXT"));

		if (!setDebugLabel) {
			return;
		}

		const vk::DebugUtilsObjectNameInfoEXT debug(type, handle, label.c_str());

		setDebugLabel(static_cast<VkDevice>(device),
					  &(static_cast<const VkDebugUtilsObjectNameInfoEXT &>(debug)));
#endif
	}

	void Core::setDebugLabel(const BufferHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}

		setDebugObjectLabel(m_Context.getDevice(), vk::ObjectType::eBuffer,
							uint64_t(static_cast<VkBuffer>(m_BufferManager->getBuffer(handle))),
							label);
	}

	void Core::setDebugLabel(const PassHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}

		setDebugObjectLabel(m_Context.getDevice(), vk::ObjectType::eRenderPass,
							uint64_t(static_cast<VkRenderPass>(m_PassManager->getVkPass(handle))),
							label);
	}

	void Core::setDebugLabel(const GraphicsPipelineHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}

		setDebugObjectLabel(
			m_Context.getDevice(), vk::ObjectType::ePipeline,
			uint64_t(static_cast<VkPipeline>(m_GraphicsPipelineManager->getVkPipeline(handle))),
			label);
	}

	void Core::setDebugLabel(const ComputePipelineHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}

		setDebugObjectLabel(
			m_Context.getDevice(), vk::ObjectType::ePipeline,
			uint64_t(static_cast<VkPipeline>(m_ComputePipelineManager->getVkPipeline(handle))),
			label);
	}

	void Core::setDebugLabel(const DescriptorSetHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}

		setDebugObjectLabel(m_Context.getDevice(), vk::ObjectType::eDescriptorSet,
							uint64_t(static_cast<VkDescriptorSet>(
								m_DescriptorSetManager->getDescriptorSet(handle).vulkanHandle)),
							label);
	}

	void Core::setDebugLabel(const SamplerHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}

		setDebugObjectLabel(
			m_Context.getDevice(), vk::ObjectType::eSampler,
			uint64_t(static_cast<VkSampler>(m_SamplerManager->getVulkanSampler(handle))), label);
	}

	void Core::setDebugLabel(const ImageHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		} else if (handle.isSwapchainImage()) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to swapchain image");
			return;
		}

		setDebugObjectLabel(m_Context.getDevice(), vk::ObjectType::eImage,
							uint64_t(static_cast<VkImage>(m_ImageManager->getVulkanImage(handle))),
							label);
	}

	void Core::setDebugLabel(const CommandStreamHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}

		setDebugObjectLabel(m_Context.getDevice(), vk::ObjectType::eCommandBuffer,
							uint64_t(static_cast<VkCommandBuffer>(
								m_CommandStreamManager->getStreamCommandBuffer(handle))),
							label);
	}

	void Core::run(const vkcv::WindowFrameFunction &frame) {
		auto start = std::chrono::system_clock::now();
		double t = 0.0;

		if (!frame)
			return;

		while (Window::hasOpenWindow()) {
			vkcv::Window::pollEvents();

			auto end = std::chrono::system_clock::now();
			auto deltatime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
			start = end;

			double dt = 0.000001 * static_cast<double>(deltatime.count());

			for (const auto &window : m_WindowManager->getWindowHandles()) {
				uint32_t swapchainWidth, swapchainHeight;
				if (!beginFrame(swapchainWidth, swapchainHeight, window)) {
					continue;
				}

				frame(window, t, dt, swapchainWidth, swapchainHeight);
				endFrame(window);
			}

			t += dt;
		}
	}

	vk::RenderPass Core::getVulkanRenderPass(const PassHandle &handle) const {
		return m_PassManager->getVkPass(handle);
	}

	vk::Pipeline Core::getVulkanPipeline(const GraphicsPipelineHandle &handle) const {
		return m_GraphicsPipelineManager->getVkPipeline(handle);
	}

	vk::Pipeline Core::getVulkanPipeline(const ComputePipelineHandle &handle) const {
		return m_ComputePipelineManager->getVkPipeline(handle);
	}

	vk::DescriptorSetLayout
	Core::getVulkanDescriptorSetLayout(const DescriptorSetLayoutHandle &handle) const {
		return m_DescriptorSetLayoutManager->getDescriptorSetLayout(handle).vulkanHandle;
	}

	vk::DescriptorSet Core::getVulkanDescriptorSet(const DescriptorSetHandle &handle) const {
		return m_DescriptorSetManager->getDescriptorSet(handle).vulkanHandle;
	}

	vk::Buffer Core::getVulkanBuffer(const BufferHandle &handle) const {
		return m_BufferManager->getBuffer(handle);
	}

	vk::Sampler Core::getVulkanSampler(const SamplerHandle &handle) const {
		return m_SamplerManager->getVulkanSampler(handle);
	}

	vk::Image Core::getVulkanImage(const ImageHandle &handle) const {
		return m_ImageManager->getVulkanImage(handle);
	}

	vk::ImageView Core::getVulkanImageView(const vkcv::ImageHandle &handle) const {
		return m_ImageManager->getVulkanImageView(handle);
	}

	vk::DeviceMemory Core::getVulkanDeviceMemory(const BufferHandle &handle) const {
		return m_BufferManager->getDeviceMemory(handle);
	}

	vk::DeviceMemory Core::getVulkanDeviceMemory(const ImageHandle &handle) const {
		return m_ImageManager->getVulkanDeviceMemory(handle);
	}

} // namespace vkcv
