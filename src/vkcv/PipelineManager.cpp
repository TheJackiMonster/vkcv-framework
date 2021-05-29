#include "PipelineManager.hpp"

namespace vkcv
{

    PipelineManager::PipelineManager(vk::Device device) noexcept :
    m_Device{device},
    m_Pipelines{},
    m_PipelineLayouts{},
    m_NextPipelineId{1}
    {}

    PipelineManager::~PipelineManager() noexcept
    {
        for(const auto &pipeline: m_Pipelines)
            m_Device.destroy(pipeline);

        for(const auto &layout : m_PipelineLayouts)
            m_Device.destroy(layout);

        m_Pipelines.clear();
        m_PipelineLayouts.clear();
        m_NextPipelineId = 1;
    }

    PipelineHandle PipelineManager::createPipeline(const PipelineConfig &config, const vk::RenderPass &pass)
    {
        const bool existsVertexShader = config.m_ShaderProgram.existsShader(ShaderStage::VERTEX);
        const bool existsFragmentShader = config.m_ShaderProgram.existsShader(ShaderStage::FRAGMENT);
        if (!(existsVertexShader && existsFragmentShader))
        {
            std::cout << "Core::createGraphicsPipeline requires vertex and fragment shader code" << std::endl;
            return PipelineHandle{0};
        }

        // vertex shader stage
        std::vector<char> vertexCode = config.m_ShaderProgram.getShader(ShaderStage::VERTEX).shaderCode;
        vk::ShaderModuleCreateInfo vertexModuleInfo({}, vertexCode.size(), reinterpret_cast<uint32_t*>(vertexCode.data()));
        vk::ShaderModule vertexModule{};
        if (m_Device.createShaderModule(&vertexModuleInfo, nullptr, &vertexModule) != vk::Result::eSuccess)
            return PipelineHandle{0};

        vk::PipelineShaderStageCreateInfo pipelineVertexShaderStageInfo(
                {},
                vk::ShaderStageFlagBits::eVertex,
                vertexModule,
                "main",
                nullptr
        );

        // fragment shader stage
        std::vector<char> fragCode = config.m_ShaderProgram.getShader(ShaderStage::FRAGMENT).shaderCode;
        vk::ShaderModuleCreateInfo fragmentModuleInfo({}, fragCode.size(), reinterpret_cast<uint32_t*>(fragCode.data()));
        vk::ShaderModule fragmentModule{};
        if (m_Device.createShaderModule(&fragmentModuleInfo, nullptr, &fragmentModule) != vk::Result::eSuccess)
        {
            m_Device.destroy(vertexModule);
            return PipelineHandle{0};
        }

        vk::PipelineShaderStageCreateInfo pipelineFragmentShaderStageInfo(
                {},
                vk::ShaderStageFlagBits::eFragment,
                fragmentModule,
                "main",
                nullptr
        );

        // vertex input state

        // Fill up VertexInputBindingDescription and VertexInputAttributeDescription Containers
        std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
        std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;

        VertexLayout layout = config.m_ShaderProgram.getVertexLayout();
        std::unordered_map<uint32_t, VertexInputAttachment> attachments = layout.attachmentMap;

        for (auto& attachment: attachments) {
            uint32_t location = attachment.second.location;
            uint32_t binding = attachment.second.binding;
            uint32_t offset = attachment.second.offset;
            vk::Format vertexFormat = vertexFormatToVulkanFormat(attachment.second.format);
            vertexBindingDescriptions.push_back({binding, layout.stride, vk::VertexInputRate::eVertex}); // TODO: What's about the input rate?
            vertexAttributeDescriptions.push_back({location, binding, vk::Format::eR32G32B32Sfloat, offset});
        }

        // Handover Containers to PipelineVertexInputStateCreateIngo Struct
        vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
                {},
                vertexBindingDescriptions.size(),
                vertexBindingDescriptions.data(),
                vertexAttributeDescriptions.size(),
                vertexAttributeDescriptions.data()
        );

        // input assembly state
        vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
                {},
                vk::PrimitiveTopology::eTriangleList,
                false
        );

        // viewport state
        vk::Viewport viewport(0.f, 0.f, static_cast<float>(config.m_Width), static_cast<float>(config.m_Height), 0.f, 1.f);
        vk::Rect2D scissor({ 0,0 }, { config.m_Width, config.m_Height });
        vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo({}, 1, &viewport, 1, &scissor);

        // rasterization state
        vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
                {},
                false,
                false,
                vk::PolygonMode::eFill,
                vk::CullModeFlagBits::eNone,
                vk::FrontFace::eCounterClockwise,
                false,
                0.f,
                0.f,
                0.f,
                1.f
        );

        // multisample state
        vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
                {},
                vk::SampleCountFlagBits::e1,
                false,
                0.f,
                nullptr,
                false,
                false
        );

        // color blend state
        vk::ColorComponentFlags colorWriteMask(VK_COLOR_COMPONENT_R_BIT |
                                               VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT |
                                               VK_COLOR_COMPONENT_A_BIT);
        vk::PipelineColorBlendAttachmentState colorBlendAttachmentState(
                false,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eOne,
                vk::BlendOp::eAdd,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eOne,
                vk::BlendOp::eAdd,
                colorWriteMask
        );
        vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
                {},
                false,
                vk::LogicOp::eClear,
                1,	//TODO: hardcoded to one
                &colorBlendAttachmentState,
                { 1.f,1.f,1.f,1.f }
        );

		const size_t matrixPushConstantSize = 4 * 4 * sizeof(float);
		const vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eAll, 0, matrixPushConstantSize);

        // pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			{},
			{},
			(pushConstantRange));
        vk::PipelineLayout vkPipelineLayout{};
        if (m_Device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout) != vk::Result::eSuccess)
        {
            m_Device.destroy(vertexModule);
            m_Device.destroy(fragmentModule);
            return PipelineHandle{0};
        }

        // graphics pipeline create
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { pipelineVertexShaderStageInfo, pipelineFragmentShaderStageInfo };
        vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
                {},
                static_cast<uint32_t>(shaderStages.size()),
                shaderStages.data(),
                &pipelineVertexInputStateCreateInfo,
                &pipelineInputAssemblyStateCreateInfo,
                nullptr,
                &pipelineViewportStateCreateInfo,
                &pipelineRasterizationStateCreateInfo,
                &pipelineMultisampleStateCreateInfo,
                nullptr,
                &pipelineColorBlendStateCreateInfo,
                nullptr,
                vkPipelineLayout,
                pass,
                0,
                {},
                0
        );

        vk::Pipeline vkPipeline{};
        if (m_Device.createGraphicsPipelines(nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &vkPipeline) != vk::Result::eSuccess)
        {
            m_Device.destroy(vertexModule);
            m_Device.destroy(fragmentModule);
            return PipelineHandle{0};
        }

        m_Device.destroy(vertexModule);
        m_Device.destroy(fragmentModule);

        m_Pipelines.push_back(vkPipeline);
        m_PipelineLayouts.push_back(vkPipelineLayout);
        return PipelineHandle{m_NextPipelineId++};
    }

    vk::Pipeline PipelineManager::getVkPipeline(const PipelineHandle &handle) const
    {
        return m_Pipelines.at(handle.id -1);
    }

    vk::PipelineLayout PipelineManager::getVkPipelineLayout(const PipelineHandle &handle) const
    {
        return m_PipelineLayouts.at(handle.id - 1);
    }
}