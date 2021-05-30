#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

namespace vkcv
{
    enum class AttachmentLayout
    {
        UNDEFINED,
        GENERAL,

        COLOR_ATTACHMENT,
        SHADER_READ_ONLY,

        DEPTH_STENCIL_ATTACHMENT,
        DEPTH_STENCIL_READ_ONLY,

        TRANSFER_SRC,
        TRANSFER_DST,

        PRESENTATION
    };

    enum class AttachmentOperation
    {
        LOAD,
        CLEAR,
        STORE,
        DONT_CARE
    };

    struct AttachmentDescription
    {
        AttachmentDescription() = delete;
        AttachmentDescription(
			AttachmentLayout initial,
			AttachmentLayout in_pass,
			AttachmentLayout final,
			AttachmentOperation store_op,
			AttachmentOperation load_op,
			vk::Format format) noexcept;

        AttachmentLayout layout_initial;
        AttachmentLayout layout_in_pass;
        AttachmentLayout layout_final;

        AttachmentOperation store_operation;
        AttachmentOperation load_operation;

		vk::Format format;
    };

    struct PassConfig
    {
        explicit PassConfig(std::vector<AttachmentDescription> attachments) noexcept;
        std::vector<AttachmentDescription> attachments{};
    };
}