#include "PassManager.hpp"

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
    m_RenderPasses{},
    m_NextPassId(0)
    {}

    PassManager::~PassManager() noexcept
    {
        for(const auto &pass : m_RenderPasses)
            m_Device.destroy(pass);

        m_RenderPasses.clear();
        m_NextPassId = 0;
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
            vk::Format format = config.attachments[i].format;

            if (config.attachments[i].layout_in_pass == AttachmentLayout::DEPTH_STENCIL_ATTACHMENT)
            {
                depthAttachmentReference.attachment = i;
                depthAttachmentReference.layout = getVkLayoutFromAttachLayout(config.attachments[i].layout_in_pass);
                pDepthAttachment = &depthAttachmentReference;
            }
            else
            {
                vk::AttachmentReference attachmentRef(i, getVkLayoutFromAttachLayout(config.attachments[i].layout_in_pass));
                colorAttachmentReferences.push_back(attachmentRef);
            }

            vk::AttachmentDescription attachmentDesc({},
                                                     format,
                                                     vk::SampleCountFlagBits::e1,
                                                     getVKLoadOpFromAttachOp(config.attachments[i].load_operation),
                                                     getVkStoreOpFromAttachOp(config.attachments[i].store_operation),
                                                     vk::AttachmentLoadOp::eDontCare,
                                                     vk::AttachmentStoreOp::eDontCare,
                                                     getVkLayoutFromAttachLayout(config.attachments[i].layout_initial),
                                                     getVkLayoutFromAttachLayout(config.attachments[i].layout_final));
            attachmentDescriptions.push_back(attachmentDesc);
        }
        vk::SubpassDescription subpassDescription({},
                                                  vk::PipelineBindPoint::eGraphics,
                                                  0,
                                                  {},
                                                  static_cast<uint32_t>(colorAttachmentReferences.size()),
                                                  colorAttachmentReferences.data(),
                                                  {},
                                                  pDepthAttachment,
                                                  0,
                                                  {});

        vk::RenderPassCreateInfo passInfo({},
                                          static_cast<uint32_t>(attachmentDescriptions.size()),
                                          attachmentDescriptions.data(),
                                          1,
                                          &subpassDescription,
                                          0,
                                          {});

        vk::RenderPass vkObject{nullptr};
        if(m_Device.createRenderPass(&passInfo, nullptr, &vkObject) != vk::Result::eSuccess)
            return PassHandle();

        m_RenderPasses.push_back(vkObject);
		return PassHandle(m_NextPassId++);
    }

    vk::RenderPass PassManager::getVkPass(const PassHandle &handle) const
    {
        return m_RenderPasses[handle.getId()];
    }
}
