#include "vkcv/PassConfig.hpp"

#include <utility>

namespace vkcv
{
    AttachmentDescription::AttachmentDescription(
		AttachmentLayout initial,
		AttachmentLayout in_pass,
		AttachmentLayout final,
		AttachmentOperation store_op,
		AttachmentOperation load_op,
		vk::Format format) noexcept :
	layout_initial{initial},
	layout_in_pass{in_pass},
	layout_final{final},
	store_operation{store_op},
	load_operation{load_op},
	format(format)
    {};

    PassConfig::PassConfig(std::vector<AttachmentDescription> attachments) noexcept :
    attachments{std::move(attachments)}
    {}
}