/**
 * @authors Artur Wasmut
 * @file src/vkcv/Core.cpp
 * @brief Handling of global states regarding dependencies
 */

#include <GLFW/glfw3.h>

#include "vkcv/Core.hpp"
#include "PassManager.hpp"
#include "PipelineManager.hpp"
#include "vkcv/BufferManager.hpp"
#include "SamplerManager.hpp"
#include "ImageManager.hpp"
#include "DescriptorManager.hpp"
#include "ImageLayoutTransitions.hpp"
#include "vkcv/CommandStreamManager.hpp"
#include <cmath>
#include "vkcv/Logger.hpp"

namespace vkcv
{
	
	static std::vector<vk::ImageView> createSwapchainImageViews( Context &context, const std::vector<vk::Image>& images,
																 vk::Format format){
		std::vector<vk::ImageView> imageViews;
		imageViews.reserve( images.size() );
		//here can be swizzled with vk::ComponentSwizzle if needed
		vk::ComponentMapping componentMapping(
				vk::ComponentSwizzle::eR,
				vk::ComponentSwizzle::eG,
				vk::ComponentSwizzle::eB,
				vk::ComponentSwizzle::eA );
		
		vk::ImageSubresourceRange subResourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 );
		
		for ( auto image : images )
		{
			vk::ImageViewCreateInfo imageViewCreateInfo(
					vk::ImageViewCreateFlags(),
					image,
					vk::ImageViewType::e2D,
					format,
					componentMapping,
					subResourceRange);
			
			imageViews.push_back(context.getDevice().createImageView(imageViewCreateInfo));
		}
		
