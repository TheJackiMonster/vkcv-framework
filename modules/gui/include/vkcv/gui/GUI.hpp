#pragma once

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include <vkcv/Core.hpp>
#include <vkcv/Window.hpp>

namespace vkcv::gui {

	class GUI final {
	private:
		GLFWwindow* m_window;
		Core& m_core;
		
		const Context& m_context;
		
		ImGuiContext* m_gui_context;
		
		vk::DescriptorPool m_descriptor_pool;
		vk::RenderPass m_render_pass;
		
		GUI(GLFWwindow* window, Core& core);
		
	public:
		virtual ~GUI();
		
		static GUI create(Core& core, Window& window);
		
		void beginGUI();
		
		void endGUI();
		
	};

}
