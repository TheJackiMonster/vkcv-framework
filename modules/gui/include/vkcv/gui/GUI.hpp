#pragma once

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include <vkcv/Core.hpp>
#include <vkcv/Window.hpp>

namespace vkcv::gui {

	class GUI final {
	private:
		Window& m_window;
		Core& m_core;
		
		const Context& m_context;
		
		ImGuiContext* m_gui_context;
		
		vk::DescriptorPool m_descriptor_pool;
		vk::RenderPass m_render_pass;
		
		event_handle<int,int,int> f_mouseButton;
		event_handle<double,double> f_mouseScroll;
		event_handle<int,int,int,int> f_key;
		event_handle<unsigned int> f_char;
		
	public:
		/**
		 * Constructor of a new instance of ImGui management
		 *
		 * @param core Valid #Core instance of the framework
		 * @param window Valid #Window instance of the framework
		 */
		GUI(Core& core, Window& window);
		
		GUI(const GUI& other) = delete;
		GUI(GUI&& other) = delete;
		
		GUI& operator=(const GUI& other) = delete;
		GUI& operator=(GUI&& other) = delete;
		
		/**
		 * Destructor of a #GUI instance
		 */
		virtual ~GUI();
		
		/**
		 * Sets up a new frame for ImGui to draw
		 */
		void beginGUI();
		
		/**
		 * Ends a frame for ImGui, renders it and draws it onto
		 * the currently active swapchain image of the core (ready to present).
		 */
		void endGUI();
		
	};

}
