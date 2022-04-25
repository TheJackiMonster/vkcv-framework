/**
 * @authors Artur Wasmut
 * @file src/vkcv/Core.cpp
 * @brief Handling of global states regarding dependencies
 */

#include <GLFW/glfw3.h>

#include "vkcv/Core.hpp"
#include "PassManager.hpp"
#include "GraphicsPipelineManager.hpp"
#include "ComputePipelineManager.hpp"
#include "vkcv/BufferManager.hpp"
#include "SamplerManager.hpp"
#include "ImageManager.hpp"
#include "DescriptorManager.hpp"
#include "WindowManager.hpp"
#include "ImageLayoutTransitions.hpp"
#include "CommandStreamManager.hpp"
#include <cmath>
#include "vkcv/Logger.hpp"

namespace vkcv
{
    Core Core::create(const char *applicationName,
                      uint32_t applicationVersion,
                      const std::vector<vk::QueueFlagBits>& queueFlags,
					  const Features& features,
                      const std::vector<const char *>& instanceExtensions)
    {
        Context context = Context::create(
        		applicationName, applicationVersion,
        		queueFlags,
				features,
        		instanceExtensions
		);

        const auto& queueManager = context.getQueueManager();
        
		const std::unordered_set<int>	queueFamilySet			= generateQueueFamilyIndexSet(queueManager);
		const auto						commandResources		= createCommandResources(context.getDevice(), queueFamilySet);
		const auto						defaultSyncResources	= createSyncResources(context.getDevice());

        return Core(std::move(context) , commandResources, defaultSyncResources);
    }

    const Context &Core::getContext() const
    {
        return m_Context;
    }

    Core::Core(Context &&context, const CommandResources& commandResources, const SyncResources& syncResources) noexcept :
            m_Context(std::move(context)),
            m_PassManager{std::make_unique<PassManager>(m_Context.m_Device)},
            m_PipelineManager{std::make_unique<GraphicsPipelineManager>(m_Context.m_Device, m_Context.m_PhysicalDevice)},
            m_ComputePipelineManager{std::make_unique<ComputePipelineManager>(m_Context.m_Device)},
            m_DescriptorManager(std::make_unique<DescriptorManager>(m_Context.m_Device)),
            m_BufferManager{std::unique_ptr<BufferManager>(new BufferManager())},
            m_SamplerManager(std::unique_ptr<SamplerManager>(new SamplerManager(m_Context.m_Device))),
            m_ImageManager{std::unique_ptr<ImageManager>(new ImageManager(*m_BufferManager))},
            m_CommandStreamManager{std::unique_ptr<CommandStreamManager>(new CommandStreamManager)},
			m_WindowManager(std::make_unique<WindowManager>()),
			m_SwapchainManager(std::make_unique<SwapchainManager>()),
            m_CommandResources(commandResources),
            m_SyncResources(syncResources)
	{
		m_BufferManager->m_core = this;
		m_BufferManager->init();
		m_CommandStreamManager->init(this);
		m_SwapchainManager->m_context = &m_Context;
		m_ImageManager->m_core = this;
	}

	Core::~Core() noexcept {
		m_Context.getDevice().waitIdle();

		destroyCommandResources(m_Context.getDevice(), m_CommandResources);
		destroySyncResources(m_Context.getDevice(), m_SyncResources);
	}
	
	GraphicsPipelineHandle Core::createGraphicsPipeline(const GraphicsPipelineConfig &config)
    {
        return m_PipelineManager->createPipeline(config, *m_PassManager, *m_DescriptorManager);
    }

    ComputePipelineHandle Core::createComputePipeline(const ComputePipelineConfig &config)
    {
		std::vector<vk::DescriptorSetLayout> layouts;
		layouts.resize(config.m_DescriptorSetLayouts.size());
	
		for (size_t i = 0; i < layouts.size(); i++) {
			layouts[i] = getDescriptorSetLayout(config.m_DescriptorSetLayouts[i]).vulkanHandle;
		}
		
        return m_ComputePipelineManager->createComputePipeline(config.m_ShaderProgram, layouts);
    }

    PassHandle Core::createPass(const PassConfig &config)
    {
        return m_PassManager->createPass(config);
    }

	Result Core::acquireSwapchainImage(const SwapchainHandle &swapchainHandle) {
    	uint32_t imageIndex;
    	vk::Result result;
    	
		try {
			result = m_Context.getDevice().acquireNextImageKHR(
					m_SwapchainManager->getSwapchain(swapchainHandle).getSwapchain(),
					std::numeric_limits<uint64_t>::max(),
					m_SyncResources.swapchainImageAcquired,
					nullptr,
					&imageIndex, {}
			);
		} catch (const vk::OutOfDateKHRError& e) {
			result = vk::Result::eErrorOutOfDateKHR;
		} catch (const vk::DeviceLostError& e) {
			result = vk::Result::eErrorDeviceLost;
		}
		
		if ((result != vk::Result::eSuccess) &&
			(result != vk::Result::eSuboptimalKHR)) {
			vkcv_log(LogLevel::ERROR, "%s", vk::to_string(result).c_str());
			return Result::ERROR;
		} else
		if (result == vk::Result::eSuboptimalKHR) {
			vkcv_log(LogLevel::WARNING, "Acquired image is suboptimal");
			m_SwapchainManager->getSwapchain(swapchainHandle).signalSwapchainRecreation();
		}
		
		m_currentSwapchainImageIndex = imageIndex;
		return Result::SUCCESS;
	}

