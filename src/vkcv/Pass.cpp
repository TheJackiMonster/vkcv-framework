
#include "vkcv/Pass.hpp"

namespace vkcv {
	
	PassHandle passFormats(Core &core,
						   const std::vector<vk::Format> formats,
						   bool clear,
						   Multisampling multisampling) {
		AttachmentDescriptions attachments;
		
		for (const auto format : formats) {
			attachments.emplace_back(
					format,
					clear? AttachmentOperation::CLEAR : AttachmentOperation::LOAD,
					AttachmentOperation::STORE
			);
		}
		
		const PassConfig config (attachments, multisampling);
		return core.createPass(config);
	}
	
	PassHandle passFormat(Core &core,
						  vk::Format format,
						  bool clear,
						  Multisampling multisampling) {
		return passFormats(core, { format }, clear, multisampling);
	}
	
	PassHandle passSwapchain(Core &core,
							 const SwapchainHandle &swapchain,
							 const std::vector<vk::Format> formats,
							 bool clear,
							 Multisampling multisampling) {
		std::vector<vk::Format> swapchainFormats (formats);
		
		for (auto& format : swapchainFormats) {
			if (vk::Format::eUndefined == format) {
				format = core.getSwapchainFormat(swapchain);
			}
		}
		
		return passFormats(core, swapchainFormats, clear, multisampling);
	}
	
}
