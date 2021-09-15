#pragma once

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

#include <vkcv/Core.hpp>
#include <vkcv/Window.hpp>

namespace vkcv::gui {

    /**
     * @defgroup vkcv_gui GUI Module
     * A module to manage ImGUI integration for VkCV applications.
     * @{
     */

    /**
     * Class to manage ImGui integration for VkCV.
     */
	class GUI final {
	private:
        /**
         * Window handle of the currently used window to draw user interface on.
         */
		WindowHandle m_windowHandle;

        /**
         * Reference to the current Core instance.
         */
		Core& m_core;

        /**
         * Reference to the current Context instance.
         */
		const Context& m_context;

        /**
         * ImGui context for drawing GUI.
         */
		ImGuiContext* m_gui_context;

        /**
         * Vulkan handle for the ImGui descriptor pool.
         */
		vk::DescriptorPool m_descriptor_pool;

        /**
         * Vulkan handle for the ImGui render pass.
         */
		vk::RenderPass m_render_pass;

        /**
         * Event handle for pressing a mouse button.
         */
		event_handle<int,int,int> f_mouseButton;

        /**
         * Event handle for scrolling with the mouse or touchpad.
         */
		event_handle<double,double> f_mouseScroll;

        /**
         * Event handle for pressing a key.
         */
		event_handle<int,int,int,int> f_key;

        /**
         * Event handle for typing a character.
         */
		event_handle<unsigned int> f_char;
		
	public:
		/**
		 * Constructor of a new instance for ImGui management.
		 *
		 * @param[in,out] core Valid Core instance of the framework
		 * @param[in,out] window Valid Window instance of the framework
		 */
		GUI(Core& core, WindowHandle& window);
		
		GUI(const GUI& other) = delete;
		GUI(GUI&& other) = delete;
		
		GUI& operator=(const GUI& other) = delete;
		GUI& operator=(GUI&& other) = delete;
		
		/**
		 * Destructor of a GUI instance.
		 */
		virtual ~GUI();
		
		/**
		 * Sets up a new frame for ImGui to draw.
		 */
		void beginGUI();
		
		/**
		 * Ends a frame for ImGui, renders it and draws it onto
		 * the currently active swapchain image of the core (ready to present).
		 */
		void endGUI();
		
	};

    /** @} */

}