	bool Core::beginFrame(uint32_t& width, uint32_t& height, const WindowHandle &windowHandle) {
		const SwapchainHandle swapchainHandle = m_WindowManager->getWindow(windowHandle).getSwapchainHandle();

		if (m_SwapchainManager->getSwapchain(swapchainHandle).shouldUpdateSwapchain()) {
			m_Context.getDevice().waitIdle();

			m_SwapchainManager->getSwapchain(swapchainHandle).updateSwapchain(m_Context, m_WindowManager->getWindow(windowHandle));
			
			if (!m_SwapchainManager->getSwapchain(swapchainHandle).getSwapchain()) {
				return false;
			}

			setSwapchainImages(swapchainHandle);
		}
		
		const auto& extent = m_SwapchainManager->getSwapchain(swapchainHandle).getExtent();
		
		width = extent.width;
		height = extent.height;
		
		if ((width < MIN_SWAPCHAIN_SIZE) || (height < MIN_SWAPCHAIN_SIZE)) {
			return false;
		}
		
    	if (acquireSwapchainImage( swapchainHandle ) != Result::SUCCESS) {
			vkcv_log(LogLevel::ERROR, "Acquire failed");
    		
    		m_currentSwapchainImageIndex = std::numeric_limits<uint32_t>::max();
    	}
		
		m_Context.getDevice().waitIdle(); // TODO: this is a sin against graphics programming, but its getting late - Alex
		
		m_ImageManager->setCurrentSwapchainImageIndex(m_currentSwapchainImageIndex);

		return (m_currentSwapchainImageIndex != std::numeric_limits<uint32_t>::max());
	}

	std::array<uint32_t, 2> getWidthHeightFromRenderTargets(
		const std::vector<ImageHandle>& renderTargets,
		const Swapchain& swapchain,
		const ImageManager& imageManager) {

		std::array<uint32_t, 2> widthHeight;

		if (renderTargets.size() > 0) {
			const vkcv::ImageHandle firstImage = renderTargets[0];
			if (firstImage.isSwapchainImage()) {
				const auto& swapchainExtent = swapchain.getExtent();
				widthHeight[0] = swapchainExtent.width;
				widthHeight[1] = swapchainExtent.height;
			}
			else {
				widthHeight[0] = imageManager.getImageWidth(firstImage);
				widthHeight[1] = imageManager.getImageHeight(firstImage);
			}
		}
		else {
			widthHeight[0] = 1;
			widthHeight[1] = 1;
		}
		// TODO: validate that width/height match for all attachments
		return widthHeight;
	}

	vk::Framebuffer createFramebuffer(
		const std::vector<ImageHandle>& renderTargets,
		const ImageManager&             imageManager,
		const Swapchain&                swapchain,
		vk::RenderPass                  renderpass,
		vk::Device                      device) {

		std::vector<vk::ImageView> attachmentsViews;
		for (const ImageHandle& handle : renderTargets) {
			attachmentsViews.push_back(imageManager.getVulkanImageView(handle));
		}

		const std::array<uint32_t, 2> widthHeight = getWidthHeightFromRenderTargets(renderTargets, swapchain, imageManager);

		const vk::FramebufferCreateInfo createInfo(
			{},
			renderpass,
			static_cast<uint32_t>(attachmentsViews.size()),
			attachmentsViews.data(),
			widthHeight[0],
			widthHeight[1],
			1);

		return device.createFramebuffer(createInfo);
	}

	void transitionRendertargetsToAttachmentLayout(
		const std::vector<ImageHandle>& renderTargets,
		ImageManager&                   imageManager,
		const vk::CommandBuffer         cmdBuffer) {

		for (const ImageHandle& handle : renderTargets) {
			const bool isDepthImage = isDepthFormat(imageManager.getImageFormat(handle));
			const vk::ImageLayout targetLayout =
				isDepthImage ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal;
			imageManager.recordImageLayoutTransition(handle, targetLayout, cmdBuffer);
		}
	}

	std::vector<vk::ClearValue> createAttachmentClearValues(const std::vector<AttachmentDescription>& attachments) {
		std::vector<vk::ClearValue> clearValues;
		for (const auto& attachment : attachments) {
			if (attachment.load_operation == AttachmentOperation::CLEAR) {
				float clear = 0.0f;

				if (isDepthFormat(attachment.format)) {
					clear = 1.0f;
				}

				clearValues.emplace_back(std::array<float, 4>{
					clear,
						clear,
						clear,
						1.f
				});
			}
		}
		return clearValues;
	}

	void recordDynamicViewport(vk::CommandBuffer cmdBuffer, uint32_t width, uint32_t height) {
		vk::Viewport dynamicViewport(
			0.0f, 0.0f,
			static_cast<float>(width), static_cast<float>(height),
			0.0f, 1.0f
		);

		vk::Rect2D dynamicScissor({ 0, 0 }, { width, height });

		cmdBuffer.setViewport(0, 1, &dynamicViewport);
		cmdBuffer.setScissor(0, 1, &dynamicScissor);
	}
	
	vk::IndexType getIndexType(IndexBitCount indexByteCount){
		switch (indexByteCount) {
			case IndexBitCount::Bit16: return vk::IndexType::eUint16;
			case IndexBitCount::Bit32: return vk::IndexType::eUint32;
			default:
			vkcv_log(LogLevel::ERROR, "unknown Enum");
				return vk::IndexType::eUint16;
		}
	}
	
	void recordDrawcall(
			const Core				&core,
			const DrawcallInfo      &drawcall,
			vk::CommandBuffer       cmdBuffer,
			vk::PipelineLayout      pipelineLayout,
			const PushConstants     &pushConstants,
			const size_t            drawcallIndex) {
		
		for (uint32_t i = 0; i < drawcall.mesh.vertexBufferBindings.size(); i++) {
			const auto& vertexBinding = drawcall.mesh.vertexBufferBindings[i];
			cmdBuffer.bindVertexBuffers(i, vertexBinding.buffer, vertexBinding.offset);
		}
		
		for (const auto& descriptorUsage : drawcall.descriptorSets) {
			cmdBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics,
					pipelineLayout,
					descriptorUsage.setLocation,
					core.getDescriptorSet(descriptorUsage.descriptorSet).vulkanHandle,
					nullptr);
		}
		
