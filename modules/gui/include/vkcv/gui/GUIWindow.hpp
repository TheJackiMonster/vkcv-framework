#pragma once

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include <vkcv/Core.hpp>

namespace vkcv::gui {

	class GUIWindow final : Window {
	private:
		Core& m_core;
		
		const Context& m_context;
		const Swapchain& m_swapchain;
		
		ImGuiContext* m_gui_context;
		
		ImGui_ImplVulkanH_Window m_gui_window;
		
		vk::DescriptorPool m_descriptor_pool;
	
	public:
		GUIWindow(GLFWwindow* window, Core& core);
	
		virtual ~GUIWindow() override;
		
		static GUIWindow create(Core& core, const char *windowTitle, int width = -1, int height = -1, bool resizable = false);
		
	};

}
