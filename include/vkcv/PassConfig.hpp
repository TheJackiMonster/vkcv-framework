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
    class AttachmentDescription {
	private:
		vk::Format m_format;
	
		AttachmentOperation m_load_op;
        AttachmentOperation m_store_op;
		
		vk::ClearValue m_clear_value;
	public:
		AttachmentDescription(vk::Format format,
							  AttachmentOperation load,
							  AttachmentOperation store);
		
		AttachmentDescription(vk::Format format,
							  AttachmentOperation load,
							  AttachmentOperation store,
							  const vk::ClearValue &clear);
	
		AttachmentDescription(const AttachmentDescription &other) = default;
		AttachmentDescription(AttachmentDescription &&other) = default;
		
		~AttachmentDescription() = default;
	
		AttachmentDescription& operator=(const AttachmentDescription &other) = default;
		AttachmentDescription& operator=(AttachmentDescription &&other) = default;
		
		vk::Format getFormat() const;
	
		AttachmentOperation getLoadOperation() const;
	
		AttachmentOperation getStoreOperation() const;
		
		void setClearValue(const vk::ClearValue &clear);
	
		const vk::ClearValue& getClearValue() const;
		
    };

	/**
	 * @brief Structure to configure a pass for usage.
	 */
    struct PassConfig {
		std::vector<AttachmentDescription> attachments;
        Multisampling msaa;
    };
	
}