
#include "vkcv/gui/GUI.hpp"

#include <GLFW/glfw3.h>
#include <vkcv/Logger.hpp>

namespace vkcv::gui {
	
	static void checkVulkanResult(VkResult resultCode) {
		if (resultCode == 0)
			return;
		
		const auto result = vk::Result(resultCode);
		
		vkcv_log(LogLevel::ERROR, "ImGui has a problem with Vulkan! (%s)", vk::to_string(result).c_str());
	}
	
	GUI::GUI(GLFWwindow* window, Core& core) :
	m_core(core),
	m_context(core.getContext()),
	m_gui_context(nullptr) {
		IMGUI_CHECKVERSION();
		
		m_gui_context = ImGui::CreateContext();
		
		ImGui_ImplGlfw_InitForVulkan(window, false);
		
		vk::DescriptorPoolSize pool_sizes[] = {
				vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformTexelBuffer, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eStorageTexelBuffer, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eStorageBufferDynamic, 1000),
				vk::DescriptorPoolSize(vk::DescriptorType::eInputAttachment, 1000)
		};
		
		const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo (
				vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
				static_cast<uint32_t>(1000 * IM_ARRAYSIZE(pool_sizes)),
				static_cast<uint32_t>(IM_ARRAYSIZE(pool_sizes)),
				pool_sizes
		);
		
		m_descriptor_pool = m_context.getDevice().createDescriptorPool(descriptorPoolCreateInfo);
		
		const vk::PhysicalDevice& physicalDevice = m_context.getPhysicalDevice();
		const Swapchain& swapchain = core.getSwapchain();
		
		const uint32_t graphicsQueueFamilyIndex = (
				m_context.getQueueManager().getGraphicsQueues()[0].familyIndex
		);
		
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = m_context.getInstance();
		init_info.PhysicalDevice = m_context.getPhysicalDevice();
		init_info.Device = m_context.getDevice();
		init_info.QueueFamily = graphicsQueueFamilyIndex;
		init_info.Queue = m_context.getQueueManager().getGraphicsQueues()[0].handle;
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = m_descriptor_pool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = swapchain.getImageCount();
		init_info.ImageCount = swapchain.getImageCount();
		init_info.CheckVkResultFn = checkVulkanResult;
		
		const vk::AttachmentDescription attachment (
				vk::AttachmentDescriptionFlags(),
				swapchain.getFormat(),
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eLoad,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR
		);
		
		const vk::AttachmentReference attachmentReference (
				0,
				vk::ImageLayout::eColorAttachmentOptimal
		);
		
		const vk::SubpassDescription subpass (
				vk::SubpassDescriptionFlags(),
				vk::PipelineBindPoint::eGraphics,
				0,
				nullptr,
				1,
				&attachmentReference,
				nullptr,
				nullptr,
				0,
				nullptr
		);
		
		const vk::SubpassDependency dependency (
				VK_SUBPASS_EXTERNAL,
				0,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlags(),
				vk::AccessFlagBits::eColorAttachmentWrite,
				vk::DependencyFlags()
		);
		
		const vk::RenderPassCreateInfo passCreateInfo (
				vk::RenderPassCreateFlags(),
				1,
				&attachment,
				1,
				&subpass,
				1,
				&dependency
		);
		
		m_render_pass = m_context.getDevice().createRenderPass(passCreateInfo);
		
		ImGui_ImplVulkan_Init(&init_info, m_render_pass);
		
		const SubmitInfo submitInfo { QueueType::Graphics, {}, {} };
		
		core.recordAndSubmitCommands(submitInfo, [](const vk::CommandBuffer& commandBuffer) {
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		}, []() {
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		});
		
		m_context.getDevice().waitIdle();
	}
	
	GUI::~GUI() {
		m_context.getDevice().waitIdle();
		
		ImGui_ImplVulkan_Shutdown();
		
		m_context.getDevice().destroyRenderPass(m_render_pass);
		m_context.getDevice().destroyDescriptorPool(m_descriptor_pool);
		
		ImGui_ImplGlfw_Shutdown();
		
		if (m_gui_context) {
			ImGui::DestroyContext(m_gui_context);
		}
	}
	
	GUI GUI::create(Core &core, Window& window) {
		return GUI(window.getWindow(), core);
	}
	
	void GUI::beginGUI() {
		const Swapchain& swapchain = m_core.getSwapchain();
		const auto extent = swapchain.getExtent();
		
		if ((extent.width > 0) && (extent.height > 0)) {
			ImGui_ImplVulkan_SetMinImageCount(swapchain.getImageCount());
		}
		
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	
	void GUI::endGUI() {
		ImGui::Render();
		
		ImDrawData* drawData = ImGui::GetDrawData();
		
		if ((!drawData) ||
			(drawData->DisplaySize.x <= 0.0f) ||
			(drawData->DisplaySize.y <= 0.0f)) {
			return;
		}
		
		const Swapchain& swapchain = m_core.getSwapchain();
		const auto extent = swapchain.getExtent();
		
		const vk::FramebufferCreateInfo framebufferCreateInfo (
				vk::FramebufferCreateFlags(),
				m_render_pass,
				1,
				&m_core.getSwapchainImageView(),
				extent.width,
				extent.height,
				1
		);
		
		const vk::Framebuffer framebuffer = m_context.getDevice().createFramebuffer(framebufferCreateInfo);
		
		SubmitInfo submitInfo;
		submitInfo.queueType = QueueType::Graphics;
		
		m_core.recordAndSubmitCommands(submitInfo, [&](const vk::CommandBuffer& commandBuffer) {
			const vk::Rect2D renderArea (
					vk::Offset2D(0, 0),
					extent
			);
			
			const vk::RenderPassBeginInfo beginInfo (
					m_render_pass,
					framebuffer,
					renderArea,
					0,
					nullptr
			);
			
			commandBuffer.beginRenderPass(beginInfo, vk::SubpassContents::eInline);
			
			ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
			
			commandBuffer.endRenderPass();
		}, [&]() {
			m_context.getDevice().destroyFramebuffer(framebuffer);
		});
	}
	
}
