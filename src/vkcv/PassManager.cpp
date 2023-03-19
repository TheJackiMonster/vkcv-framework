#include "PassManager.hpp"
#include "vkcv/Core.hpp"
#include "vkcv/Image.hpp"

namespace vkcv {

	static vk::AttachmentStoreOp getVkStoreOpFromAttachOp(AttachmentOperation op) {
		switch (op) {
		case AttachmentOperation::STORE:
			return vk::AttachmentStoreOp::eStore;
		default:
			return vk::AttachmentStoreOp::eDontCare;
		}
	}

	static vk::AttachmentLoadOp getVKLoadOpFromAttachOp(AttachmentOperation op) {
		switch (op) {
		case AttachmentOperation::LOAD:
			return vk::AttachmentLoadOp::eLoad;
		case AttachmentOperation::CLEAR:
			return vk::AttachmentLoadOp::eClear;
		default:
			return vk::AttachmentLoadOp::eDontCare;
		}
	}

	uint64_t PassManager::getIdFrom(const PassHandle &handle) const {
		return handle.getId();
	}

	PassHandle PassManager::createById(uint64_t id, const HandleDestroyFunction &destroy) {
		return PassHandle(id, destroy);
	}

	void PassManager::destroyById(uint64_t id) {
		auto &pass = getById(id);

		if (pass.m_Handle) {
			getCore().getContext().getDevice().destroy(pass.m_Handle);
			pass.m_Handle = nullptr;
		}
	}

	PassManager::PassManager() noexcept : HandleManager<PassEntry, PassHandle>() {}

	PassManager::~PassManager() noexcept {
		clear();
	}

	PassHandle PassManager::createPass(const PassConfig &config) {
		// description of all {color, input, depth/stencil} attachments of the render pass
		Vector<vk::AttachmentDescription> attachmentDescriptions {};

		// individual references to color attachments (of a subpass)
		Vector<vk::AttachmentReference> colorAttachmentReferences {};
		// individual reference to depth attachment (of a subpass)
		vk::AttachmentReference depthStencilAttachmentRef;

		// stays nullptr if no depth attachment used
		vk::AttachmentReference* pDepthStencil = nullptr;

		const auto &featureManager = getCore().getContext().getFeatureManager();

		const bool separateDepthStencil =
			(featureManager.checkFeatures<vk::PhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR>(
				vk::StructureType::ePhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR,
				[](const vk::PhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR &features) {
					return features.separateDepthStencilLayouts;
				}));

		const auto &attachments = config.getAttachments();

		Vector<vk::ImageLayout> layouts;
		layouts.reserve(attachments.size());

		for (uint32_t i = 0; i < attachments.size(); i++) {
			vk::Format format = attachments [i].getFormat();
			vk::ImageLayout layout;

			bool depthFormat = isDepthFormat(attachments [i].getFormat());
			bool stencilFormat = isStencilFormat(attachments [i].getFormat());

			if ((separateDepthStencil) && (depthFormat) && (!stencilFormat)) {
				layout = vk::ImageLayout::eDepthAttachmentOptimal;
			} else if ((separateDepthStencil) && (!depthFormat) && (stencilFormat)) {
				layout = vk::ImageLayout::eStencilAttachmentOptimal;
			} else if ((depthFormat) || (stencilFormat)) {
				layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			} else {
				layout = vk::ImageLayout::eColorAttachmentOptimal;
			}

			if ((depthFormat) || (stencilFormat)) {
				depthStencilAttachmentRef = vk::AttachmentReference(i, layout);
				pDepthStencil = &depthStencilAttachmentRef;
			} else {
				vk::AttachmentReference attachmentRef(i, layout);
				colorAttachmentReferences.push_back(attachmentRef);
			}

			vk::AttachmentDescription attachmentDesc(
				{}, format, msaaToSampleCountFlagBits(config.getMultisampling()),
				getVKLoadOpFromAttachOp(attachments [i].getLoadOperation()),
				getVkStoreOpFromAttachOp(attachments [i].getStoreOperation()),
				vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, layout, layout);

			if (stencilFormat) {
				attachmentDesc.setStencilLoadOp(attachmentDesc.loadOp);
				attachmentDesc.setStencilStoreOp(attachmentDesc.storeOp);
			}

			attachmentDescriptions.push_back(attachmentDesc);
			layouts.push_back(layout);
		}

		const vk::SubpassDescription subpassDescription(
			{}, vk::PipelineBindPoint::eGraphics, 0, {},
			static_cast<uint32_t>(colorAttachmentReferences.size()),
			colorAttachmentReferences.data(), {}, pDepthStencil, 0, {});

		const vk::RenderPassCreateInfo passInfo(
			{}, static_cast<uint32_t>(attachmentDescriptions.size()), attachmentDescriptions.data(),
			1, &subpassDescription, 0, {});

		vk::RenderPass renderPass = getCore().getContext().getDevice().createRenderPass(passInfo);

		return add({ renderPass, config, layouts });
	}

	vk::RenderPass PassManager::getVkPass(const PassHandle &handle) const {
		auto &pass = (*this) [handle];
		return pass.m_Handle;
	}

	const PassConfig &PassManager::getPassConfig(const PassHandle &handle) const {
		auto &pass = (*this) [handle];
		return pass.m_Config;
	}

	const Vector<vk::ImageLayout> &PassManager::getLayouts(const PassHandle &handle) const {
		auto &pass = (*this) [handle];
		return pass.m_Layouts;
	}

} // namespace vkcv
