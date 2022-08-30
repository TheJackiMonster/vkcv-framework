
#include "vkcv/PassConfig.hpp"
#include "vkcv/Image.hpp"

namespace vkcv
{
	
	AttachmentDescription::AttachmentDescription(vk::Format format,
												 AttachmentOperation load,
												 AttachmentOperation store)
			: m_format(format),
			  m_load_op(load),
			  m_store_op(store),
			  m_clear_value()
	{
		if (isDepthFormat(format)) {
			setClearValue(vk::ClearValue(
					vk::ClearDepthStencilValue(1.0f, 0)
			));
		} else {
			setClearValue(vk::ClearValue(
					vk::ClearColorValue(std::array<float, 4>{
							0.0f, 0.0f, 0.0f, 0.0f
					})
			));
		}
	}
	
	AttachmentDescription::AttachmentDescription(vk::Format format,
												 AttachmentOperation load,
												 AttachmentOperation store,
												 const vk::ClearValue &clear)
	: m_format(format),
	  m_load_op(load),
	  m_store_op(store),
	  m_clear_value(clear)
	{}
	
	vk::Format AttachmentDescription::getFormat() const {
		return m_format;
	}
	
	AttachmentOperation AttachmentDescription::getLoadOperation() const {
		return m_load_op;
	}
	
	AttachmentOperation AttachmentDescription::getStoreOperation() const {
		return m_store_op;
	}
	
	void AttachmentDescription::setClearValue(const vk::ClearValue &clear) {
		m_clear_value = clear;
	}
	
	const vk::ClearValue &AttachmentDescription::getClearValue() const {
		return m_clear_value;
	}
	
	PassConfig::PassConfig()
	: m_attachments(),
	  m_multisampling(Multisampling::None)
	{}
	
	PassConfig::PassConfig(const AttachmentDescriptions &attachments,
						   Multisampling multisampling)
	: m_attachments(attachments),
	  m_multisampling(multisampling)
	{}
	
	const AttachmentDescriptions &PassConfig::getAttachments() const {
		return m_attachments;
	}
	
	void PassConfig::setMultisampling(Multisampling multisampling) {
		m_multisampling = multisampling;
	}
	
	Multisampling PassConfig::getMultisampling() const {
		return m_multisampling;
	}
	
}
