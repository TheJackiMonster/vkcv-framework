#include "PipelineManager.hpp"
#include "vkcv/Image.hpp"

namespace vkcv
{

    PipelineManager::PipelineManager(vk::Device device) noexcept :
    m_Device{device},
    m_Pipelines{},
    m_Configs{}
    {}

    PipelineManager::~PipelineManager() noexcept
    {
    	for (uint64_t id = 0; id < m_Pipelines.size(); id++) {
			destroyPipelineById(id);
    	}
    }

	// currently assuming default 32 bit formats, no lower precision or normalized variants supported
	vk::Format vertexFormatToVulkanFormat(const VertexFormat format) {
		switch (format) {
		case VertexFormat::FLOAT: return vk::Format::eR32Sfloat;
		case VertexFormat::FLOAT2: return vk::Format::eR32G32Sfloat;
		case VertexFormat::FLOAT3: return vk::Format::eR32G32B32Sfloat;
		case VertexFormat::FLOAT4: return vk::Format::eR32G32B32A32Sfloat;
		case VertexFormat::INT: return vk::Format::eR32Sint;
		case VertexFormat::INT2: return vk::Format::eR32G32Sint;
		case VertexFormat::INT3: return vk::Format::eR32G32B32Sint;
		case VertexFormat::INT4: return vk::Format::eR32G32B32A32Sint;
		default: std::cerr << "Warning: Unknown vertex format" << std::endl; return vk::Format::eUndefined;
		}
	}

    PipelineHandle PipelineManager::createPipeline(const PipelineConfig &config, PassManager& passManager)
    {
		const vk::RenderPass &pass = passManager.getVkPass(config.m_PassHandle);
    	
        const bool existsVertexShader = config.m_ShaderProgram.existsShader(ShaderStage::VERTEX);
        const bool existsFragmentShader = config.m_ShaderProgram.existsShader(ShaderStage::FRAGMENT);
        if (!(existsVertexShader && existsFragmentShader))
        {
            std::cout << "Core::createGraphicsPipeline requires vertex and fragment shader code" << std::endl;
            return PipelineHandle();
        }

        // vertex shader stage
        std::vector<char> vertexCode = config.m_ShaderProgram.getShader(ShaderStage::VERTEX).shaderCode;
        vk::ShaderModuleCreateInfo vertexModuleInfo({}, vertexCode.size(), reinterpret_cast<uint32_t*>(vertexCode.data()));
        vk::ShaderModule vertexModule{};
        if (m_Device.createShaderModule(&vertexModuleInfo, nullptr, &vertexModule) != vk::Result::eSuccess)
            return PipelineHandle();

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
            return PipelineHandle();
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
        std::vector<vk::VertexInputAttributeDescription>	vertexAttributeDescriptions;
		std::vector<vk::VertexInputBindingDescription>		vertexBindingDescriptions;

        VertexLayout layout = config.m_ShaderProgram.getVertexLayout();
        std::unordered_map<uint32_t, VertexInputAttachment> attachments = layout.attachmentMap;

		for (int i = 0; i < attachments.size(); i++) {
			VertexInputAttachment &attachment = attachments.at(i);

            uint32_t	location		= attachment.location;
            uint32_t	binding			= i;
            vk::Format	vertexFormat	= vertexFormatToVulkanFormat(attachment.format);

			//FIXME: hoping that order is the same and compatible: add explicit mapping and validation
			const VertexAttribute attribute = config.m_VertexAttributes[i];

            vertexAttributeDescriptions.emplace_back(location, binding, vertexFormatToVulkanFormat(attachment.format), 0);
			vertexBindingDescriptions.emplace_back(vk::VertexInputBindingDescription(
				binding,
				attribute.stride + getFormatSize(attachment.format),
				vk::VertexInputRate::eVertex));
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

		const size_t matrixPushConstantSize = config.m_ShaderProgram.getPushConstantSize();
		const vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eAll, 0, matrixPushConstantSize);

        // pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
			{},
			(config.m_DescriptorLayouts),
			(pushConstantRange));

