#pragma once
/**
 * @authors Tobias Frisch
 * @file vkcv/Pass.hpp
 * @brief Support functions for basic pass creation.
 */

#include "Container.hpp"
#include "Core.hpp"
#include "Handles.hpp"
#include "PassConfig.hpp"

namespace vkcv {

	PassHandle passFormats(Core &core, const Vector<vk::Format> &formats, bool clear = true,
						   Multisampling multisampling = Multisampling::None);

	PassHandle passFormat(Core &core, vk::Format format, bool clear = true,
						  Multisampling multisampling = Multisampling::None);

	PassHandle passSwapchain(Core &core, const SwapchainHandle &swapchain,
							 const Vector<vk::Format> &formats, bool clear = true,
							 Multisampling multisampling = Multisampling::None);

} // namespace vkcv
