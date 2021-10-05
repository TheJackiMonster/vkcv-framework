#include "GraphicsPipelineManager.hpp"
#include "vkcv/Image.hpp"
#include "vkcv/Logger.hpp"

namespace vkcv
{
	
	GraphicsPipelineManager::GraphicsPipelineManager(vk::Device device, vk::PhysicalDevice physicalDevice) noexcept :
            m_Device(device),
            m_physicalDevice(physicalDevice),
            m_Pipelines{}
    {}
	
	GraphicsPipelineManager::~GraphicsPipelineManager() noexcept
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
            case(PrimitiveTopology::PointList):
                return vk::PrimitiveTopology::ePointList;
            case(PrimitiveTopology::LineList):
                return vk::PrimitiveTopology::eLineList;
            case(PrimitiveTopology::TriangleList):
                return vk::PrimitiveTopology::eTriangleList;
            default:
            vkcv_log(LogLevel::ERROR, "Unknown primitive topology type");
                return vk::PrimitiveTopology::eTriangleList;
        }
    }

    vk::CompareOp depthTestToVkCompareOp(DepthTest depthTest) {
        switch (depthTest) {
            case(DepthTest::None):
                return vk::CompareOp::eAlways;
            case(DepthTest::Less):
                return vk::CompareOp::eLess;
            case(DepthTest::LessEqual):
                return vk::CompareOp::eLessOrEqual;
            case(DepthTest::Greater):
                return vk::CompareOp::eGreater;
            case(DepthTest::GreatherEqual):
                return vk::CompareOp::eGreaterOrEqual;
            case(DepthTest::Equal):
                return vk::CompareOp::eEqual;
            default:
                vkcv_log(LogLevel::ERROR, "Unknown depth test enum");
                return vk::CompareOp::eAlways;
        }
    }

    vk::ShaderStageFlagBits shaderStageToVkShaderStage(ShaderStage stage) {
        switch (stage) {
            case ShaderStage::VERTEX:
                return vk::ShaderStageFlagBits::eVertex;
            case ShaderStage::FRAGMENT:
                return vk::ShaderStageFlagBits::eFragment;
            case ShaderStage::GEOMETRY:
                return vk::ShaderStageFlagBits::eGeometry;
            case ShaderStage::TESS_CONTROL:
                return vk::ShaderStageFlagBits::eTessellationControl;
            case ShaderStage::TESS_EVAL:
                return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case ShaderStage::COMPUTE:
                return vk::ShaderStageFlagBits::eCompute;
            case ShaderStage::TASK:
                return vk::ShaderStageFlagBits::eTaskNV;
            case ShaderStage::MESH:
                return vk::ShaderStageFlagBits::eMeshNV;
            default:
                vkcv_log(LogLevel::ERROR, "Unknown shader stage");
                return vk::ShaderStageFlagBits::eAll;
        }
    }

    bool createPipelineShaderStageCreateInfo(
            const ShaderProgram&          shaderProgram,
            ShaderStage                         stage,
            vk::Device                          device,
            vk::PipelineShaderStageCreateInfo*  outCreateInfo) {

        assert(outCreateInfo);
        std::vector<char>           code = shaderProgram.getShader(stage).shaderCode;
        vk::ShaderModuleCreateInfo  vertexModuleInfo({}, code.size(), reinterpret_cast<uint32_t*>(code.data()));
        vk::ShaderModule            shaderModule;
        if (device.createShaderModule(&vertexModuleInfo, nullptr, &shaderModule) != vk::Result::eSuccess)
            return false;

        const static auto entryName = "main";

        *outCreateInfo = vk::PipelineShaderStageCreateInfo(
                {},
                shaderStageToVkShaderStage(stage),
                shaderModule,
                entryName,
                nullptr);
        return true;
    }
	
	/**
	 * Fills Vertex Attribute and Binding Description with the corresponding objects form the Vertex Layout.
	 * @param vertexAttributeDescriptions
	 * @param vertexBindingDescriptions
	 * @param existsVertexShader
	 * @param config
	 */
	void fillVertexInputDescription(
			std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions,
			std::vector<vk::VertexInputBindingDescription>   &vertexBindingDescriptions,
			const bool existsVertexShader,
			const GraphicsPipelineConfig &config) {
		
		if (existsVertexShader) {
			const VertexLayout& layout = config.m_VertexLayout;
			
			// iterate over the layout's specified, mutually exclusive buffer bindings that make up a vertex buffer
			for (const auto& vertexBinding : layout.vertexBindings)
			{
				vertexBindingDescriptions.emplace_back(vertexBinding.bindingLocation,
													   vertexBinding.stride,
													   vk::VertexInputRate::eVertex);
				
				// iterate over the bindings' specified, mutually exclusive vertex input attachments that make up a vertex
				for (const auto& vertexAttachment : vertexBinding.vertexAttachments)
				{
					vertexAttributeDescriptions.emplace_back(vertexAttachment.inputLocation,
															 vertexBinding.bindingLocation,
															 vertexFormatToVulkanFormat(vertexAttachment.format),
															 vertexAttachment.offset % vertexBinding.stride);
				}
			}
		}
	}
	
	/**
	 * Creates a Pipeline Vertex Input State Create Info Struct and fills it with Attribute and Binding data.
	 * @param vertexAttributeDescriptions
	 * @param vertexBindingDescriptions
	 * @return Pipeline Vertex Input State Create Info Struct
	 */
	vk::PipelineVertexInputStateCreateInfo createPipelineVertexInputStateCreateInfo(
			std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions,
			std::vector<vk::VertexInputBindingDescription>   &vertexBindingDescriptions) {
		
		vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
				{},
				vertexBindingDescriptions.size(),
				vertexBindingDescriptions.data(),
				vertexAttributeDescriptions.size(),
				vertexAttributeDescriptions.data()
		);
		return pipelineVertexInputStateCreateInfo;
	}
	
	/**
	 * Creates a Pipeline Input Assembly State Create Info Struct with 'Primitive Restart' disabled.
	 * @param config provides data for primitive topology.
	 * @return Pipeline Input Assembly State Create Info Struct
	 */
	vk::PipelineInputAssemblyStateCreateInfo createPipelineInputAssemblyStateCreateInfo(const GraphicsPipelineConfig &config) {
		vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
				{},
				primitiveTopologyToVulkanPrimitiveTopology(config.m_PrimitiveTopology),
				false
		);
		
		return pipelineInputAssemblyStateCreateInfo;
	}
	
	/**
	 * Creates a Pipeline Viewport State Create Info Struct with default set viewport and scissor settings.
	 * @param config provides with and height of the output window
	 * @return Pipeline Viewport State Create Info Struct
	 */
	vk::PipelineViewportStateCreateInfo createPipelineViewportStateCreateInfo(const GraphicsPipelineConfig &config) {
		static vk::Viewport viewport;
		static vk::Rect2D scissor;
		
		viewport = vk::Viewport(
				0.f, 0.f,
				static_cast<float>(config.m_Width),
				static_cast<float>(config.m_Height),
				0.f, 1.f
		);
		
		scissor = vk::Rect2D(
				{ 0,0 },
				{ config.m_Width, config.m_Height }
		);
		
		vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(
				{},
				1,
				&viewport,
				1,
				&scissor
		);
		
		return pipelineViewportStateCreateInfo;
	}
	
	/**
	 * Creates a Pipeline Rasterization State Create Info Struct with default values set to:
	 * Rasterizer Discard: Disabled
	 * Polygon Mode: Fill
	 * Front Face: Counter Clockwise
	 * Depth Bias: Disabled
	 * Line Width: 1.0
	 * Depth Clamping and Culling Mode ist set by the Pipeline Config
	 * @param config sets Depth Clamping and Culling Mode
	 * @return Pipeline Rasterization State Create Info Struct
	 */
	vk::PipelineRasterizationStateCreateInfo createPipelineRasterizationStateCreateInfo(
		const GraphicsPipelineConfig &config,
		const vk::PhysicalDeviceConservativeRasterizationPropertiesEXT& conservativeRasterProperties) {
		vk::CullModeFlags cullMode;
		switch (config.m_culling) {
			case CullMode::None:
				cullMode = vk::CullModeFlagBits::eNone;
				break;
			case CullMode::Front:
				cullMode = vk::CullModeFlagBits::eFront;
				break;
			case CullMode::Back:
				cullMode = vk::CullModeFlagBits::eBack;
				break;
			default:
			vkcv_log(LogLevel::ERROR, "Unknown CullMode");
				cullMode = vk::CullModeFlagBits::eNone;
		}
		
		vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo (
				{},
				config.m_EnableDepthClamping,
				false,
				vk::PolygonMode::eFill,
				cullMode,
				vk::FrontFace::eCounterClockwise,
				false,
				0.f,
				0.f,
				0.f,
				1.f
		);

		static vk::PipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterization;
		
		if (config.m_UseConservativeRasterization) {
			conservativeRasterization = vk::PipelineRasterizationConservativeStateCreateInfoEXT(
					{},
					vk::ConservativeRasterizationModeEXT::eOverestimate,
					std::max(1 - conservativeRasterProperties.primitiveOverestimationSize, 0.f)
			);
			
			pipelineRasterizationStateCreateInfo.pNext = &conservativeRasterization;
		}
		
		return pipelineRasterizationStateCreateInfo;
	}
	
	/**
	 * Creates a Pipeline Multisample State Create Info Struct.
	 * @param config set MSAA Sample Count Flag
	 * @return Pipeline Multisample State Create Info Struct
	 */
	vk::PipelineMultisampleStateCreateInfo createPipelineMultisampleStateCreateInfo(const GraphicsPipelineConfig &config) {
		vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
				{},
				msaaToVkSampleCountFlag(config.m_multisampling),
				false,
				0.f,
				nullptr,
				config.m_alphaToCoverage,
				false
		);
		
		return pipelineMultisampleStateCreateInfo;
	}
	
	/**
	 * Creates a Pipeline Color Blend State Create Info Struct.
	 * Currently only one blend mode is supported! There for, blending is set to additive.
	 * @param config sets blend mode
	 * @return
	 */
	vk::PipelineColorBlendStateCreateInfo createPipelineColorBlendStateCreateInfo(const GraphicsPipelineConfig &config) {
		// currently set to additive, if not disabled
		// BlendFactors must be set as soon as additional BlendModes are added
		static vk::PipelineColorBlendAttachmentState colorBlendAttachmentState (
				config.m_blendMode != BlendMode::None,
				vk::BlendFactor::eOne,
				vk::BlendFactor::eOne,
				vk::BlendOp::eAdd,
				vk::BlendFactor::eOne,
				vk::BlendFactor::eOne,
				vk::BlendOp::eAdd,
				vk::ColorComponentFlags(
						VK_COLOR_COMPONENT_R_BIT |
						VK_COLOR_COMPONENT_G_BIT |
						VK_COLOR_COMPONENT_B_BIT |
						VK_COLOR_COMPONENT_A_BIT
				)
		);
		
		vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
				{},
				false,
				vk::LogicOp::eClear,
				1,	//TODO: hardcoded to one
				&colorBlendAttachmentState,
				{ 1.f,1.f,1.f,1.f }
		);
		
		return pipelineColorBlendStateCreateInfo;
	}
	
	/**
	 * Creates a Pipeline Layout Create Info Struct.
	 * @param config sets Push Constant Size and Descriptor Layouts.
	 * @return Pipeline Layout Create Info Struct
	 */
	vk::PipelineLayoutCreateInfo createPipelineLayoutCreateInfo(const GraphicsPipelineConfig &config) {
		static vk::PushConstantRange pushConstantRange;
		
		const size_t pushConstantSize = config.m_ShaderProgram.getPushConstantSize();
		pushConstantRange = vk::PushConstantRange(
				vk::ShaderStageFlagBits::eAll, 0, pushConstantSize
		);
		
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
				{},
				(config.m_DescriptorLayouts),
				(pushConstantRange)
		);
		
		if (pushConstantSize == 0) {
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		}
		
		return pipelineLayoutCreateInfo;
	}
	
	/**
	 * Creates a Pipeline Depth Stencil State Create Info Struct.
	 * @param config sets if depth test in enabled or not.
	 * @return Pipeline Layout Create Info Struct
	 */
	vk::PipelineDepthStencilStateCreateInfo createPipelineDepthStencilStateCreateInfo(const GraphicsPipelineConfig &config) {
		const vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilCreateInfo(
				vk::PipelineDepthStencilStateCreateFlags(),
				config.m_depthTest != DepthTest::None,
				config.m_depthWrite,
				depthTestToVkCompareOp(config.m_depthTest),
				false,
				false,
				{},
				{},
				0.0f,
				1.0f
		);
		
		return pipelineDepthStencilCreateInfo;
	}
	
	/**
	 * Creates a Pipeline Dynamic State Create Info Struct.
	 * @param config sets whenever a dynamic viewport is used or not.
	 * @return Pipeline Dynamic State Create Info Struct
	 */
	vk::PipelineDynamicStateCreateInfo createPipelineDynamicStateCreateInfo(const GraphicsPipelineConfig &config) {
		static std::vector<vk::DynamicState> dynamicStates;
		dynamicStates.clear();
		
		if(config.m_UseDynamicViewport) {
			dynamicStates.push_back(vk::DynamicState::eViewport);
			dynamicStates.push_back(vk::DynamicState::eScissor);
		}
		
		vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo(
				{},
				static_cast<uint32_t>(dynamicStates.size()),
				dynamicStates.data()
		);
		
		return dynamicStateCreateInfo;
	}

    GraphicsPipelineHandle GraphicsPipelineManager::createPipeline(const GraphicsPipelineConfig &config, PassManager& passManager) {
        const vk::RenderPass &pass = passManager.getVkPass(config.m_PassHandle);

        const bool existsTaskShader     = config.m_ShaderProgram.existsShader(ShaderStage::TASK);
        const bool existsMeshShader     = config.m_ShaderProgram.existsShader(ShaderStage::MESH);
        const bool existsVertexShader   = config.m_ShaderProgram.existsShader(ShaderStage::VERTEX);
        const bool existsFragmentShader = config.m_ShaderProgram.existsShader(ShaderStage::FRAGMENT);
        const bool existsGeometryShader = config.m_ShaderProgram.existsShader(ShaderStage::GEOMETRY);
        const bool validGeometryStages  = existsVertexShader || (existsTaskShader && existsMeshShader);

        if (!validGeometryStages)
        {
            vkcv_log(LogLevel::ERROR, "Requires vertex or task and mesh shader");
            return GraphicsPipelineHandle();
        }

        if (!existsFragmentShader) {
            vkcv_log(LogLevel::ERROR, "Requires fragment shader code");
            return GraphicsPipelineHandle();
        }

        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
        auto destroyShaderModules = [&shaderStages, this] {
            for (auto stage : shaderStages) {
                m_Device.destroyShaderModule(stage.module);
            }
            shaderStages.clear();
        };

        if (existsVertexShader) {
            vk::PipelineShaderStageCreateInfo createInfo;
            const bool success = createPipelineShaderStageCreateInfo(
                    config.m_ShaderProgram,
                    ShaderStage::VERTEX,
                    m_Device,
                    &createInfo);

            if (success) {
                shaderStages.push_back(createInfo);
            }
            else {
                destroyShaderModules();
                return GraphicsPipelineHandle();
            }
        }

        if (existsTaskShader) {
            vk::PipelineShaderStageCreateInfo createInfo;
            const bool success = createPipelineShaderStageCreateInfo(
                    config.m_ShaderProgram,
                    ShaderStage::TASK,
                    m_Device,
                    &createInfo);

            if (success) {
                shaderStages.push_back(createInfo);
            }
            else {
                destroyShaderModules();
                return GraphicsPipelineHandle();
            }
        }

        if (existsMeshShader) {
            vk::PipelineShaderStageCreateInfo createInfo;
            const bool success = createPipelineShaderStageCreateInfo(
                    config.m_ShaderProgram,
                    ShaderStage::MESH,
                    m_Device,
                    &createInfo);

            if (success) {
                shaderStages.push_back(createInfo);
            }
            else {
                destroyShaderModules();
                return GraphicsPipelineHandle();
            }
        }

        {
            vk::PipelineShaderStageCreateInfo createInfo;
            const bool success = createPipelineShaderStageCreateInfo(
                    config.m_ShaderProgram,
                    ShaderStage::FRAGMENT,
                    m_Device,
                    &createInfo);

            if (success) {
                shaderStages.push_back(createInfo);
            }
            else {
                destroyShaderModules();
                return GraphicsPipelineHandle();
            }
        }

        if (existsGeometryShader) {
            vk::PipelineShaderStageCreateInfo createInfo;
            const bool success = createPipelineShaderStageCreateInfo(
                    config.m_ShaderProgram,
                    ShaderStage::GEOMETRY,
                    m_Device,
                    &createInfo);

            if (success) {
                shaderStages.push_back(createInfo);
            }
            else {
                destroyShaderModules();
                return GraphicsPipelineHandle();
            }
        }

        // vertex input state
        // Fill up VertexInputBindingDescription and VertexInputAttributeDescription Containers
        std::vector<vk::VertexInputAttributeDescription>	vertexAttributeDescriptions;
        std::vector<vk::VertexInputBindingDescription>		vertexBindingDescriptions;
        fillVertexInputDescription(vertexAttributeDescriptions, vertexBindingDescriptions, existsVertexShader, config);

        // Handover Containers to PipelineVertexInputStateCreateIngo Struct
        vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo =
                createPipelineVertexInputStateCreateInfo(vertexAttributeDescriptions,
                                                         vertexBindingDescriptions);

        // input assembly state
        vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo =
                createPipelineInputAssemblyStateCreateInfo(config);

        // viewport state
        vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo =
                createPipelineViewportStateCreateInfo(config);

        // rasterization state
        vk::PhysicalDeviceConservativeRasterizationPropertiesEXT    conservativeRasterProperties;
        vk::PhysicalDeviceProperties                                deviceProperties;
        vk::PhysicalDeviceProperties2                               deviceProperties2(deviceProperties);
        deviceProperties2.pNext = &conservativeRasterProperties;
        m_physicalDevice.getProperties2(&deviceProperties2);
        vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo =
                createPipelineRasterizationStateCreateInfo(config, conservativeRasterProperties);

        // multisample state
        vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo =
                createPipelineMultisampleStateCreateInfo(config);

        // color blend state
        vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo =
                createPipelineColorBlendStateCreateInfo(config);

        // Dynamic State
        vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo =
                createPipelineDynamicStateCreateInfo(config);

        // pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo =
                createPipelineLayoutCreateInfo(config);

        vk::PipelineLayout vkPipelineLayout{};
        if (m_Device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout) != vk::Result::eSuccess) {
            destroyShaderModules();
            return GraphicsPipelineHandle();
        }

        // Depth Stencil
        const vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo =
                createPipelineDepthStencilStateCreateInfo(config);

        const vk::PipelineDepthStencilStateCreateInfo* p_depthStencilCreateInfo = nullptr;
        const PassConfig& passConfig = passManager.getPassConfig(config.m_PassHandle);

        for (const auto& attachment : passConfig.attachments) {
            if (isDepthFormat(attachment.format)) {
                p_depthStencilCreateInfo = &depthStencilCreateInfo;
                break;
            }
        }

        // Get all setting structs together and create the Pipeline
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
            // Catch runtime error if the creation of the pipeline fails.
            // Destroy everything to keep the memory clean.
            destroyShaderModules();
            return GraphicsPipelineHandle();
        }

        // Clean Up
        destroyShaderModules();

        // Hand over Handler to main Application
        const uint64_t id = m_Pipelines.size();
        m_Pipelines.push_back({ vkPipeline, vkPipelineLayout, config });
        return GraphicsPipelineHandle(id, [&](uint64_t id) { destroyPipelineById(id); });
    }

    vk::Pipeline GraphicsPipelineManager::getVkPipeline(const GraphicsPipelineHandle &handle) const
    {
        const uint64_t id = handle.getId();

        if (id >= m_Pipelines.size()) {
            return nullptr;
        }

        auto& pipeline = m_Pipelines[id];

        return pipeline.m_handle;
    }

    vk::PipelineLayout GraphicsPipelineManager::getVkPipelineLayout(const GraphicsPipelineHandle &handle) const
    {
        const uint64_t id = handle.getId();

        if (id >= m_Pipelines.size()) {
            return nullptr;
        }

        auto& pipeline = m_Pipelines[id];

        return pipeline.m_layout;
    }

    void GraphicsPipelineManager::destroyPipelineById(uint64_t id) {
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

    const GraphicsPipelineConfig& GraphicsPipelineManager::getPipelineConfig(const GraphicsPipelineHandle &handle) const
    {
        const uint64_t id = handle.getId();

        if (id >= m_Pipelines.size()) {
            static GraphicsPipelineConfig dummyConfig;
            vkcv_log(LogLevel::ERROR, "Invalid handle");
            return dummyConfig;
        }

        return m_Pipelines[id].m_config;
    }
}
