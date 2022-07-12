#pragma once
/**
 * @authors Alexander Gauggel, Artur Wasmut, Tobias Frisch
 * @file vkcv/PassConfig.hpp
 * @brief Enums and structures to handle render pass configuration.
 */

#include <vector>
#include <vulkan/vulkan.hpp>

#include "Multisampling.hpp"

namespace vkcv
{
	
	/**
	 * @brief Enum class to specify kinds of attachment layouts.
	 */
    enum class AttachmentLayout {
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

	/**
	 * @brief Enum class to specify types of attachment operations.
	 */
    enum class AttachmentOperation {
        LOAD,
        CLEAR,
        STORE,
        DONT_CARE
    };

	/**
	 * @brief Structure to store details about an attachment of a pass.
	 */
    struct AttachmentDescription {
        AttachmentOperation store_operation;
        AttachmentOperation load_operation;
        vk::Format format;
    };

	/**
	 * @brief Structure to configure a pass for usage.
	 */
    struct PassConfig {
		std::vector<AttachmentDescription> attachments;
        Multisampling msaa;
    };
	
}