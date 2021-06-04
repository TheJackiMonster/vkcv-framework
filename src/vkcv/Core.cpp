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

namespace vkcv
{

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

        SwapChain swapChain = SwapChain::create(window, context);

        std::vector<vk::ImageView> imageViews;
        imageViews = createImageViews( context, swapChain);

        const auto& queueManager = context.getQueueManager();
        
		const int						graphicQueueFamilyIndex	= queueManager.getGraphicsQueues()[0].familyIndex;
		const std::unordered_set<int>	queueFamilySet			= generateQueueFamilyIndexSet(queueManager);
		const auto						commandResources		= createCommandResources(context.getDevice(), queueFamilySet);
		const auto						defaultSyncResources	= createSyncResources(context.getDevice());

        return Core(std::move(context) , window, swapChain, imageViews, commandResources, defaultSyncResources);
    }

    const Context &Core::getContext() const
    {
        return m_Context;
    }

	Core::Core(Context &&context, Window &window, const SwapChain& swapChain,  std::vector<vk::ImageView> imageViews,
		const CommandResources& commandResources, const SyncResources& syncResources) noexcept :
            m_Context(std::move(context)),
            m_window(window),
            m_swapchain(swapChain),
            m_swapchainImageViews(imageViews),
            m_PassManager{std::make_unique<PassManager>(m_Context.m_Device)},
            m_PipelineManager{std::make_unique<PipelineManager>(m_Context.m_Device)},
            m_DescriptorManager(std::make_unique<DescriptorManager>(m_Context.m_Device)),
			m_BufferManager{std::unique_ptr<BufferManager>(new BufferManager())},
			m_SamplerManager(std::unique_ptr<SamplerManager>(new SamplerManager(m_Context.m_Device))),
			m_ImageManager{std::unique_ptr<ImageManager>(new ImageManager(*m_BufferManager))},
            m_CommandResources(commandResources),
            m_SyncResources(syncResources)
	{
    	m_BufferManager->m_core = this;
    	m_BufferManager->init();
    	
    	m_ImageManager->m_core = this;

        e_resizeHandle = window.e_resize.add( [&](int width, int height) {
        	m_swapchain.recreateSwapchain();
        });
	}

	Core::~Core() noexcept {
		m_Context.getDevice().waitIdle();
		for (auto image : m_swapchainImageViews) {
			m_Context.m_Device.destroyImageView(image);
		}

		destroyCommandResources(m_Context.getDevice(), m_CommandResources);
		destroySyncResources(m_Context.getDevice(), m_SyncResources);

		m_Context.m_Device.destroySwapchainKHR(m_swapchain.getSwapchain());
		m_Context.m_Instance.destroySurfaceKHR(m_swapchain.getSurface());
	}

    PipelineHandle Core::createGraphicsPipeline(const PipelineConfig &config)
    {
        return m_PipelineManager->createPipeline(config, *m_PassManager);
    }


    PassHandle Core::createPass(const PassConfig &config)
    {
        return m_PassManager->createPass(config);
    }

	Result Core::acquireSwapchainImage() {
    	uint32_t imageIndex;
    	
		const auto& acquireResult = m_Context.getDevice().acquireNextImageKHR(
			m_swapchain.getSwapchain(), 
			std::numeric_limits<uint64_t>::max(), 
			m_SyncResources.swapchainImageAcquired,
			nullptr, 
			&imageIndex, {}
		);
		
		if (acquireResult != vk::Result::eSuccess) {
			std::cerr << vk::to_string(acquireResult) << std::endl;
			return Result::ERROR;
		}
		
		m_currentSwapchainImageIndex = imageIndex;
		return Result::SUCCESS;
	}

	void Core::beginFrame() {
		if (m_swapchain.shouldUpdateSwapchain()) {
			m_Context.getDevice().waitIdle();
			
			for (auto image : m_swapchainImageViews)
				m_Context.m_Device.destroyImageView(image);
			
			m_swapchain.updateSwapchain(m_Context, m_window);
			m_swapchainImageViews = createImageViews(m_Context, m_swapchain);
		}
		
    	if (acquireSwapchainImage() != Result::SUCCESS) {
    		std::cerr << "Acquire failed!" << std::endl;
    		
    		m_currentSwapchainImageIndex = std::numeric_limits<uint32_t>::max();
    	}
		
		m_Context.getDevice().waitIdle(); // TODO: this is a sin against graphics programming, but its getting late - Alex
	}

	void Core::renderMesh(
		const PassHandle				renderpassHandle, 
		const PipelineHandle			pipelineHandle, 
		const size_t					pushConstantSize, 
		const void						*pushConstantData,
		const Mesh						&mesh,
		const vkcv::ResourcesHandle		resourceHandle,
		const size_t					resourceDescriptorSetIndex,
		const std::vector<ImageHandle>&	renderTargets) {

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

		const vk::ImageView swapchainImageView = m_swapchainImageViews[m_currentSwapchainImageIndex];

		std::vector<vk::ImageView> attachmentsViews;
		for (const ImageHandle handle : renderTargets) {
			vk::ImageView targetHandle;
			if (handle.isSwapchainImage()) {
				targetHandle = m_swapchainImageViews[m_currentSwapchainImageIndex];
			}
			else {
				targetHandle = m_ImageManager->getVulkanImageView(handle);
			}
			attachmentsViews.push_back(targetHandle);
		}
		
		vk::Framebuffer framebuffer = nullptr;
        const vk::FramebufferCreateInfo createInfo(
            {},
            renderpass,
            static_cast<uint32_t>(attachmentsViews.size()),
            attachmentsViews.data(),
            width,
            height,
            1);
        if(m_Context.m_Device.createFramebuffer(&createInfo, nullptr, &framebuffer) != vk::Result::eSuccess)
        {
            std::cout << "FAILED TO CREATE TEMPORARY FRAMEBUFFER!" << std::endl;
            return;
        }

        vk::Viewport dynamicViewport(0.0f, 0.0f,
            static_cast<float>(width), static_cast<float>(height),
            0.0f, 1.0f);

        vk::Rect2D dynamicScissor({0, 0}, {width, height});

		auto &bufferManager = m_BufferManager;

		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Graphics;
		submitInfo.signalSemaphores = { m_SyncResources.renderFinished };

		auto submitFunction = [&](const vk::CommandBuffer& cmdBuffer) {
            std::vector<vk::ClearValue> clearValues;

            for (const auto& attachment : passConfig.attachments) {
                if (attachment.load_operation == AttachmentOperation::CLEAR) {
                    float clear = 0.0f;

                    if (attachment.layout_final == AttachmentLayout::DEPTH_STENCIL_ATTACHMENT) {
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
            if(pipeConfig.m_UseDynamicViewport)
            {
                cmdBuffer.setViewport(0, 1, &dynamicViewport);
                cmdBuffer.setScissor(0, 1, &dynamicScissor);
            }

            for (uint32_t i = 0; i < mesh.vertexBufferBindings.size(); i++) {
                const auto &vertexBinding = mesh.vertexBufferBindings[i];
                const auto vertexBuffer = bufferManager->getBuffer(vertexBinding.buffer);
                cmdBuffer.bindVertexBuffers(i, (vertexBuffer), (vertexBinding.offset));
            }

            if (resourceHandle) {
                const vk::DescriptorSet descriptorSet = m_DescriptorManager->getDescriptorSet(resourceHandle, resourceDescriptorSetIndex);
                cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, descriptorSet, nullptr);
            }

            const vk::Buffer indexBuffer = m_BufferManager->getBuffer(mesh.indexBuffer);

            cmdBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);	//FIXME: choose proper size
            cmdBuffer.pushConstants(pipelineLayout, vk::ShaderStageFlagBits::eAll, 0, pushConstantSize, pushConstantData);
            cmdBuffer.drawIndexed(mesh.indexCount, 1, 0, 0, {});
            cmdBuffer.endRenderPass();
        };

        auto finishFunction = [&]()
        {
            m_Context.m_Device.destroy(framebuffer);
        };

		submitCommands(submitInfo, submitFunction, finishFunction);
	}

	void Core::endFrame() {
		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}
  
		const auto swapchainImages = m_Context.getDevice().getSwapchainImagesKHR(m_swapchain.getSwapchain());

		const auto& queueManager = m_Context.getQueueManager();
		std::array<vk::Semaphore, 2> waitSemaphores{ 
			m_SyncResources.renderFinished, 
			m_SyncResources.swapchainImageAcquired };

		vk::Result presentResult;
		const vk::SwapchainKHR& swapchain = m_swapchain.getSwapchain();
		const vk::PresentInfoKHR presentInfo(
			waitSemaphores,
			swapchain,
			m_currentSwapchainImageIndex, 
			presentResult);
			queueManager.getPresentQueue().handle.presentKHR(presentInfo);
		if (presentResult != vk::Result::eSuccess) {
			std::cout << "Error: swapchain present failed" << std::endl;
		}
	}

	vk::Format Core::getSwapchainImageFormat() {
		return m_swapchain.getSwapchainFormat();
	}
	
	void Core::submitCommands(const SubmitInfo &submitInfo, const RecordCommandFunction& record, const FinishCommandFunction& finish)
	{
		const vk::Device& device = m_Context.getDevice();

		const vkcv::Queue		queue		= getQueueForSubmit(submitInfo.queueType, m_Context.getQueueManager());
		const vk::CommandPool	cmdPool		= chooseCmdPool(queue, m_CommandResources);
		const vk::CommandBuffer	cmdBuffer	= allocateCommandBuffer(device, cmdPool);

		beginCommandBuffer(cmdBuffer, vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
		record(cmdBuffer);
		cmdBuffer.end();

		const vk::Fence waitFence = createFence(device);
		submitCommandBufferToQueue(queue.handle, cmdBuffer, waitFence, submitInfo.waitSemaphores, submitInfo.signalSemaphores);
		waitForFence(device, waitFence);
		device.destroyFence(waitFence);
		
		device.freeCommandBuffers(cmdPool, cmdBuffer);
		
		if (finish) {
			finish();
		}
	}
	
	SamplerHandle Core::createSampler(SamplerFilterType magFilter, SamplerFilterType minFilter,
									  SamplerMipmapMode mipmapMode, SamplerAddressMode addressMode) {
    	return m_SamplerManager->createSampler(magFilter, minFilter, mipmapMode, addressMode);
    }
    
	Image Core::createImage(vk::Format format, uint32_t width, uint32_t height, uint32_t depth)
	{
    	return Image::create(m_ImageManager.get(), format, width, height, depth);
	}

    ResourcesHandle Core::createResourceDescription(const std::vector<DescriptorSetConfig> &descriptorSets)
    {
        return m_DescriptorManager->createResourceDescription(descriptorSets);
    }

	void Core::writeResourceDescription(ResourcesHandle handle, size_t setIndex, const DescriptorWrites &writes) {
		m_DescriptorManager->writeResourceDescription(
			handle, 
			setIndex, 
			writes, 
			*m_ImageManager, 
			*m_BufferManager, 
			*m_SamplerManager);
	}

	vk::DescriptorSetLayout Core::getDescriptorSetLayout(ResourcesHandle handle, size_t setIndex) {
		return m_DescriptorManager->getDescriptorSetLayout(handle, setIndex);
	}

    std::vector<vk::ImageView> Core::createImageViews( Context &context, SwapChain& swapChain){
        std::vector<vk::ImageView> imageViews;
        std::vector<vk::Image> swapChainImages = context.getDevice().getSwapchainImagesKHR(swapChain.getSwapchain());
        imageViews.reserve( swapChainImages.size() );
        //here can be swizzled with vk::ComponentSwizzle if needed
        vk::ComponentMapping componentMapping(
                vk::ComponentSwizzle::eR,
                vk::ComponentSwizzle::eG,
                vk::ComponentSwizzle::eB,
                vk::ComponentSwizzle::eA );

        vk::ImageSubresourceRange subResourceRange( vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 );

        for ( auto image : swapChainImages )
        {
            vk::ImageViewCreateInfo imageViewCreateInfo(
                    vk::ImageViewCreateFlags(),
                    image,
                    vk::ImageViewType::e2D,
                    swapChain.getSwapchainFormat(),
                    componentMapping,
                    subResourceRange
            );

            imageViews.push_back(context.getDevice().createImageView(imageViewCreateInfo));
        }
        return imageViews;
    }
}
