#pragma once
/**
 * @authors Alexander Gauggel, Artur Wasmut, Tobias Frisch
 * @file vkcv/PassConfig.hpp
 * @brief Enums and structures to handle render pass configuration.
 */

#include <vulkan/vulkan.hpp>

#include "Container.hpp"
#include "Multisampling.hpp"

namespace vkcv {

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
	 * @brief Class to store details about an attachment of a pass.
	 */
	class AttachmentDescription {
	private:
		vk::Format m_format;

		AttachmentOperation m_load_op;
		AttachmentOperation m_store_op;

		vk::ClearValue m_clear_value;

	public:
		AttachmentDescription(vk::Format format, AttachmentOperation load,
							  AttachmentOperation store);

		AttachmentDescription(vk::Format format, AttachmentOperation load,
							  AttachmentOperation store, const vk::ClearValue &clear);

		AttachmentDescription(const AttachmentDescription &other) = default;
		AttachmentDescription(AttachmentDescription &&other) = default;

		~AttachmentDescription() = default;

		AttachmentDescription &operator=(const AttachmentDescription &other) = default;
		AttachmentDescription &operator=(AttachmentDescription &&other) = default;

		[[nodiscard]] vk::Format getFormat() const;

		[[nodiscard]] AttachmentOperation getLoadOperation() const;

		[[nodiscard]] AttachmentOperation getStoreOperation() const;

		void setClearValue(const vk::ClearValue &clear);

		[[nodiscard]] const vk::ClearValue &getClearValue() const;
	};

	using AttachmentDescriptions = Vector<AttachmentDescription>;

	/**
	 * @brief Class to configure a pass for usage.
	 */
	class PassConfig {
	private:
		AttachmentDescriptions m_attachments;
		Multisampling m_multisampling;

	public:
		PassConfig();

		explicit PassConfig(const AttachmentDescriptions &attachments,
							Multisampling multisampling = Multisampling::None);

		PassConfig(const PassConfig &other) = default;
		PassConfig(PassConfig &&other) = default;

		~PassConfig() = default;

		PassConfig &operator=(const PassConfig &other) = default;
		PassConfig &operator=(PassConfig &&other) = default;

		[[nodiscard]] const AttachmentDescriptions &getAttachments() const;

		void setMultisampling(Multisampling multisampling);

		[[nodiscard]] Multisampling getMultisampling() const;
	};

} // namespace vkcv