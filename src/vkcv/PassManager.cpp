#include "PassManager.hpp"
#include "vkcv/Image.hpp"
#include "vkcv/Core.hpp"

namespace vkcv
{

    static vk::AttachmentStoreOp getVkStoreOpFromAttachOp(AttachmentOperation op)
    {
        switch(op)
        {
            case AttachmentOperation::STORE:
                return vk::AttachmentStoreOp::eStore;
            default:
                return vk::AttachmentStoreOp::eDontCare;
        }
    }

    static vk::AttachmentLoadOp getVKLoadOpFromAttachOp(AttachmentOperation op)
    {
        switch(op)
        {
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
		auto& pass = getById(id);
		
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
        std::vector<vk::AttachmentDescription> attachmentDescriptions{};

        // individual references to color attachments (of a subpass)
        std::vector<vk::AttachmentReference> colorAttachmentReferences{};
        // individual reference to depth attachment (of a subpass)
        vk::AttachmentReference depthAttachmentReference{};
        vk::AttachmentReference *pDepthAttachment = nullptr;	//stays nullptr if no depth attachment used

        for (uint32_t i = 0; i < config.attachments.size(); i++)
        {
            // TODO: Renderpass struct should hold proper format information
            vk::Format      format = config.attachments[i].format;
            vk::ImageLayout layout;

            if (isDepthFormat(config.attachments[i].format))
            {
                layout                              = vk::ImageLayout::eDepthStencilAttachmentOptimal;
                depthAttachmentReference.attachment = i;
                depthAttachmentReference.layout     = layout;
                pDepthAttachment                    = &depthAttachmentReference;
            }
            else
            {
                layout = vk::ImageLayout::eColorAttachmentOptimal;
                vk::AttachmentReference attachmentRef(i, layout);
                colorAttachmentReferences.push_back(attachmentRef);
            }

            vk::AttachmentDescription attachmentDesc(
                {},
                format,
                msaaToVkSampleCountFlag(config.msaa),
                getVKLoadOpFromAttachOp(config.attachments[i].load_operation),
                getVkStoreOpFromAttachOp(config.attachments[i].store_operation),
                vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare,
                layout,
                layout);

            attachmentDescriptions.push_back(attachmentDesc);
        }
        
        const vk::SubpassDescription subpassDescription(
            {},
            vk::PipelineBindPoint::eGraphics,
            0,
            {},
            static_cast<uint32_t>(colorAttachmentReferences.size()),
            colorAttachmentReferences.data(),
            {},
            pDepthAttachment,
            0,
            {});

        const vk::RenderPassCreateInfo passInfo(
            {},
            static_cast<uint32_t>(attachmentDescriptions.size()),
            attachmentDescriptions.data(),
            1,
            &subpassDescription,
            0,
            {});

        vk::RenderPass renderPass = getCore().getContext().getDevice().createRenderPass(passInfo);

        return add({ renderPass, config });
    }

    vk::RenderPass PassManager::getVkPass(const PassHandle &handle) const {
    	auto& pass = (*this)[handle];
        return pass.m_Handle;
    }
    
    const PassConfig& PassManager::getPassConfig(const PassHandle &handle) const {
		auto& pass = (*this)[handle];
		return pass.m_Config;
    }
    
}
