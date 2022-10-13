//
// Created by Charlotte on 28.05.2021.
//

#include "vkcv/VertexLayout.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv {

	uint32_t getFormatSize(VertexAttachmentFormat format) {
		switch (format) {
		case VertexAttachmentFormat::FLOAT:
			return 4;
		case VertexAttachmentFormat::FLOAT2:
			return 8;
		case VertexAttachmentFormat::FLOAT3:
			return 12;
		case VertexAttachmentFormat::FLOAT4:
			return 16;
		case VertexAttachmentFormat::INT:
			return 4;
		case VertexAttachmentFormat::INT2:
			return 8;
		case VertexAttachmentFormat::INT3:
			return 12;
		case VertexAttachmentFormat::INT4:
			return 16;
		default:
			vkcv_log(LogLevel::WARNING, "No format given");
			return 0;
		}
	}

	VertexBinding createVertexBinding(uint32_t bindingLocation,
									  const VertexAttachments &attachments) {
		VertexBinding binding { bindingLocation, 0, attachments };
		uint32_t offset = 0;

		for (auto &attachment : binding.vertexAttachments) {
			attachment.offset = offset;
			offset += getFormatSize(attachment.format);
		}

		binding.stride = offset;
		return binding;
	}

	VertexBindings createVertexBindings(const VertexAttachments &attachments) {
		VertexBindings bindings;
		bindings.reserve(attachments.size());

		for (const auto& attachment : attachments) {
			bindings.push_back(createVertexBinding(attachment.inputLocation, { attachment }));
		}

		return bindings;
	}

} // namespace vkcv