        vk::PipelineLayout vkPipelineLayout{};
        if (m_Device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout) != vk::Result::eSuccess)
        {
            m_Device.destroy(vertexModule);
            m_Device.destroy(fragmentModule);
            return PipelineHandle();
        }
	
		const vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo(
				vk::PipelineDepthStencilStateCreateFlags(),
				true,
				true,
				vk::CompareOp::eLessOrEqual,
				false,
				false,
				{},
				{},
				0.0f,
				1.0f
		);
	
		const vk::PipelineDepthStencilStateCreateInfo* p_depthStencilCreateInfo = nullptr;
		
		const PassConfig& passConfig = passManager.getPassConfig(config.m_PassHandle);
		
		for (const auto& attachment : passConfig.attachments) {
			if (isDepthFormat(attachment.format)) {
				p_depthStencilCreateInfo = &depthStencilCreateInfo;
				break;
			}
		}

		std::vector<vk::DynamicState> dynamicStates = {};
		if(config.m_UseDynamicViewport)
        {
		    dynamicStates.push_back(vk::DynamicState::eViewport);
		    dynamicStates.push_back(vk::DynamicState::eScissor);
        }

        vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo({},
                                                            static_cast<uint32_t>(dynamicStates.size()),
                                                            dynamicStates.data());

        // graphics pipeline create
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = { pipelineVertexShaderStageInfo, pipelineFragmentShaderStageInfo };
        const vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
                {},
                static_cast<uint32_t>(shaderStages.size()),
                shaderStages.data(),
                &pipelineVertexInputStateCreateInfo,
                &pipelineInputAssemblyStateCreateInfo,
                nullptr,
                &pipelineViewportStateCreateInfo,
                &pipelineRasterizationStateCreateInfo,
                &pipelineMultisampleStateCreateInfo,
				p_depthStencilCreateInfo,
                &pipelineColorBlendStateCreateInfo,
                &dynamicStateCreateInfo,
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
            return PipelineHandle();
        }

        m_Device.destroy(vertexModule);
        m_Device.destroy(fragmentModule);
        
        const uint64_t id = m_Pipelines.size();
        m_Pipelines.push_back({ vkPipeline, vkPipelineLayout });
        m_Configs.push_back(config);
        return PipelineHandle(id, [&](uint64_t id) { destroyPipelineById(id); });
    }

    vk::Pipeline PipelineManager::getVkPipeline(const PipelineHandle &handle) const
    {
		const uint64_t id = handle.getId();
	
		if (id >= m_Pipelines.size()) {
			return nullptr;
		}
	
		auto& pipeline = m_Pipelines[id];
	
		return pipeline.m_handle;
    }

    vk::PipelineLayout PipelineManager::getVkPipelineLayout(const PipelineHandle &handle) const
    {
    	const uint64_t id = handle.getId();
    	
		if (id >= m_Pipelines.size()) {
			return nullptr;
		}
	
		auto& pipeline = m_Pipelines[id];
    	
        return pipeline.m_layout;
    }
    
    void PipelineManager::destroyPipelineById(uint64_t id) {
    	if (id >= m_Pipelines.size()) {
    		return;
    	}
    	
    	auto& pipeline = m_Pipelines[id];
    	
    	if (pipeline.m_handle) {
			m_Device.destroy(pipeline.m_handle);
			pipeline.m_handle = nullptr;
    	}
	
		if (pipeline.m_layout) {
			m_Device.destroy(pipeline.m_layout);
			pipeline.m_layout = nullptr;
		}
    }

    const PipelineConfig &PipelineManager::getPipelineConfig(const PipelineHandle &handle) const
    {
        const uint64_t id = handle.getId();
        return m_Configs.at(id);
    }

    PipelineHandle PipelineManager::createComputePipeline(const ShaderProgram &shaderProgram) {
        // Temporally handing over the Shader Program instead of a pipeline config

        vk::ShaderModule computeModule{};
        if (createShaderModule(computeModule, shaderProgram, ShaderStage::COMPUTE) != vk::Result::eSuccess)
            return PipelineHandle();

        vk::PipelineShaderStageCreateInfo pipelineComputeShaderStageInfo(
                {},
                vk::ShaderStageFlagBits::eCompute,
                computeModule,
                "main",
                nullptr
        );

        // TODO: Validation Layer Error -> the size is 0 but has to be greater!
        const size_t matrixPushConstantSize = shaderProgram.getPushConstantSize();
        const vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eAll, 0, matrixPushConstantSize);

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo( // TODO: Check this. I'm not sure if this is correct
                {},
                {}, // TODO: For now no Descriptor Set
                (pushConstantRange));

        vk::PipelineLayout vkPipelineLayout{};
        if (m_Device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout) != vk::Result::eSuccess)
        {
            m_Device.destroy(computeModule);
            return PipelineHandle();
        }

        vk::ComputePipelineCreateInfo computePipelineCreateInfo{};
        computePipelineCreateInfo.stage = pipelineComputeShaderStageInfo;
        computePipelineCreateInfo.layout = vkPipelineLayout;

        vk::Pipeline vkPipeline;
        if (m_Device.createComputePipelines(nullptr, 1, &computePipelineCreateInfo, nullptr, &vkPipeline)!= vk::Result::eSuccess)
        {
            m_Device.destroy(computeModule);
            return PipelineHandle();
        }

        m_Device.destroy(computeModule);

        const uint64_t id = m_Pipelines.size();
        m_Pipelines.push_back({ vkPipeline, vkPipelineLayout });

        return PipelineHandle(id, [&](uint64_t id) { destroyPipelineById(id); });
    }

    // There is an issue for refactoring the Pipeline Manager.
    // While including Compute Pipeline Creation, some private helper functions where introduced:

    vk::Result PipelineManager::createShaderModule(vk::ShaderModule &module, const ShaderProgram &shaderProgram, const ShaderStage stage)
    {
        std::vector<char> code = shaderProgram.getShader(stage).shaderCode;
        vk::ShaderModuleCreateInfo moduleInfo({}, code.size(), reinterpret_cast<uint32_t*>(code.data()));
        return m_Device.createShaderModule(&moduleInfo, nullptr, &module);
    }
}