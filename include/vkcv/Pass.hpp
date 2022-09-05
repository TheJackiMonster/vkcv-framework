#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Pass.hpp
 * @brief Support functions for basic pass creation.
 */

#include "Core.hpp"
#include "Handles.hpp"
#include "PassConfig.hpp"

namespace vkcv
{

	PassHandle passFormats(Core &core,
						   const std::vector<vk::Format> formats,
						   bool clear = true,
						   Multisampling multisampling = Multisampling::None);
	
	PassHandle passFormat(Core &core,
						  vk::Format format,
						  bool clear = true,
						  Multisampling multisampling = Multisampling::None);
	
	PassHandle passSwapchain(Core &core,
							 const SwapchainHandle &swapchain,
							 const std::vector<vk::Format> formats,
							 bool clear = true,
							 Multisampling multisampling = Multisampling::None);
	
}