		return imageViews;
	}

    Core Core::create(Window &window,
                      const char *applicationName,
                      uint32_t applicationVersion,
                      std::vector<vk::QueueFlagBits> queueFlags,
                      std::vector<const char *> instanceExtensions,
                      std::vector<const char *> deviceExtensions)
    {
        Context context = Context::create(
        		applicationName, applicationVersion,
        		queueFlags,
        		instanceExtensions,
        		deviceExtensions
		);

        Swapchain swapChain = Swapchain::create(window, context);
	
		const auto swapchainImages = context.getDevice().getSwapchainImagesKHR(swapChain.getSwapchain());
		const auto swapchainImageViews = createSwapchainImageViews( context, swapchainImages, swapChain.getFormat());

        const auto& queueManager = context.getQueueManager();
        
		const std::unordered_set<int>	queueFamilySet			= generateQueueFamilyIndexSet(queueManager);
		const auto						commandResources		= createCommandResources(context.getDevice(), queueFamilySet);
		const auto						defaultSyncResources	= createSyncResources(context.getDevice());

        return Core(std::move(context) , window, swapChain, swapchainImageViews, commandResources, defaultSyncResources);
    }

    const Context &Core::getContext() const
    {
        return m_Context;
    }
    
    const Swapchain& Core::getSwapchain() const {
    	return m_swapchain;
    }

    Core::Core(Context &&context, Window &window, const Swapchain& swapChain,  std::vector<vk::ImageView> swapchainImageViews,
        const CommandResources& commandResources, const SyncResources& syncResources) noexcept :
            m_Context(std::move(context)),
            m_window(window),
            m_swapchain(swapChain),
            m_PassManager{std::make_unique<PassManager>(m_Context.m_Device)},
            m_PipelineManager{std::make_unique<PipelineManager>(m_Context.m_Device)},
            m_DescriptorManager(std::make_unique<DescriptorManager>(m_Context.m_Device)),
            m_BufferManager{std::unique_ptr<BufferManager>(new BufferManager())},
            m_SamplerManager(std::unique_ptr<SamplerManager>(new SamplerManager(m_Context.m_Device))),
            m_ImageManager{std::unique_ptr<ImageManager>(new ImageManager(*m_BufferManager))},
            m_CommandStreamManager{std::unique_ptr<CommandStreamManager>(new CommandStreamManager)},
            m_CommandResources(commandResources),
            m_SyncResources(syncResources)
	{
		m_BufferManager->m_core = this;
		m_BufferManager->init();
		m_CommandStreamManager->init(this);

		m_ImageManager->m_core = this;
		
		e_resizeHandle = m_window.e_resize.add( [&](int width, int height) {
			m_swapchain.signalSwapchainRecreation();
		});

		const auto swapchainImages = m_Context.getDevice().getSwapchainImagesKHR(m_swapchain.getSwapchain());
		m_ImageManager->setSwapchainImages(
			swapchainImages, 
			swapchainImageViews, 
			swapChain.getExtent().width,
			swapChain.getExtent().height,
			swapChain.getFormat());
	}

	Core::~Core() noexcept {
    	m_window.e_resize.remove(e_resizeHandle);
    	
		m_Context.getDevice().waitIdle();

		destroyCommandResources(m_Context.getDevice(), m_CommandResources);
		destroySyncResources(m_Context.getDevice(), m_SyncResources);

		m_Context.m_Device.destroySwapchainKHR(m_swapchain.getSwapchain());
		m_Context.m_Instance.destroySurfaceKHR(m_swapchain.getSurface());
	}

    PipelineHandle Core::createGraphicsPipeline(const PipelineConfig &config)
    {
        return m_PipelineManager->createPipeline(config, *m_PassManager);
    }

    PipelineHandle Core::createComputePipeline(
        const ShaderProgram &shaderProgram, 
        const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts)
    {
        return m_PipelineManager->createComputePipeline(shaderProgram, descriptorSetLayouts);
    }

    PassHandle Core::createPass(const PassConfig &config)
    {
        return m_PassManager->createPass(config);
    }

	Result Core::acquireSwapchainImage() {
    	uint32_t imageIndex;
    	vk::Result result;
    	
		try {
			result = m_Context.getDevice().acquireNextImageKHR(
					m_swapchain.getSwapchain(),
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
			m_swapchain.signalSwapchainRecreation();
		}
		
		m_currentSwapchainImageIndex = imageIndex;
		return Result::SUCCESS;
	}

	bool Core::beginFrame(uint32_t& width, uint32_t& height) {
		if (m_swapchain.shouldUpdateSwapchain()) {
			m_Context.getDevice().waitIdle();

			m_swapchain.updateSwapchain(m_Context, m_window);
			
			if (!m_swapchain.getSwapchain()) {
				return false;
			}
			
			const auto swapchainImages = m_Context.getDevice().getSwapchainImagesKHR(m_swapchain.getSwapchain());
			const auto swapchainViews = createSwapchainImageViews(m_Context, swapchainImages, m_swapchain.getFormat());
			
			const auto& extent = m_swapchain.getExtent();

			m_ImageManager->setSwapchainImages(
					swapchainImages,
					swapchainViews,
					extent.width, extent.height,
					m_swapchain.getFormat()
			);
		}
		
		const auto& extent = m_swapchain.getExtent();
		
		width = extent.width;
		height = extent.height;
		
		if ((width < MIN_SWAPCHAIN_SIZE) || (height < MIN_SWAPCHAIN_SIZE)) {
			return false;
		}
		
    	if (acquireSwapchainImage() != Result::SUCCESS) {
			vkcv_log(LogLevel::ERROR, "Acquire failed");
    		
    		m_currentSwapchainImageIndex = std::numeric_limits<uint32_t>::max();
    	}
		
		m_Context.getDevice().waitIdle(); // TODO: this is a sin against graphics programming, but its getting late - Alex
		
		m_ImageManager->setCurrentSwapchainImageIndex(m_currentSwapchainImageIndex);

		return (m_currentSwapchainImageIndex != std::numeric_limits<uint32_t>::max());
	}

	void Core::recordDrawcallsToCmdStream(
		const CommandStreamHandle       cmdStreamHandle,
		const PassHandle                renderpassHandle, 
		const PipelineHandle            pipelineHandle, 
        const PushConstants             &pushConstants,
        const std::vector<DrawcallInfo> &drawcalls,
		const std::vector<ImageHandle>  &renderTargets) {

		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}

		uint32_t width;
		uint32_t height;
		if (renderTargets.size() > 0) {
			const vkcv::ImageHandle firstImage = renderTargets[0];
			if (firstImage.isSwapchainImage()) {
				const auto& swapchainExtent = m_swapchain.getExtent();
				width = swapchainExtent.width;
				height = swapchainExtent.height;
			}
			else {
				width = m_ImageManager->getImageWidth(firstImage);
				height = m_ImageManager->getImageHeight(firstImage);
			}
		}
		else {
			width = 1;
			height = 1;
		}
		// TODO: validate that width/height match for all attachments

		const vk::RenderPass renderpass = m_PassManager->getVkPass(renderpassHandle);
		const PassConfig passConfig = m_PassManager->getPassConfig(renderpassHandle);

		const vk::Pipeline pipeline		= m_PipelineManager->getVkPipeline(pipelineHandle);
		const vk::PipelineLayout pipelineLayout = m_PipelineManager->getVkPipelineLayout(pipelineHandle);
		const vk::Rect2D renderArea(vk::Offset2D(0, 0), vk::Extent2D(width, height));

		std::vector<vk::ImageView> attachmentsViews;
		for (const ImageHandle& handle : renderTargets) {
			vk::ImageView targetHandle;
			const auto cmdBuffer = m_CommandStreamManager->getStreamCommandBuffer(cmdStreamHandle);

			targetHandle = m_ImageManager->getVulkanImageView(handle);
			const bool isDepthImage = isDepthFormat(m_ImageManager->getImageFormat(handle));
			const vk::ImageLayout targetLayout = 
				isDepthImage ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal;
			m_ImageManager->recordImageLayoutTransition(handle, targetLayout, cmdBuffer);
			attachmentsViews.push_back(targetHandle);
		}
		
        const vk::FramebufferCreateInfo createInfo(
            {},
            renderpass,
            static_cast<uint32_t>(attachmentsViews.size()),
            attachmentsViews.data(),
            width,
            height,
            1
		);
		
		vk::Framebuffer framebuffer = m_Context.m_Device.createFramebuffer(createInfo);
        
        if (!framebuffer) {
			vkcv_log(LogLevel::ERROR, "Failed to create temporary framebuffer");
            return;
        }

        vk::Viewport dynamicViewport(
        		0.0f, 0.0f,
            	static_cast<float>(width), static_cast<float>(height),
            0.0f, 1.0f
		);

        vk::Rect2D dynamicScissor({0, 0}, {width, height});

		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Graphics;
		submitInfo.signalSemaphores = { m_SyncResources.renderFinished };

		auto submitFunction = [&](const vk::CommandBuffer& cmdBuffer) {
            std::vector<vk::ClearValue> clearValues;

            for (const auto& attachment : passConfig.attachments) {
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

            const vk::RenderPassBeginInfo beginInfo(renderpass, framebuffer, renderArea, clearValues.size(), clearValues.data());
            const vk::SubpassContents subpassContents = {};
            cmdBuffer.beginRenderPass(beginInfo, subpassContents, {});

            cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline, {});

            const PipelineConfig &pipeConfig = m_PipelineManager->getPipelineConfig(pipelineHandle);
            if (pipeConfig.m_UseDynamicViewport) {
                cmdBuffer.setViewport(0, 1, &dynamicViewport);
                cmdBuffer.setScissor(0, 1, &dynamicScissor);
            }

            for (size_t i = 0; i < drawcalls.size(); i++) {
                recordDrawcall(drawcalls[i], cmdBuffer, pipelineLayout, pushConstants, i);
            }

            cmdBuffer.endRenderPass();
        };

        auto finishFunction = [framebuffer, this]()
        {
            m_Context.m_Device.destroy(framebuffer);
        };

		recordCommandsToStream(cmdStreamHandle, submitFunction, finishFunction);
	}

	void Core::recordComputeDispatchToCmdStream(
		CommandStreamHandle cmdStreamHandle,
		PipelineHandle computePipeline,
		const uint32_t dispatchCount[3],
		const std::vector<DescriptorSetUsage>& descriptorSetUsages,
		const PushConstants& pushConstants) {

		auto submitFunction = [&](const vk::CommandBuffer& cmdBuffer) {

			const auto pipelineLayout = m_PipelineManager->getVkPipelineLayout(computePipeline);

			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_PipelineManager->getVkPipeline(computePipeline));
			for (const auto& usage : descriptorSetUsages) {
				cmdBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eCompute,
					pipelineLayout,
					usage.setLocation,
					{ usage.vulkanHandle },
					{});
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

	void Core::endFrame() {
		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}
  
		const auto swapchainImages = m_Context.getDevice().getSwapchainImagesKHR(m_swapchain.getSwapchain());

		const auto& queueManager = m_Context.getQueueManager();
		std::array<vk::Semaphore, 2> waitSemaphores{
			m_SyncResources.renderFinished,
			m_SyncResources.swapchainImageAcquired
		};

		const vk::SwapchainKHR& swapchain = m_swapchain.getSwapchain();
		const vk::PresentInfoKHR presentInfo(
			waitSemaphores,
			swapchain,
			m_currentSwapchainImageIndex
		);
		
		vk::Result result;
		
		try {
			result = queueManager.getPresentQueue().handle.presentKHR(presentInfo);
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
			m_swapchain.signalSwapchainRecreation();
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

	void Core::submitCommandStream(const CommandStreamHandle handle) {
		std::vector<vk::Semaphore> waitSemaphores;
		// FIXME: add proper user controllable sync
		std::vector<vk::Semaphore> signalSemaphores = { m_SyncResources.renderFinished };
		m_CommandStreamManager->submitCommandStreamSynchronous(handle, waitSemaphores, signalSemaphores);
	}

	SamplerHandle Core::createSampler(SamplerFilterType magFilter, SamplerFilterType minFilter,
									  SamplerMipmapMode mipmapMode, SamplerAddressMode addressMode) {
		return m_SamplerManager->createSampler(magFilter, minFilter, mipmapMode, addressMode);
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

    DescriptorSetHandle Core::createDescriptorSet(const std::vector<DescriptorBinding>& bindings)
    {
        return m_DescriptorManager->createDescriptorSet(bindings);
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

	void Core::prepareSwapchainImageForPresent(const CommandStreamHandle cmdStream) {
		auto swapchainHandle = ImageHandle::createSwapchainImageHandle();
		recordCommandsToStream(cmdStream, [swapchainHandle, this](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordImageLayoutTransition(swapchainHandle, vk::ImageLayout::ePresentSrcKHR, cmdBuffer);
		}, nullptr);
	}

	void Core::prepareImageForSampling(const CommandStreamHandle cmdStream, const ImageHandle image) {
		recordCommandsToStream(cmdStream, [image, this](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordImageLayoutTransition(image, vk::ImageLayout::eShaderReadOnlyOptimal, cmdBuffer);
		}, nullptr);
	}

	void Core::prepareImageForStorage(const CommandStreamHandle cmdStream, const ImageHandle image) {
		recordCommandsToStream(cmdStream, [image, this](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordImageLayoutTransition(image, vk::ImageLayout::eGeneral, cmdBuffer);
		}, nullptr);
	}

	void Core::recordImageMemoryBarrier(const CommandStreamHandle cmdStream, const ImageHandle image) {
		recordCommandsToStream(cmdStream, [image, this](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordImageMemoryBarrier(image, cmdBuffer);
		}, nullptr);
	}

	void Core::recordBufferMemoryBarrier(const CommandStreamHandle cmdStream, const BufferHandle buffer) {
		recordCommandsToStream(cmdStream, [buffer, this](const vk::CommandBuffer cmdBuffer) {
			m_BufferManager->recordBufferMemoryBarrier(buffer, cmdBuffer);
		}, nullptr);
	}
	
	void Core::resolveMSAAImage(CommandStreamHandle cmdStream, ImageHandle src, ImageHandle dst) {
		recordCommandsToStream(cmdStream, [src, dst, this](const vk::CommandBuffer cmdBuffer) {
			m_ImageManager->recordMSAAResolve(cmdBuffer, src, dst);
		}, nullptr);
	}

	vk::ImageView Core::getSwapchainImageView() const {
    	return m_ImageManager->getVulkanImageView(vkcv::ImageHandle::createSwapchainImageHandle());
    }
	
}
