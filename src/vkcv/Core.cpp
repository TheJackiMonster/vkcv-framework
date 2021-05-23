/**
 * @authors Artur Wasmut
 * @file src/vkcv/Core.cpp
 * @brief Handling of global states regarding dependencies
 */

#include <GLFW/glfw3.h>

#include "vkcv/Core.hpp"
#include "PassManager.hpp"
#include "PipelineManager.hpp"
#include "DescriptorManager.hpp"
#include "Surface.hpp"
#include "ImageLayoutTransitions.hpp"
#include "Framebuffer.hpp"

namespace vkcv
{

    Core Core::create(const Window &window,
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
	
		const vk::SurfaceKHR surface = createSurface(
				window.getWindow(),
				context.getInstance(),
				context.getPhysicalDevice()
		);

        SwapChain swapChain = SwapChain::create(window, context, surface);

        std::vector<vk::Image> swapChainImages = context.getDevice().getSwapchainImagesKHR(swapChain.getSwapchain());
        std::vector<vk::ImageView> imageViews;
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
                    swapChain.getSurfaceFormat().format,
                    componentMapping,
                    subResourceRange
            );

            imageViews.push_back(context.getDevice().createImageView(imageViewCreateInfo));
        }

        const auto& queueManager = context.getQueueManager();
        
		const int graphicQueueFamilyIndex = queueManager.getGraphicsQueues()[0].familyIndex;
		const auto defaultCommandResources = createDefaultCommandResources(context.getDevice(), graphicQueueFamilyIndex);
		const auto defaultSyncResources = createDefaultSyncResources(context.getDevice());

        return Core(std::move(context) , window, swapChain, imageViews, defaultCommandResources, defaultSyncResources);
    }

    const Context &Core::getContext() const
    {
        return m_Context;
    }

	Core::Core(Context &&context, const Window &window , SwapChain swapChain,  std::vector<vk::ImageView> imageViews, 
		const CommandResources& commandResources, const SyncResources& syncResources) noexcept :
            m_Context(std::move(context)),
            m_window(window),
            m_swapchain(swapChain),
            m_swapchainImageViews(imageViews),
            m_PassManager{std::make_unique<PassManager>(m_Context.m_Device)},
            m_PipelineManager{std::make_unique<PipelineManager>(m_Context.m_Device)},
            m_CommandResources(commandResources),
            m_SyncResources(syncResources)
	{}

	Core::~Core() noexcept {
		m_Context.getDevice().waitIdle();
		for (auto image : m_swapchainImageViews) {
			m_Context.m_Device.destroyImageView(image);
		}

		destroyCommandResources(m_Context.getDevice(), m_CommandResources);
		destroySyncResources(m_Context.getDevice(), m_SyncResources);
		destroyTemporaryFramebuffers();

		m_Context.m_Device.destroySwapchainKHR(m_swapchain.getSwapchain());
		m_Context.m_Instance.destroySurfaceKHR(m_swapchain.getSurface());
	}

    PipelineHandle Core::createGraphicsPipeline(const PipelineConfig &config)
    {
        const vk::RenderPass &pass = m_PassManager->getVkPass(config.m_PassHandle);
        return m_PipelineManager->createPipeline(config, pass);
    }


    PassHandle Core::createPass(const PassConfig &config)
    {
        return m_PassManager->createPass(config);
    }

	Result Core::acquireSwapchainImage() {
    	uint32_t imageIndex;
    	
		const auto& acquireResult = m_Context.getDevice().acquireNextImageKHR(
				m_swapchain.getSwapchain(), std::numeric_limits<uint64_t>::max(), nullptr,
				m_SyncResources.swapchainImageAcquired, &imageIndex, {}
		);
		
		if (acquireResult != vk::Result::eSuccess) {
			return Result::ERROR;
		}
		
		const auto& result = m_Context.getDevice().waitForFences(
				m_SyncResources.swapchainImageAcquired, true,
				std::numeric_limits<uint64_t>::max()
		);
		
		m_Context.getDevice().resetFences(m_SyncResources.swapchainImageAcquired);
		
		if (result != vk::Result::eSuccess) {
			return Result::ERROR;
		}
		
		m_currentSwapchainImageIndex = imageIndex;
		return Result::SUCCESS;
	}

	void Core::destroyTemporaryFramebuffers() {
		for (const vk::Framebuffer f : m_TemporaryFramebuffers) {
			m_Context.getDevice().destroyFramebuffer(f);
		}
		m_TemporaryFramebuffers.clear();
	}

	void Core::beginFrame() {
    	if (acquireSwapchainImage() != Result::SUCCESS) {
    		return;
    	}
		
		m_Context.getDevice().waitIdle();	// FIMXE: this is a sin against graphics programming, but its getting late - Alex
		destroyTemporaryFramebuffers();
		const vk::CommandBufferUsageFlags beginFlags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		const vk::CommandBufferBeginInfo beginInfos(beginFlags);
		m_CommandResources.commandBuffer.begin(beginInfos);
	}

	void Core::renderTriangle(const PassHandle renderpassHandle, const PipelineHandle pipelineHandle, 
		const int width, const int height) {
		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}
  
		const vk::RenderPass renderpass = m_PassManager->getVkPass(renderpassHandle);
		const std::array<float, 4> clearColor = { 0.f, 0.f, 0.f, 1.f };
		const vk::ClearValue clearValues(clearColor);
		const vk::Rect2D renderArea(vk::Offset2D(0, 0), vk::Extent2D(width, height));
		const vk::ImageView imageView = m_swapchainImageViews[m_currentSwapchainImageIndex];
		const vk::Framebuffer framebuffer = createFramebuffer(m_Context.getDevice(), renderpass, width, height, imageView);
		m_TemporaryFramebuffers.push_back(framebuffer);
		const vk::RenderPassBeginInfo beginInfo(renderpass, framebuffer, renderArea, 1, &clearValues);
		const vk::SubpassContents subpassContents = {};
		m_CommandResources.commandBuffer.beginRenderPass(beginInfo, subpassContents, {});

		const vk::Pipeline pipeline = m_PipelineManager->getVkPipeline(pipelineHandle);
		m_CommandResources.commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline, {});
		m_CommandResources.commandBuffer.draw(3, 1, 0, 0, {});
		m_CommandResources.commandBuffer.endRenderPass();
	}

	void Core::endFrame() {
		if (m_currentSwapchainImageIndex == std::numeric_limits<uint32_t>::max()) {
			return;
		}
  
		const auto swapchainImages = m_Context.getDevice().getSwapchainImagesKHR(m_swapchain.getSwapchain());
		const vk::Image presentImage = swapchainImages[m_currentSwapchainImageIndex];

		m_CommandResources.commandBuffer.end();
		
		const auto& queueManager = m_Context.getQueueManager();
		
		const vk::SubmitInfo submitInfo(0, nullptr, 0, 1, &(m_CommandResources.commandBuffer), 1, &m_SyncResources.renderFinished);
		queueManager.getGraphicsQueues()[0].handle.submit({submitInfo}, nullptr);

		vk::Result presentResult;
		const vk::SwapchainKHR& swapchain = m_swapchain.getSwapchain();
		const vk::PresentInfoKHR presentInfo(1, &m_SyncResources.renderFinished, 1, &swapchain, 
			&m_currentSwapchainImageIndex, &presentResult);
        queueManager.getPresentQueue().handle.presentKHR(presentInfo);
		if (presentResult != vk::Result::eSuccess) {
			std::cout << "Error: swapchain present failed" << std::endl;
		}
	}

	vk::Format Core::getSwapchainImageFormat() {
		return m_swapchain.getSurfaceFormat().format;
	}

    ResourcesHandle Core::createResourceDescription(const std::vector<DescriptorSet> &descriptorSets)
    {
        // TODO:
        //  call DescriptorManager's createResourceDescription
        //  let it do the actual work! No vulkan stuff here.
		DescriptorManager descriptorManager = DescriptorManager(m_Context.getDevice());
        return descriptorManager.createResourceDescription(descriptorSets);
    }
}
