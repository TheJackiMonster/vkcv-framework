#include "vkcv/PassConfig.hpp"

#include <utility>

namespace vkcv
{
    AttachmentDescription::AttachmentDescription(
		AttachmentOperation store_op,
		AttachmentOperation load_op,
		vk::Format format) noexcept :
	store_operation{store_op},
	load_operation{load_op},
	format(format)
    {}

    PassConfig::PassConfig(std::vector<AttachmentDescription> attachments, Multisampling msaa) noexcept :
    attachments{std::move(attachments) }, msaa(msaa)
    {}
}