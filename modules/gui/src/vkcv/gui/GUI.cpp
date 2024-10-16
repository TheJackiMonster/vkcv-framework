
#include "vkcv/gui/GUI.hpp"

#include <GLFW/glfw3.h>
#include <vkcv/Logger.hpp>

namespace vkcv::gui {
	
	const static vk::ImageLayout initialImageLayout = vk::ImageLayout::eColorAttachmentOptimal;
	const static vk::ImageLayout finalImageLayout   = vk::ImageLayout::ePresentSrcKHR;

	static void checkVulkanResult(VkResult resultCode) {
		if (resultCode == 0)
			return;
		
		const auto result = vk::Result(resultCode);
		
		vkcv_log(LogLevel::ERROR, "ImGui has a problem with Vulkan! (%s)",
						 vk::to_string(result).c_str());
	}
	
	GUI::GUI(Core& core, WindowHandle& windowHandle) :
	m_windowHandle(windowHandle),
	m_core(core),
	m_context(m_core.getContext()),
	m_gui_context(nullptr) {
		IMGUI_CHECKVERSION();
		
		m_gui_context = ImGui::CreateContext();

		Window& window = m_core.getWindow(windowHandle);

		ImGui_ImplGlfw_InitForVulkan(window.getWindow(), false);
		
		f_mouseButton = window.e_mouseButton.add([&](int button, int action, int mods) {
			ImGui_ImplGlfw_MouseButtonCallback(window.getWindow(), button, action, mods);
		});
		
		f_mouseScroll = window.e_mouseScroll.add([&](double xoffset, double yoffset) {
			ImGui_ImplGlfw_ScrollCallback(window.getWindow(), xoffset, yoffset);
		});
		
		f_key = window.e_key.add([&](int key, int scancode, int action, int mods) {
			ImGui_ImplGlfw_KeyCallback(window.getWindow(), key, scancode, action, mods);
		});
		
		f_char = window.e_char.add([&](unsigned int c) {
			ImGui_ImplGlfw_CharCallback(window.getWindow(), c);
		});
		
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
		const SwapchainHandle& swapchainHandle = m_core.getWindow(m_windowHandle).getSwapchain();
		const uint32_t swapchainImageCount = m_core.getSwapchainImageCount(swapchainHandle);
		
		const uint32_t graphicsQueueFamilyIndex = (
				m_context.getQueueManager().getGraphicsQueues()[0].familyIndex
		);
		
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = static_cast<VkInstance>(m_context.getInstance());
		init_info.PhysicalDevice = static_cast<VkPhysicalDevice>(m_context.getPhysicalDevice());
		init_info.Device = static_cast<VkDevice>(m_context.getDevice());
		init_info.QueueFamily = graphicsQueueFamilyIndex;
		init_info.Queue = static_cast<VkQueue>(m_context.getQueueManager().getGraphicsQueues()[0].handle);
		init_info.PipelineCache = 0;
		init_info.DescriptorPool = static_cast<VkDescriptorPool>(m_descriptor_pool);
		init_info.Allocator = nullptr;
		init_info.MinImageCount = swapchainImageCount;
		init_info.ImageCount = swapchainImageCount;
		init_info.CheckVkResultFn = checkVulkanResult;

		const vk::AttachmentDescription attachment (
				vk::AttachmentDescriptionFlags(),
				m_core.getSwapchainFormat(swapchainHandle),
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eLoad,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				initialImageLayout,
				finalImageLayout
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
		
		init_info.RenderPass = m_context.getDevice().createRenderPass(passCreateInfo);
		
		ImGui_ImplVulkan_Init(&init_info);
		ImGui_ImplVulkan_CreateFontsTexture();

		m_render_pass = init_info.RenderPass;

		m_context.getDevice().waitIdle();
	}
	
	GUI::~GUI() {
		m_context.getDevice().waitIdle();
		Window& window = m_core.getWindow(m_windowHandle);

		ImGui_ImplVulkan_DestroyFontsTexture();
		ImGui_ImplVulkan_Shutdown();
		
		m_context.getDevice().destroyRenderPass(m_render_pass);
		m_context.getDevice().destroyDescriptorPool(m_descriptor_pool);
		
		ImGui_ImplGlfw_Shutdown();

		window.e_mouseButton.remove(f_mouseButton);
		window.e_mouseScroll.remove(f_mouseScroll);
		window.e_key.remove(f_key);
		window.e_char.remove(f_char);
		
		if (m_gui_context) {
			ImGui::DestroyContext(m_gui_context);
		}
	}
	
	void GUI::beginGUI() {
		const auto swapchainHandle = m_core.getWindow(m_windowHandle).getSwapchain();
		const auto& extent = m_core.getSwapchainExtent(swapchainHandle);
		
		if ((extent.width > 0) && (extent.height > 0)) {
			ImGui_ImplVulkan_SetMinImageCount(m_core.getSwapchainImageCount(swapchainHandle));
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
		
		const auto swapchainHandle = m_core.getWindow(m_windowHandle).getSwapchain();
		const auto& extent = m_core.getSwapchainExtent(swapchainHandle);

		const vk::ImageView swapchainImageView = m_core.getSwapchainImageView();

		const vk::FramebufferCreateInfo framebufferCreateInfo (
				vk::FramebufferCreateFlags(),
				m_render_pass,
				1,
				&swapchainImageView,
				extent.width,
				extent.height,
				1
		);
		
		const vk::Framebuffer framebuffer = m_context.getDevice().createFramebuffer(framebufferCreateInfo);
		auto stream = m_core.createCommandStream(QueueType::Graphics);
		
		m_core.recordCommandsToStream(stream, [&](const vk::CommandBuffer& commandBuffer) {

			assert(initialImageLayout == vk::ImageLayout::eColorAttachmentOptimal);
			m_core.prepareImageForAttachmentManually(commandBuffer, vkcv::ImageHandle::createSwapchainImageHandle());

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
			
			ImGui_ImplVulkan_RenderDrawData(drawData, static_cast<VkCommandBuffer>(commandBuffer));
			
			commandBuffer.endRenderPass();

			// executing the renderpass changed the image layout without going through the image manager
			// therefore the layout must be updated manually
			m_core.updateImageLayoutManual(vkcv::ImageHandle::createSwapchainImageHandle(), finalImageLayout);

		}, [&]() {
			m_context.getDevice().destroyFramebuffer(framebuffer);
		});
		
		m_core.submitCommandStream(stream, false);
	}
	
}
