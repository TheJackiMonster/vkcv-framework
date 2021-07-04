#include "PassManager.hpp"
#include "vkcv/Image.hpp"

namespace vkcv
{
    static vk::ImageLayout getVkLayoutFromAttachLayout(AttachmentLayout layout)
    {
        switch(layout)
        {
            case AttachmentLayout::GENERAL:
                return vk::ImageLayout::eGeneral;
            case AttachmentLayout::COLOR_ATTACHMENT:
                return vk::ImageLayout::eColorAttachmentOptimal;
            case AttachmentLayout::SHADER_READ_ONLY:
                return vk::ImageLayout::eShaderReadOnlyOptimal;
            case AttachmentLayout::DEPTH_STENCIL_ATTACHMENT:
                return vk::ImageLayout::eDepthStencilAttachmentOptimal;
            case AttachmentLayout::DEPTH_STENCIL_READ_ONLY:
                return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
            case AttachmentLayout::PRESENTATION:
                return vk::ImageLayout::ePresentSrcKHR;
            default:
                return vk::ImageLayout::eUndefined;
        }
    }

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

    PassManager::PassManager(vk::Device device) noexcept :
    m_Device{device},
    m_Passes{}
    {}

    PassManager::~PassManager() noexcept
    {
    	for (uint64_t id = 0; id < m_Passes.size(); id++) {
			destroyPassById(id);
    	}
    }

    PassHandle PassManager::createPass(const PassConfig &config)
    {
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

        vk::RenderPass renderPass = m_Device.createRenderPass(passInfo);

        const uint64_t id = m_Passes.size();
        m_Passes.push_back({ renderPass, config });
        return PassHandle(id, [&](uint64_t id) { destroyPassById(id); });
    }

    vk::RenderPass PassManager::getVkPass(const PassHandle &handle) const
    {
    	const uint64_t id = handle.getId();
    	
    	if (id >= m_Passes.size()) {
    		return nullptr;
    	}
    	
    	auto& pass = m_Passes[id];
    	
        return pass.m_Handle;
    }
    
    const PassConfig& PassManager::getPassConfig(const PassHandle &handle) const {
		const uint64_t id = handle.getId();
	
		if (id >= m_Passes.size()) {
			static PassConfig emptyConfig = PassConfig({});
			return emptyConfig;
		}
	
		auto& pass = m_Passes[id];
	
		return pass.m_Config;
    }
    
    void PassManager::destroyPassById(uint64_t id) {
    	if (id >= m_Passes.size()) {
    		return;
    	}
    	
    	auto& pass = m_Passes[id];
	
		if (pass.m_Handle) {
			m_Device.destroy(pass.m_Handle);
			pass.m_Handle = nullptr;
		}
    }
    
}
