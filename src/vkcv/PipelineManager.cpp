#include "PipelineManager.hpp"
#include "vkcv/Image.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv
{

    PipelineManager::PipelineManager(vk::Device device) noexcept :
    m_Device{device},
    m_Pipelines{}
    {}

    PipelineManager::~PipelineManager() noexcept
    {
    	for (uint64_t id = 0; id < m_Pipelines.size(); id++) {
			destroyPipelineById(id);
    	}
    }

	// currently assuming default 32 bit formats, no lower precision or normalized variants supported
	vk::Format vertexFormatToVulkanFormat(const VertexAttachmentFormat format) {
		switch (format) {
		case VertexAttachmentFormat::FLOAT:
			return vk::Format::eR32Sfloat;
		case VertexAttachmentFormat::FLOAT2:
			return vk::Format::eR32G32Sfloat;
		case VertexAttachmentFormat::FLOAT3:
			return vk::Format::eR32G32B32Sfloat;
		case VertexAttachmentFormat::FLOAT4:
			return vk::Format::eR32G32B32A32Sfloat;
		case VertexAttachmentFormat::INT:
			return vk::Format::eR32Sint;
		case VertexAttachmentFormat::INT2:
			return vk::Format::eR32G32Sint;
		case VertexAttachmentFormat::INT3:
			return vk::Format::eR32G32B32Sint;
		case VertexAttachmentFormat::INT4:
			return vk::Format::eR32G32B32A32Sint;
		default:
			vkcv_log(LogLevel::WARNING, "Unknown vertex format");
			return vk::Format::eUndefined;
		}
	}

    vk::PrimitiveTopology primitiveTopologyToVulkanPrimitiveTopology(const PrimitiveTopology topology) {
        switch (topology) {
        case(PrimitiveTopology::PointList):     return vk::PrimitiveTopology::ePointList;
        case(PrimitiveTopology::LineList):      return vk::PrimitiveTopology::eLineList;
        case(PrimitiveTopology::TriangleList):  return vk::PrimitiveTopology::eTriangleList;
        default: std::cout << "Error: Unknown primitive topology type" << std::endl; return vk::PrimitiveTopology::eTriangleList;
        }
    }

    PipelineHandle PipelineManager::createPipeline(const PipelineConfig &config, PassManager& passManager)
    {
		const vk::RenderPass &pass = passManager.getVkPass(config.m_PassHandle);
    	
        const bool existsVertexShader = config.m_ShaderProgram.existsShader(ShaderStage::VERTEX);
        const bool existsFragmentShader = config.m_ShaderProgram.existsShader(ShaderStage::FRAGMENT);
        if (!(existsVertexShader && existsFragmentShader))
        {
			vkcv_log(LogLevel::ERROR, "Requires vertex and fragment shader code");
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

        const VertexLayout &layout = config.m_VertexLayout;

        // iterate over the layout's specified, mutually exclusive buffer bindings that make up a vertex buffer
        for (const auto &vertexBinding : layout.vertexBindings)
        {
            vertexBindingDescriptions.emplace_back(vertexBinding.bindingLocation,
                                                   vertexBinding.stride,
                                                   vk::VertexInputRate::eVertex);

            // iterate over the bindings' specified, mutually exclusive vertex input attachments that make up a vertex
            for(const auto &vertexAttachment: vertexBinding.vertexAttachments)
            {
                vertexAttributeDescriptions.emplace_back(vertexAttachment.inputLocation,
                                                         vertexBinding.bindingLocation,
                                                         vertexFormatToVulkanFormat(vertexAttachment.format),
                                                         vertexAttachment.offset % vertexBinding.stride);

            }
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
            primitiveTopologyToVulkanPrimitiveTopology(config.m_PrimitiveTopology),
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
        vk::PipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterization;
        if (config.m_UseConservativeRasterization) {
            conservativeRasterization = vk::PipelineRasterizationConservativeStateCreateInfoEXT(
                {}, 
                vk::ConservativeRasterizationModeEXT::eOverestimate,
                0.f);
            pipelineRasterizationStateCreateInfo.pNext = &conservativeRasterization;
        }

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

		const char *geometryShaderName = "main";	// outside of if to make sure it stays in scope
		vk::ShaderModule geometryModule;
		if (config.m_ShaderProgram.existsShader(ShaderStage::GEOMETRY)) {
			const vkcv::Shader geometryShader = config.m_ShaderProgram.getShader(ShaderStage::GEOMETRY);
			const auto& geometryCode = geometryShader.shaderCode;
			const vk::ShaderModuleCreateInfo geometryModuleInfo({}, geometryCode.size(), reinterpret_cast<const uint32_t*>(geometryCode.data()));
			if (m_Device.createShaderModule(&geometryModuleInfo, nullptr, &geometryModule) != vk::Result::eSuccess) {
				return PipelineHandle();
			}
			vk::PipelineShaderStageCreateInfo geometryStage({}, vk::ShaderStageFlagBits::eGeometry, geometryModule, geometryShaderName);
			shaderStages.push_back(geometryStage);
		}

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
            if (geometryModule) {
                m_Device.destroy(geometryModule);
            }
            m_Device.destroy();
            return PipelineHandle();
        }

        m_Device.destroy(vertexModule);
        m_Device.destroy(fragmentModule);
        if (geometryModule) {
            m_Device.destroy(geometryModule);
        }
        
        const uint64_t id = m_Pipelines.size();
        m_Pipelines.push_back({ vkPipeline, vkPipelineLayout, config });
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

    const PipelineConfig& PipelineManager::getPipelineConfig(const PipelineHandle &handle) const
    {
        const uint64_t id = handle.getId();
        
        if (id >= m_Pipelines.size()) {
        	static PipelineConfig dummyConfig;
			vkcv_log(LogLevel::ERROR, "Invalid handle");
			return dummyConfig;
        }
        
        return m_Pipelines[id].m_config;
    }

    PipelineHandle PipelineManager::createComputePipeline(
        const ShaderProgram &shaderProgram, 
        const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts) {

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

        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo({}, descriptorSetLayouts);

        const size_t pushConstantSize = shaderProgram.getPushConstantSize();
        vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eCompute, 0, pushConstantSize);
        if (pushConstantSize > 0) {
            pipelineLayoutCreateInfo.setPushConstantRangeCount(1);
            pipelineLayoutCreateInfo.setPPushConstantRanges(&pushConstantRange);
        }

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
        m_Pipelines.push_back({ vkPipeline, vkPipelineLayout, PipelineConfig() });

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