		if (pushConstants.getSizePerDrawcall() > 0) {
			cmdBuffer.pushConstants(
					pipelineLayout,
					vk::ShaderStageFlagBits::eAll,
					0,
					pushConstants.getSizePerDrawcall(),
					pushConstants.getDrawcallData(drawcallIndex));
		}
		
		if (drawcall.mesh.indexBuffer) {
			cmdBuffer.bindIndexBuffer(drawcall.mesh.indexBuffer, 0, getIndexType(drawcall.mesh.indexBitCount));
			cmdBuffer.drawIndexed(drawcall.mesh.indexCount, drawcall.instanceCount, 0, 0, {});
		}
		else {
			cmdBuffer.draw(drawcall.mesh.indexCount, drawcall.instanceCount, 0, 0, {});
		}
	}

	void Core::recordDrawcallsToCmdStream(
		const CommandStreamHandle&      cmdStreamHandle,
		const PassHandle&               renderpassHandle,
		const GraphicsPipelineHandle    &pipelineHandle,
        const PushConstants             &pushConstantData,
        const std::vector<DrawcallInfo> &drawcalls,
		const std::vector<ImageHandle>  &renderTargets,
		const WindowHandle              &windowHandle) {

		SwapchainHandle swapchainHandle = m_WindowManager->getWindow(windowHandle).getSwapchainHandle();

		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}

		const std::array<uint32_t, 2> widthHeight = getWidthHeightFromRenderTargets(renderTargets, m_SwapchainManager->getSwapchain(swapchainHandle), *m_ImageManager);
		const auto width  = widthHeight[0];
		const auto height = widthHeight[1];

		const vk::RenderPass        renderpass      = m_PassManager->getVkPass(renderpassHandle);
		const PassConfig            passConfig      = m_PassManager->getPassConfig(renderpassHandle);

		const vk::Pipeline          pipeline        = m_PipelineManager->getVkPipeline(pipelineHandle);
		const vk::PipelineLayout    pipelineLayout  = m_PipelineManager->getVkPipelineLayout(pipelineHandle);
		const vk::Rect2D            renderArea(vk::Offset2D(0, 0), vk::Extent2D(width, height));

		vk::CommandBuffer cmdBuffer = m_CommandStreamManager->getStreamCommandBuffer(cmdStreamHandle);
		transitionRendertargetsToAttachmentLayout(renderTargets, *m_ImageManager, cmdBuffer);

		const vk::Framebuffer framebuffer = createFramebuffer(renderTargets, *m_ImageManager, m_SwapchainManager->getSwapchain(swapchainHandle), renderpass, m_Context.m_Device);

		if (!framebuffer) {
			vkcv_log(LogLevel::ERROR, "Failed to create temporary framebuffer");
			return;
		}

		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Graphics;
		submitInfo.signalSemaphores = { m_SyncResources.renderFinished };

		auto submitFunction = [&](const vk::CommandBuffer& cmdBuffer) {
			const std::vector<vk::ClearValue> clearValues = createAttachmentClearValues(passConfig.attachments);

			const vk::RenderPassBeginInfo beginInfo(renderpass, framebuffer, renderArea, clearValues.size(), clearValues.data());
			cmdBuffer.beginRenderPass(beginInfo, {}, {});

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline, {});

			const GraphicsPipelineConfig &pipeConfig = m_PipelineManager->getPipelineConfig(pipelineHandle);
			if (pipeConfig.m_UseDynamicViewport) {
				recordDynamicViewport(cmdBuffer, width, height);
			}

			for (size_t i = 0; i < drawcalls.size(); i++) {
				recordDrawcall(*this, drawcalls[i], cmdBuffer, pipelineLayout, pushConstantData, i);
			}

			cmdBuffer.endRenderPass();
		};

		auto finishFunction = [framebuffer, this]()
		{
			m_Context.m_Device.destroy(framebuffer);
		};

		recordCommandsToStream(cmdStreamHandle, submitFunction, finishFunction);
	}

    void Core::recordIndexedIndirectDrawcallsToCmdStream(
            const CommandStreamHandle                           cmdStreamHandle,
            const PassHandle                                    renderpassHandle,
            const GraphicsPipelineHandle                        &pipelineHandle,
            const PushConstants                                 &pushConstantData,
            const vkcv::DescriptorSetHandle                     &compiledDescriptorSet,
            const vkcv::Mesh                                    &compiledMesh,
            const std::vector<ImageHandle>                      &renderTargets,
            const vkcv::Buffer<vk::DrawIndexedIndirectCommand>  &indirectBuffer,
            const uint32_t                                      drawCount,
			const WindowHandle                                  &windowHandle) {

        if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
            return;
        }
		SwapchainHandle swapchainHandle = m_WindowManager->getWindow(windowHandle).getSwapchainHandle();
        const std::array<uint32_t, 2> widthHeight = getWidthHeightFromRenderTargets(renderTargets, m_SwapchainManager->getSwapchain(swapchainHandle),
                                                                                    *m_ImageManager);
        const auto width = widthHeight[0];
        const auto height = widthHeight[1];

        const vk::RenderPass        renderpass      = m_PassManager->getVkPass(renderpassHandle);
        const PassConfig            passConfig      = m_PassManager->getPassConfig(renderpassHandle);

        const vk::Pipeline          pipeline        = m_PipelineManager->getVkPipeline(pipelineHandle);
        const vk::PipelineLayout    pipelineLayout  = m_PipelineManager->getVkPipelineLayout(pipelineHandle);
        const vk::Rect2D            renderArea(vk::Offset2D(0, 0), vk::Extent2D(width, height));

        vk::CommandBuffer cmdBuffer = m_CommandStreamManager->getStreamCommandBuffer(cmdStreamHandle);
        transitionRendertargetsToAttachmentLayout(renderTargets, *m_ImageManager, cmdBuffer);

        const vk::Framebuffer framebuffer = createFramebuffer(renderTargets, *m_ImageManager, m_SwapchainManager->getSwapchain(swapchainHandle), renderpass,
                                                              m_Context.m_Device);

        if (!framebuffer) {
            vkcv_log(LogLevel::ERROR, "Failed to create temporary framebuffer");
            return;
        }

        SubmitInfo submitInfo;
        submitInfo.queueType = QueueType::Graphics;
        submitInfo.signalSemaphores = {m_SyncResources.renderFinished};

        auto submitFunction = [&](const vk::CommandBuffer &cmdBuffer) {

            const std::vector<vk::ClearValue> clearValues = createAttachmentClearValues(passConfig.attachments);

            const vk::RenderPassBeginInfo beginInfo(renderpass, framebuffer, renderArea, clearValues.size(),
                                                    clearValues.data());
            cmdBuffer.beginRenderPass(beginInfo, {}, {});

            cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline, {});

            const GraphicsPipelineConfig &pipeConfig = m_PipelineManager->getPipelineConfig(pipelineHandle);
            if (pipeConfig.m_UseDynamicViewport) {
                recordDynamicViewport(cmdBuffer, width, height);
            }

			if (pushConstantData.getSizePerDrawcall() > 0)
			{
				cmdBuffer.pushConstants(
					pipelineLayout,
					vk::ShaderStageFlagBits::eAll,
					0,
					pushConstantData.getSizePerDrawcall(),
					pushConstantData.getDrawcallData(0));
			}

            vkcv::DescriptorSet descSet = m_DescriptorManager->getDescriptorSet(compiledDescriptorSet);

            cmdBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    pipelineLayout,
                    0,
                    descSet.vulkanHandle,
                    nullptr);

			vk::DeviceSize deviceSize = 0;
			cmdBuffer.bindVertexBuffers(0, 1, &compiledMesh.vertexBufferBindings[0].buffer,&deviceSize);
            cmdBuffer.bindIndexBuffer(compiledMesh.indexBuffer, 0, getIndexType(compiledMesh.indexBitCount));

            cmdBuffer.drawIndexedIndirect(
                    indirectBuffer.getVulkanHandle(),
                    0,
                    drawCount,
                    sizeof(vk::DrawIndexedIndirectCommand));

            cmdBuffer.endRenderPass();
        };

        auto finishFunction = [framebuffer, this]() {
            m_Context.m_Device.destroy(framebuffer);
        };

        recordCommandsToStream(cmdStreamHandle, submitFunction, finishFunction);
    }

	void Core::recordMeshShaderDrawcalls(
		const CommandStreamHandle&                          cmdStreamHandle,
		const PassHandle&                                   renderpassHandle,
		const GraphicsPipelineHandle                        &pipelineHandle,
		const PushConstants&                                pushConstantData,
		const std::vector<MeshShaderDrawcall>&              drawcalls,
		const std::vector<ImageHandle>&                     renderTargets,
		const WindowHandle&                                 windowHandle) {

		SwapchainHandle swapchainHandle = m_WindowManager->getWindow(windowHandle).getSwapchainHandle();

		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}

		const std::array<uint32_t, 2> widthHeight = getWidthHeightFromRenderTargets(renderTargets, m_SwapchainManager->getSwapchain(swapchainHandle), *m_ImageManager);
		const auto width  = widthHeight[0];
		const auto height = widthHeight[1];

		const vk::RenderPass        renderpass = m_PassManager->getVkPass(renderpassHandle);
		const PassConfig            passConfig = m_PassManager->getPassConfig(renderpassHandle);

		const vk::Pipeline          pipeline = m_PipelineManager->getVkPipeline(pipelineHandle);
		const vk::PipelineLayout    pipelineLayout = m_PipelineManager->getVkPipelineLayout(pipelineHandle);
		const vk::Rect2D            renderArea(vk::Offset2D(0, 0), vk::Extent2D(width, height));

		vk::CommandBuffer cmdBuffer = m_CommandStreamManager->getStreamCommandBuffer(cmdStreamHandle);
		transitionRendertargetsToAttachmentLayout(renderTargets, *m_ImageManager, cmdBuffer);

		const vk::Framebuffer framebuffer = createFramebuffer(renderTargets, *m_ImageManager, m_SwapchainManager->getSwapchain(swapchainHandle), renderpass, m_Context.m_Device);

		if (!framebuffer) {
			vkcv_log(LogLevel::ERROR, "Failed to create temporary framebuffer");
			return;
		}

		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Graphics;
		submitInfo.signalSemaphores = { m_SyncResources.renderFinished };

		auto submitFunction = [&](const vk::CommandBuffer& cmdBuffer) {
			const std::vector<vk::ClearValue> clearValues = createAttachmentClearValues(passConfig.attachments);

			const vk::RenderPassBeginInfo beginInfo(renderpass, framebuffer, renderArea, clearValues.size(), clearValues.data());
			cmdBuffer.beginRenderPass(beginInfo, {}, {});

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline, {});

			const GraphicsPipelineConfig& pipeConfig = m_PipelineManager->getPipelineConfig(pipelineHandle);
			if (pipeConfig.m_UseDynamicViewport) {
				recordDynamicViewport(cmdBuffer, width, height);
			}

			for (size_t i = 0; i < drawcalls.size(); i++) {
                const uint32_t pushConstantOffset = i * pushConstantData.getSizePerDrawcall();
                recordMeshShaderDrawcall(
					*this,
                    cmdBuffer,
                    pipelineLayout,
                    pushConstantData,
                    pushConstantOffset,
                    drawcalls[i],
                    0
				);
			}

			cmdBuffer.endRenderPass();
		};

		auto finishFunction = [framebuffer, this]() {
			m_Context.m_Device.destroy(framebuffer);
		};

		recordCommandsToStream(cmdStreamHandle, submitFunction, finishFunction);
	}


	void Core::recordRayGenerationToCmdStream(
		CommandStreamHandle cmdStreamHandle,
		vk::Pipeline rtxPipeline,
		vk::PipelineLayout rtxPipelineLayout,
		vk::StridedDeviceAddressRegionKHR rgenRegion,
		vk::StridedDeviceAddressRegionKHR rmissRegion,
		vk::StridedDeviceAddressRegionKHR rchitRegion,
		vk::StridedDeviceAddressRegionKHR rcallRegion,
		const std::vector<DescriptorSetUsage>& descriptorSetUsages,
        const PushConstants& pushConstants,
		const WindowHandle windowHandle) {

		auto submitFunction = [&](const vk::CommandBuffer& cmdBuffer) {
			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rtxPipeline);
			for (const auto& usage : descriptorSetUsages) {
				cmdBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eRayTracingKHR,
					rtxPipelineLayout,
					usage.setLocation,
					{ getDescriptorSet(usage.descriptorSet).vulkanHandle },
					usage.dynamicOffsets
				);
			}

			if (pushConstants.getSizePerDrawcall() > 0) {
				cmdBuffer.pushConstants(
					rtxPipelineLayout,
					(vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eMissKHR | vk::ShaderStageFlagBits::eRaygenKHR), // TODO: add Support for eAnyHitKHR, eCallableKHR, eIntersectionKHR
					0,
					pushConstants.getSizePerDrawcall(),
					pushConstants.getData());
			}
			
			auto m_rtxDispatcher = vk::DispatchLoaderDynamic((PFN_vkGetInstanceProcAddr)m_Context.getInstance().getProcAddr("vkGetInstanceProcAddr"));
			m_rtxDispatcher.init(m_Context.getInstance());

			cmdBuffer.traceRaysKHR(&rgenRegion,&rmissRegion,&rchitRegion,&rcallRegion,
									getWindow(windowHandle).getWidth(), getWindow(windowHandle).getHeight(),1, m_rtxDispatcher);

		};
		recordCommandsToStream(cmdStreamHandle, submitFunction, nullptr);
    }

	void Core::recordComputeDispatchToCmdStream(
		CommandStreamHandle cmdStreamHandle,
		ComputePipelineHandle computePipeline,
		const uint32_t dispatchCount[3],
		const std::vector<DescriptorSetUsage>& descriptorSetUsages,
		const PushConstants& pushConstants) {

		auto submitFunction = [&](const vk::CommandBuffer& cmdBuffer) {

			const auto pipelineLayout = m_ComputePipelineManager->getVkPipelineLayout(computePipeline);

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_ComputePipelineManager->getVkPipeline(computePipeline));
			for (const auto& usage : descriptorSetUsages) {
				cmdBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eCompute,
					pipelineLayout,
					usage.setLocation,
					{ getDescriptorSet(usage.descriptorSet).vulkanHandle },
					usage.dynamicOffsets
				);
			}
			if (pushConstants.getSizePerDrawcall() > 0) {
				cmdBuffer.pushConstants(
					pipelineLayout,
					vk::ShaderStageFlagBits::eCompute,
					0,
					pushConstants.getSizePerDrawcall(),
					pushConstants.getData());
			}
			cmdBuffer.dispatch(dispatchCount[0], dispatchCount[1], dispatchCount[2]);
		};

		recordCommandsToStream(cmdStreamHandle, submitFunction, nullptr);
	}
	
	void Core::recordBeginDebugLabel(const CommandStreamHandle &cmdStream,
									 const std::string& label,
									 const std::array<float, 4>& color) {
	#ifdef VULKAN_DEBUG_LABELS
		static PFN_vkCmdBeginDebugUtilsLabelEXT beginDebugLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
				m_Context.getDevice().getProcAddr("vkCmdBeginDebugUtilsLabelEXT")
		);
		
		if (!beginDebugLabel) {
			return;
		}
		
		auto submitFunction = [&](const vk::CommandBuffer& cmdBuffer) {
			const vk::DebugUtilsLabelEXT debug (
					label.c_str(),
					color
			);
			
			beginDebugLabel(static_cast<VkCommandBuffer>(cmdBuffer), &(static_cast<const VkDebugUtilsLabelEXT&>(debug)));
		};

		recordCommandsToStream(cmdStream, submitFunction, nullptr);
	#endif
	}
	
	void Core::recordEndDebugLabel(const CommandStreamHandle &cmdStream) {
	#ifdef VULKAN_DEBUG_LABELS
		static PFN_vkCmdEndDebugUtilsLabelEXT endDebugLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
				m_Context.getDevice().getProcAddr("vkCmdEndDebugUtilsLabelEXT")
		);
		
		if (!endDebugLabel) {
			return;
		}
		
		auto submitFunction = [&](const vk::CommandBuffer& cmdBuffer) {
			endDebugLabel(static_cast<VkCommandBuffer>(cmdBuffer));
		};

		recordCommandsToStream(cmdStream, submitFunction, nullptr);
	#endif
	}
	
	void Core::recordComputeIndirectDispatchToCmdStream(
		const CommandStreamHandle               cmdStream,
		const ComputePipelineHandle             computePipeline,
		const vkcv::BufferHandle                buffer,
		const size_t                            bufferArgOffset,
		const std::vector<DescriptorSetUsage>&  descriptorSetUsages,
		const PushConstants&                    pushConstants) {

		auto submitFunction = [&](const vk::CommandBuffer& cmdBuffer) {

			const auto pipelineLayout = m_ComputePipelineManager->getVkPipelineLayout(computePipeline);

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_ComputePipelineManager->getVkPipeline(computePipeline));
			for (const auto& usage : descriptorSetUsages) {
				cmdBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eCompute,
					pipelineLayout,
					usage.setLocation,
					{ getDescriptorSet(usage.descriptorSet).vulkanHandle },
					usage.dynamicOffsets
				);
			}
			if (pushConstants.getSizePerDrawcall() > 0) {
				cmdBuffer.pushConstants(
					pipelineLayout,
					vk::ShaderStageFlagBits::eCompute,
					0,
					pushConstants.getSizePerDrawcall(),
					pushConstants.getData());
			}
			cmdBuffer.dispatchIndirect(m_BufferManager->getBuffer(buffer), bufferArgOffset);
		};

		recordCommandsToStream(cmdStream, submitFunction, nullptr);
	}

	void Core::endFrame(const WindowHandle& windowHandle) {

		SwapchainHandle swapchainHandle = m_WindowManager->getWindow(windowHandle).getSwapchainHandle();

		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}
		
		std::array<vk::Semaphore, 2> waitSemaphores{
			m_SyncResources.renderFinished,
			m_SyncResources.swapchainImageAcquired
		};

		const vk::SwapchainKHR& swapchain = m_SwapchainManager->getSwapchain(swapchainHandle).getSwapchain();
		const vk::PresentInfoKHR presentInfo(
			waitSemaphores,
			swapchain,
			m_currentSwapchainImageIndex
		);
		
		vk::Result result;
		
		try {
			result = m_Context.getDevice().getQueue(m_SwapchainManager->getSwapchain(swapchainHandle).getPresentQueueIndex(),0).presentKHR(presentInfo);
		} catch (const vk::OutOfDateKHRError& e) {
			result = vk::Result::eErrorOutOfDateKHR;
		} catch (const vk::DeviceLostError& e) {
			result = vk::Result::eErrorDeviceLost;
		}
		
		if ((result != vk::Result::eSuccess) &&
			(result != vk::Result::eSuboptimalKHR)) {
			vkcv_log(LogLevel::ERROR, "Swapchain presentation failed (%s)", vk::to_string(result).c_str());
		} else
		if (result == vk::Result::eSuboptimalKHR) {
			vkcv_log(LogLevel::WARNING, "Swapchain presentation is suboptimal");
			m_SwapchainManager->signalRecreation(swapchainHandle);
		}
	}
	
	void Core::recordAndSubmitCommandsImmediate(
		const SubmitInfo &submitInfo, 
		const RecordCommandFunction &record, 
		const FinishCommandFunction &finish)
	{
		const vk::Device& device = m_Context.getDevice();

		const vkcv::Queue		queue		= getQueueForSubmit(submitInfo.queueType, m_Context.getQueueManager());
		const vk::CommandPool	cmdPool		= chooseCmdPool(queue, m_CommandResources);
		const vk::CommandBuffer	cmdBuffer	= allocateCommandBuffer(device, cmdPool);

		beginCommandBuffer(cmdBuffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		record(cmdBuffer);
		cmdBuffer.end();
		
		vk::Fence waitFence = createFence(device);

		submitCommandBufferToQueue(queue.handle, cmdBuffer, waitFence, submitInfo.waitSemaphores, submitInfo.signalSemaphores);
		waitForFence(device, waitFence);
		
		device.destroyFence(waitFence);
		
		device.freeCommandBuffers(cmdPool, cmdBuffer);
		
		if (finish) {
			finish();
		}
	}

	CommandStreamHandle Core::createCommandStream(QueueType queueType) {
		const vkcv::Queue       queue   = getQueueForSubmit(queueType, m_Context.getQueueManager());
		const vk::CommandPool   cmdPool = chooseCmdPool(queue, m_CommandResources);

		return m_CommandStreamManager->createCommandStream(queue.handle, cmdPool);
	}

    void Core::recordCommandsToStream(
		const CommandStreamHandle   cmdStreamHandle,
		const RecordCommandFunction &record, 
		const FinishCommandFunction &finish) {

		m_CommandStreamManager->recordCommandsToStream(cmdStreamHandle, record);
		if (finish) {
			m_CommandStreamManager->addFinishCallbackToStream(cmdStreamHandle, finish);
		}
	}

	void Core::submitCommandStream(const CommandStreamHandle& handle) {
		std::vector<vk::Semaphore> waitSemaphores;
		// FIXME: add proper user controllable sync
		std::vector<vk::Semaphore> signalSemaphores = { m_SyncResources.renderFinished };
		m_CommandStreamManager->submitCommandStreamSynchronous(handle, waitSemaphores, signalSemaphores);
	}

	SamplerHandle Core::createSampler(SamplerFilterType magFilter, SamplerFilterType minFilter,
									  SamplerMipmapMode mipmapMode, SamplerAddressMode addressMode,
									  float mipLodBias, SamplerBorderColor borderColor) {
		return m_SamplerManager->createSampler(magFilter, minFilter, mipmapMode, addressMode, mipLodBias, borderColor);
	}

	Image Core::createImage(
		vk::Format      format,
		uint32_t        width,
		uint32_t        height,
		uint32_t        depth,
		bool            createMipChain,
		bool            supportStorage,
		bool            supportColorAttachment,
		Multisampling   multisampling)
	{

		uint32_t mipCount = 1;
		if (createMipChain) {
			mipCount = 1 + (uint32_t)std::floor(std::log2(std::max(width, std::max(height, depth))));
		}

		return Image::create(
			m_ImageManager.get(), 
			format,
			width,
			height,
			depth,
			mipCount,
			supportStorage,
			supportColorAttachment,
			multisampling);
	}

	WindowHandle Core::createWindow(
			const char *applicationName,
			uint32_t windowWidth,
			uint32_t windowHeight,
			bool resizeable) {

		WindowHandle windowHandle = m_WindowManager->createWindow(*m_SwapchainManager ,applicationName, windowWidth, windowHeight, resizeable);
		SwapchainHandle swapchainHandle = m_WindowManager->getWindow(windowHandle).getSwapchainHandle();
		setSwapchainImages( swapchainHandle );
		return windowHandle;
	}

	Window& Core::getWindow(const WindowHandle& handle) {
		return m_WindowManager->getWindow(handle);
	}

	uint32_t Core::getImageWidth(const ImageHandle& image)
	{
		return m_ImageManager->getImageWidth(image);
	}

	uint32_t Core::getImageHeight(const ImageHandle& image)
	{
		return m_ImageManager->getImageHeight(image);
	}
	
	vk::Format Core::getImageFormat(const ImageHandle& image) {
		return m_ImageManager->getImageFormat(image);
	}

	Swapchain& Core::getSwapchainOfCurrentWindow() {
		return m_SwapchainManager->getSwapchain(Window::getFocusedWindow().getSwapchainHandle());
	}

	Swapchain& Core::getSwapchain(const SwapchainHandle& handle) {
		return m_SwapchainManager->getSwapchain(handle);
	}

	Swapchain& Core::getSwapchain(const WindowHandle& handle) {
		SwapchainHandle swapchainHandle = m_WindowManager->getWindow(handle).getSwapchainHandle();
		return getSwapchain(swapchainHandle);
	}

	DescriptorSetLayoutHandle Core::createDescriptorSetLayout(const DescriptorBindings &bindings)
	{
	    return m_DescriptorManager->createDescriptorSetLayout(bindings);
	}

	DescriptorSetLayout Core::getDescriptorSetLayout(const DescriptorSetLayoutHandle handle) const
	{
	    return m_DescriptorManager->getDescriptorSetLayout(handle);
	}

	DescriptorSetHandle Core::createDescriptorSet(const DescriptorSetLayoutHandle &layoutHandle)
    {
        return m_DescriptorManager->createDescriptorSet(layoutHandle);
    }

	void Core::writeDescriptorSet(DescriptorSetHandle handle, const DescriptorWrites &writes) {
		m_DescriptorManager->writeDescriptorSet(
			handle,
			writes, 
			*m_ImageManager, 
			*m_BufferManager, 
			*m_SamplerManager);
	}

	DescriptorSet Core::getDescriptorSet(const DescriptorSetHandle handle) const {
		return m_DescriptorManager->getDescriptorSet(handle);
	}

	void Core::prepareSwapchainImageForPresent(const CommandStreamHandle& cmdStream) {
		auto swapchainHandle = ImageHandle::createSwapchainImageHandle();
		recordCommandsToStream(cmdStream, [swapchainHandle, this](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordImageLayoutTransition(swapchainHandle, vk::ImageLayout::ePresentSrcKHR, cmdBuffer);
		}, nullptr);
	}

	void Core::prepareImageForSampling(const CommandStreamHandle& cmdStream, const ImageHandle& image) {
		recordCommandsToStream(cmdStream, [image, this](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordImageLayoutTransition(image, vk::ImageLayout::eShaderReadOnlyOptimal, cmdBuffer);
		}, nullptr);
	}

	void Core::prepareImageForStorage(const CommandStreamHandle& cmdStream, const ImageHandle& image) {
		recordCommandsToStream(cmdStream, [image, this](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordImageLayoutTransition(image, vk::ImageLayout::eGeneral, cmdBuffer);
		}, nullptr);
	}

	void Core::prepareImageForAttachmentManually(const vk::CommandBuffer& cmdBuffer, const ImageHandle& image) {
		transitionRendertargetsToAttachmentLayout({ image }, *m_ImageManager, cmdBuffer);
	}

	void Core::updateImageLayoutManual(const vkcv::ImageHandle& image, const vk::ImageLayout layout) {
		m_ImageManager->updateImageLayoutManual(image, layout);
	}

	void Core::recordImageMemoryBarrier(const CommandStreamHandle& cmdStream, const ImageHandle& image) {
		recordCommandsToStream(cmdStream, [image, this](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordImageMemoryBarrier(image, cmdBuffer);
		}, nullptr);
	}

	void Core::recordBufferMemoryBarrier(const CommandStreamHandle& cmdStream, const BufferHandle& buffer) {
		recordCommandsToStream(cmdStream, [buffer, this](const vk::CommandBuffer cmdBuffer) {
			m_BufferManager->recordBufferMemoryBarrier(buffer, cmdBuffer);
		}, nullptr);
	}
	
	void Core::resolveMSAAImage(const CommandStreamHandle& cmdStream, const ImageHandle& src, const ImageHandle& dst) {
		recordCommandsToStream(cmdStream, [src, dst, this](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordMSAAResolve(cmdBuffer, src, dst);
		}, nullptr);
	}

	vk::ImageView Core::getSwapchainImageView() const {
    	return m_ImageManager->getVulkanImageView(vkcv::ImageHandle::createSwapchainImageHandle());
    }
    
    void Core::recordMemoryBarrier(const CommandStreamHandle& cmdStream) {
		recordCommandsToStream(cmdStream, [](const vk::CommandBuffer cmdBuffer) {
			vk::MemoryBarrier barrier (
					vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead,
					vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead
			);
			
			cmdBuffer.pipelineBarrier(
					vk::PipelineStageFlagBits::eAllCommands,
					vk::PipelineStageFlagBits::eAllCommands,
					vk::DependencyFlags(),
					1, &barrier,
					0, nullptr,
					0, nullptr
			);
		}, nullptr);
	}
	
	void Core::recordBlitImage(const CommandStreamHandle& cmdStream, const ImageHandle& src, const ImageHandle& dst,
							   SamplerFilterType filterType) {
		recordCommandsToStream(cmdStream, [&](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordImageLayoutTransition(
					src, vk::ImageLayout::eTransferSrcOptimal, cmdBuffer
			);
			
			m_ImageManager->recordImageLayoutTransition(
					dst, vk::ImageLayout::eTransferDstOptimal, cmdBuffer
			);
			
			const std::array<vk::Offset3D, 2> srcOffsets = {
					vk::Offset3D(0, 0, 0),
					vk::Offset3D(
							m_ImageManager->getImageWidth(src),
							m_ImageManager->getImageHeight(src),
							1
					)
			};
			
			const std::array<vk::Offset3D, 2> dstOffsets = {
					vk::Offset3D(0, 0, 0),
					vk::Offset3D(
							m_ImageManager->getImageWidth(dst),
							m_ImageManager->getImageHeight(dst),
							1
					)
			};
			
			const bool srcDepth = isDepthFormat(m_ImageManager->getImageFormat(src));
			const bool dstDepth = isDepthFormat(m_ImageManager->getImageFormat(dst));
			
			const vk::ImageBlit blit = vk::ImageBlit(
					vk::ImageSubresourceLayers(
							srcDepth?
							vk::ImageAspectFlagBits::eDepth :
							vk::ImageAspectFlagBits::eColor,
							0, 0, 1
					),
					srcOffsets,
					vk::ImageSubresourceLayers(
							dstDepth?
							vk::ImageAspectFlagBits::eDepth :
							vk::ImageAspectFlagBits::eColor,
							0, 0, 1
					),
					dstOffsets
			);
			
			cmdBuffer.blitImage(
					m_ImageManager->getVulkanImage(src),
					vk::ImageLayout::eTransferSrcOptimal,
					m_ImageManager->getVulkanImage(dst),
					vk::ImageLayout::eTransferDstOptimal,
					1,
					&blit,
					filterType == SamplerFilterType::LINEAR?
					vk::Filter::eLinear :
					vk::Filter::eNearest
			);
		}, nullptr);
	}

	void Core::setSwapchainImages( SwapchainHandle handle ) {
		Swapchain swapchain = m_SwapchainManager->getSwapchain(handle);
		const auto swapchainImages = m_SwapchainManager->getSwapchainImages(handle);
		const auto swapchainImageViews = m_SwapchainManager->createSwapchainImageViews(handle);

		m_ImageManager->setSwapchainImages(
				swapchainImages,
				swapchainImageViews,
				swapchain.getExtent().width,
				swapchain.getExtent().height,
				swapchain.getFormat()
		);
	}
	
	static void setDebugObjectLabel(const vk::Device& device, const vk::ObjectType& type,
									uint64_t handle, const std::string& label) {
#ifndef VULKAN_DEBUG_LABELS
		static PFN_vkSetDebugUtilsObjectNameEXT setDebugLabel = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(
				device.getProcAddr("vkSetDebugUtilsObjectNameEXT")
		);
		
		if (!setDebugLabel) {
			return;
		}
		
		const vk::DebugUtilsObjectNameInfoEXT debug (
				type,
				handle,
				label.c_str()
		);
		
		setDebugLabel(static_cast<VkDevice>(device), &(static_cast<const VkDebugUtilsObjectNameInfoEXT&>(debug)));
#endif
	}
	
	void Core::setDebugLabel(const BufferHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}
		
		setDebugObjectLabel(
				m_Context.getDevice(),
				vk::ObjectType::eBuffer,
				uint64_t(static_cast<VkBuffer>(
						m_BufferManager->getBuffer(handle)
				)),
				label
		);
	}
	
	void Core::setDebugLabel(const PassHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}
		
		setDebugObjectLabel(
				m_Context.getDevice(),
				vk::ObjectType::eRenderPass,
				uint64_t(static_cast<VkRenderPass>(
						m_PassManager->getVkPass(handle)
				)),
				label
		);
	}
	
	void Core::setDebugLabel(const GraphicsPipelineHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}
		
		setDebugObjectLabel(
				m_Context.getDevice(),
				vk::ObjectType::ePipeline,
				uint64_t(static_cast<VkPipeline>(
						m_PipelineManager->getVkPipeline(handle)
				)),
				label
		);
	}
	
	void Core::setDebugLabel(const ComputePipelineHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}
		
		setDebugObjectLabel(
				m_Context.getDevice(),
				vk::ObjectType::ePipeline,
				uint64_t(static_cast<VkPipeline>(
								 m_ComputePipelineManager->getVkPipeline(handle)
						 )),
				label
		);
	}
	
	void Core::setDebugLabel(const DescriptorSetHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}
		
		setDebugObjectLabel(
				m_Context.getDevice(),
				vk::ObjectType::eDescriptorSet,
				uint64_t(static_cast<VkDescriptorSet>(
						m_DescriptorManager->getDescriptorSet(handle).vulkanHandle
				)),
				label
		);
	}
	
	void Core::setDebugLabel(const SamplerHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}
		
		setDebugObjectLabel(
				m_Context.getDevice(),
				vk::ObjectType::eSampler,
				uint64_t(static_cast<VkSampler>(
						m_SamplerManager->getVulkanSampler(handle)
				)),
				label
		);
	}
	
	void Core::setDebugLabel(const ImageHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		} else
		if (handle.isSwapchainImage()) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to swapchain image");
			return;
		}
		
		setDebugObjectLabel(
				m_Context.getDevice(),
				vk::ObjectType::eImage,
				uint64_t(static_cast<VkImage>(
						m_ImageManager->getVulkanImage(handle)
				)),
				label
		);
	}
	
	void Core::setDebugLabel(const CommandStreamHandle &handle, const std::string &label) {
		if (!handle) {
			vkcv_log(LogLevel::WARNING, "Can't set debug label to invalid handle");
			return;
		}
		
		setDebugObjectLabel(
				m_Context.getDevice(),
				vk::ObjectType::eCommandBuffer,
				uint64_t(static_cast<VkCommandBuffer>(
						m_CommandStreamManager->getStreamCommandBuffer(handle)
				)),
				label
		);
	}
}
