
#include "vkcv/gui/GUIWindow.hpp"

#include <vkcv/Logger.hpp>

namespace vkcv::gui {
	
	static void checkVulkanResult(VkResult resultCode) {
		if (resultCode == 0)
			return;
		
		const vk::Result result = vk::Result(resultCode);
		
		vkcv_log(LogLevel::ERROR, "ImGui has a problem with Vulkan! (%s)", vk::to_string(result).c_str());
	}
	
	GUIWindow::GUIWindow(GLFWwindow* window, Core& core) :
	Window(window),
	m_core(core),
	m_context(core.getContext()),
	m_swapchain(core.getSwapchain()),
	m_gui_context(nullptr) {
		IMGUI_CHECKVERSION();
		
		m_gui_context = ImGui::CreateContext();
		
		const auto extent = m_swapchain.getExtent();
		const uint32_t graphicsQueueFamilyIndex = (
				m_context.getQueueManager().getGraphicsQueues()[0].familyIndex
		);
		
		ImGui_ImplVulkanH_CreateOrResizeWindow(
				m_context.getInstance(),
				m_context.getPhysicalDevice(),
				m_context.getDevice(),
				&m_gui_window,
				graphicsQueueFamilyIndex,
				nullptr,
				static_cast<int>(extent.width),
				static_cast<int>(extent.height),
				m_swapchain.getImageCount()
		);
		
		ImGui_ImplGlfw_InitForVulkan(window, true);
		
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
		
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = m_context.getInstance();
		init_info.PhysicalDevice = m_context.getPhysicalDevice();
		init_info.Device = m_context.getDevice();
		init_info.QueueFamily = graphicsQueueFamilyIndex;
		init_info.Queue = m_context.getQueueManager().getGraphicsQueues()[0].handle;
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = m_descriptor_pool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = m_swapchain.getImageCount();
		init_info.ImageCount = m_gui_window.ImageCount;
		init_info.CheckVkResultFn = checkVulkanResult;
		ImGui_ImplVulkan_Init(&init_info, m_gui_window.RenderPass);
		
		const SubmitInfo submitInfo { QueueType::Transfer, {}, {} };
		
		core.recordAndSubmitCommands(submitInfo, [](const vk::CommandBuffer& commandBuffer) {
			ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		}, []() {
			ImGui_ImplVulkan_DestroyFontUploadObjects();
		});
	}
	
	GUIWindow::~GUIWindow() {
		m_context.getDevice().waitIdle();
		
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		
		vkDestroyDescriptorPool(m_context.getDevice(), m_descriptor_pool, nullptr);
		
		ImGui_ImplVulkanH_DestroyWindow(
				m_context.getInstance(),
				m_context.getDevice(),
				&m_gui_window,
				nullptr
		);
		
		if (m_gui_context) {
			ImGui::DestroyContext(m_gui_context);
		}
	}
	
	GUIWindow GUIWindow::create(Core &core, const char *windowTitle, int width, int height, bool resizable) {
		return GUIWindow(Window::createGLFWWindow(windowTitle, width, height, resizable), core);
	}
	
}
