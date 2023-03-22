#include "GraphicsPipelineManager.hpp"

#include "vkcv/Core.hpp"
#include "vkcv/Image.hpp"
#include "vkcv/Logger.hpp"
#include "vkcv/Multisampling.hpp"

namespace vkcv {

	uint64_t GraphicsPipelineManager::getIdFrom(const GraphicsPipelineHandle &handle) const {
		return handle.getId();
	}

	GraphicsPipelineHandle
	GraphicsPipelineManager::createById(uint64_t id, const HandleDestroyFunction &destroy) {
		return GraphicsPipelineHandle(id, destroy);
	}

	void GraphicsPipelineManager::destroyById(uint64_t id) {
		auto &pipeline = getById(id);

		if (pipeline.m_handle) {
			getCore().getContext().getDevice().destroy(pipeline.m_handle);
			pipeline.m_handle = nullptr;
		}

		if (pipeline.m_layout) {
			getCore().getContext().getDevice().destroy(pipeline.m_layout);
			pipeline.m_layout = nullptr;
		}
	}

	GraphicsPipelineManager::GraphicsPipelineManager() noexcept :
		HandleManager<GraphicsPipelineEntry, GraphicsPipelineHandle>() {}

	GraphicsPipelineManager::~GraphicsPipelineManager() noexcept {
		clear();
	}

	// currently assuming default 32 bit formats, no lower precision or normalized variants
	// supported
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

	vk::PrimitiveTopology
	primitiveTopologyToVulkanPrimitiveTopology(const PrimitiveTopology topology) {
		switch (topology) {
		case (PrimitiveTopology::PointList):
			return vk::PrimitiveTopology::ePointList;
		case (PrimitiveTopology::LineList):
			return vk::PrimitiveTopology::eLineList;
		case (PrimitiveTopology::TriangleList):
			return vk::PrimitiveTopology::eTriangleList;
		case (PrimitiveTopology::PatchList):
			return vk::PrimitiveTopology::ePatchList;
		default:
			vkcv_log(LogLevel::ERROR, "Unknown primitive topology type");
			return vk::PrimitiveTopology::eTriangleList;
		}
	}

	vk::CompareOp depthTestToVkCompareOp(DepthTest depthTest) {
		switch (depthTest) {
		case (DepthTest::None):
			return vk::CompareOp::eAlways;
		case (DepthTest::Less):
			return vk::CompareOp::eLess;
		case (DepthTest::LessEqual):
			return vk::CompareOp::eLessOrEqual;
		case (DepthTest::Greater):
			return vk::CompareOp::eGreater;
		case (DepthTest::GreatherEqual):
			return vk::CompareOp::eGreaterOrEqual;
		case (DepthTest::Equal):
			return vk::CompareOp::eEqual;
		default:
			vkcv_log(LogLevel::ERROR, "Unknown depth test enum");
			return vk::CompareOp::eAlways;
		}
	}

	static vk::ShaderStageFlagBits shaderStageToVkShaderStage(ShaderStage stage) {
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
			return vk::ShaderStageFlagBits::eTaskEXT;
		case ShaderStage::MESH:
			return vk::ShaderStageFlagBits::eMeshEXT;
		default:
			vkcv_log(LogLevel::ERROR, "Unknown shader stage");
			return vk::ShaderStageFlagBits::eAll;
		}
	}
	
	static bool createPipelineShaderStageCreateInfo(
			const ShaderProgram &shaderProgram,
			ShaderStage stage,
			vk::Device device,
			vk::PipelineShaderStageCreateInfo* outCreateInfo) {

		assert(outCreateInfo);
		std::vector<uint32_t> code = shaderProgram.getShaderBinary(stage);
		vk::ShaderModuleCreateInfo vertexModuleInfo(
				{},
				code.size() * sizeof(uint32_t),
				code.data()
		);
		vk::ShaderModule shaderModule;
		if (device.createShaderModule(&vertexModuleInfo, nullptr, &shaderModule)
			!= vk::Result::eSuccess)
			return false;

		const static auto entryName = "main";
		*outCreateInfo = vk::PipelineShaderStageCreateInfo(
				vk::PipelineShaderStageCreateFlags(),
				shaderStageToVkShaderStage(stage),
				shaderModule,
				entryName,
				nullptr
		);
		return true;
	}

	/**
	 * Fills Vertex Attribute and Binding Description with the corresponding objects form the Vertex
	 * Layout.
	 * @param vertexAttributeDescriptions
	 * @param vertexBindingDescriptions
	 * @param existsVertexShader
	 * @param config
	 */
	static void fillVertexInputDescription(
		std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions,
		std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions,
		const bool existsVertexShader,
		const GraphicsPipelineConfig &config) {

		if (existsVertexShader) {
			const VertexLayout &layout = config.getVertexLayout();

			// iterate over the layout's specified, mutually exclusive buffer bindings that make up
			// a vertex buffer
			for (const auto &vertexBinding : layout.vertexBindings) {
				vertexBindingDescriptions.emplace_back(vertexBinding.bindingLocation,
													   vertexBinding.stride,
													   vk::VertexInputRate::eVertex);

				// iterate over the bindings' specified, mutually exclusive vertex input attachments
				// that make up a vertex
				for (const auto &vertexAttachment : vertexBinding.vertexAttachments) {
					vertexAttributeDescriptions.emplace_back(
						vertexAttachment.inputLocation, vertexBinding.bindingLocation,
						vertexFormatToVulkanFormat(vertexAttachment.format),
						vertexAttachment.offset % vertexBinding.stride);
				}
			}
		}
	}

	/**
	 * Creates a Pipeline Vertex Input State Create Info Struct and fills it with Attribute and
	 * Binding data.
	 * @param vertexAttributeDescriptions
	 * @param vertexBindingDescriptions
	 * @return Pipeline Vertex Input State Create Info Struct
	 */
	static vk::PipelineVertexInputStateCreateInfo createPipelineVertexInputStateCreateInfo(
		std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions,
		std::vector<vk::VertexInputBindingDescription> &vertexBindingDescriptions) {

		vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo(
			{}, vertexBindingDescriptions.size(), vertexBindingDescriptions.data(),
			vertexAttributeDescriptions.size(), vertexAttributeDescriptions.data());
		return pipelineVertexInputStateCreateInfo;
	}

	/**
	 * Creates a Pipeline Input Assembly State Create Info Struct with 'Primitive Restart' disabled.
	 * @param config provides data for primitive topology.
	 * @return Pipeline Input Assembly State Create Info Struct
	 */
	static vk::PipelineInputAssemblyStateCreateInfo
	createPipelineInputAssemblyStateCreateInfo(const GraphicsPipelineConfig &config) {
		vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
			{}, primitiveTopologyToVulkanPrimitiveTopology(config.getPrimitiveTopology()), false);

		return pipelineInputAssemblyStateCreateInfo;
	}
	
	static vk::PipelineTessellationStateCreateInfo
	createPipelineTessellationStateCreateInfo(const GraphicsPipelineConfig &config) {
		vk::PipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo(
			{},
			config.getTesselationControlPoints()
		);

		return pipelineTessellationStateCreateInfo;
	}

	/**
	 * Creates a Pipeline Viewport State Create Info Struct with default set viewport and scissor
	 * settings.
	 * @param config provides with and height of the output window
	 * @return Pipeline Viewport State Create Info Struct
	 */
	static vk::PipelineViewportStateCreateInfo
	createPipelineViewportStateCreateInfo(const GraphicsPipelineConfig &config) {
		static vk::Viewport viewport;
		static vk::Rect2D scissor;

		viewport = vk::Viewport(0.f, 0.f, static_cast<float>(config.getWidth()),
								static_cast<float>(config.getHeight()), 0.f, 1.f);

		scissor = vk::Rect2D({ 0, 0 }, { config.getWidth(), config.getHeight() });

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
	static vk::PipelineRasterizationStateCreateInfo createPipelineRasterizationStateCreateInfo(
		const GraphicsPipelineConfig &config,
		const vk::PhysicalDeviceConservativeRasterizationPropertiesEXT
			&conservativeRasterProperties) {
		vk::CullModeFlags cullMode;
		switch (config.getCulling()) {
		case CullMode::None:
			cullMode = vk::CullModeFlagBits::eNone;
			break;
		case CullMode::Front:
			cullMode = vk::CullModeFlagBits::eFront;
			break;
		case CullMode::Back:
			cullMode = vk::CullModeFlagBits::eBack;
			break;
		case CullMode::Both:
			cullMode = vk::CullModeFlagBits::eFrontAndBack;
			break;
		default:
			vkcv_log(LogLevel::ERROR, "Unknown CullMode");
			cullMode = vk::CullModeFlagBits::eNone;
		}

		vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(
			{},
			config.isDepthClampingEnabled(),
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

		if (config.isUsingConservativeRasterization()) {
			const float overestimationSize =
				1.0f - conservativeRasterProperties.primitiveOverestimationSize;
			const float maxOverestimationSize =
				conservativeRasterProperties.maxExtraPrimitiveOverestimationSize;

			conservativeRasterization = vk::PipelineRasterizationConservativeStateCreateInfoEXT(
				{}, vk::ConservativeRasterizationModeEXT::eOverestimate,
				std::min(std::max(overestimationSize, 0.f), maxOverestimationSize));

			pipelineRasterizationStateCreateInfo.pNext = &conservativeRasterization;
		}

		return pipelineRasterizationStateCreateInfo;
	}

	/**
	 * Creates a Pipeline Multisample State Create Info Struct.
	 * @param config set MSAA Sample Count Flag
	 * @return Pipeline Multisample State Create Info Struct
	 */
	static vk::PipelineMultisampleStateCreateInfo
	createPipelineMultisampleStateCreateInfo(const GraphicsPipelineConfig &config,
											 const PassConfig &passConfig) {
		vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo(
			{}, msaaToSampleCountFlagBits(passConfig.getMultisampling()), false, 0.f, nullptr,
			config.isWritingAlphaToCoverage(), false);

		return pipelineMultisampleStateCreateInfo;
	}

	/**
	 * Creates a Pipeline Color Blend State Create Info Struct.
	 * Currently only one blend mode is supported! There for, blending is set to additive.
	 * @param config sets blend mode
	 * @return
	 */
	static vk::PipelineColorBlendStateCreateInfo
	createPipelineColorBlendStateCreateInfo(const GraphicsPipelineConfig &config,
											const PassConfig &passConfig) {
		static std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates;
		
		colorBlendAttachmentStates.clear();
		colorBlendAttachmentStates.reserve(passConfig.getAttachments().size());
		
		for (const auto& attachment : passConfig.getAttachments()) {
			if ((isDepthFormat(attachment.getFormat())) ||
				(isStencilFormat(attachment.getFormat()))) {
				continue;
			}
			
			// currently set to additive, if not disabled
			// BlendFactors must be set as soon as additional BlendModes are added
			vk::PipelineColorBlendAttachmentState colorBlendAttachmentState (
					config.getBlendMode() != BlendMode::None,
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
			
			colorBlendAttachmentStates.push_back(colorBlendAttachmentState);
		}

		vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
			{},
			false,
			vk::LogicOp::eClear,
			colorBlendAttachmentStates.size(),
			colorBlendAttachmentStates.data(),
			{ 1.f, 1.f, 1.f, 1.f }
		);

		return pipelineColorBlendStateCreateInfo;
	}

	/**
	 * Creates a Pipeline Layout Create Info Struct.
	 * @param config sets Push Constant Size and Descriptor Layouts.
	 * @return Pipeline Layout Create Info Struct
	 */
	static vk::PipelineLayoutCreateInfo createPipelineLayoutCreateInfo(
		const GraphicsPipelineConfig &config,
		const std::vector<vk::DescriptorSetLayout> &descriptorSetLayouts) {
		static vk::PushConstantRange pushConstantRange;

		const size_t pushConstantsSize = config.getShaderProgram().getPushConstantsSize();
		pushConstantRange = vk::PushConstantRange(
				vk::ShaderStageFlagBits::eAll,
				0,
				pushConstantsSize
		);

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo (
				vk::PipelineLayoutCreateFlags(),
				descriptorSetLayouts,
				pushConstantRange
		);

		if (pushConstantsSize == 0) {
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		}

		return pipelineLayoutCreateInfo;
	}

	/**
	 * Creates a Pipeline Depth Stencil State Create Info Struct.
	 * @param config sets if depth test in enabled or not.
	 * @return Pipeline Layout Create Info Struct
	 */
	static vk::PipelineDepthStencilStateCreateInfo
	createPipelineDepthStencilStateCreateInfo(const GraphicsPipelineConfig &config) {
		const vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilCreateInfo(
			vk::PipelineDepthStencilStateCreateFlags(),
			config.getDepthTest() != DepthTest::None,
			config.isWritingDepth(),
			depthTestToVkCompareOp(config.getDepthTest()),
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
	static vk::PipelineDynamicStateCreateInfo
	createPipelineDynamicStateCreateInfo(const GraphicsPipelineConfig &config) {
		static std::vector<vk::DynamicState> dynamicStates;
		dynamicStates.clear();

		if (config.isViewportDynamic()) {
			dynamicStates.push_back(vk::DynamicState::eViewport);
			dynamicStates.push_back(vk::DynamicState::eScissor);
		}

		vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo(
			{}, static_cast<uint32_t>(dynamicStates.size()), dynamicStates.data());

		return dynamicStateCreateInfo;
	}

	GraphicsPipelineHandle
	GraphicsPipelineManager::createPipeline(const GraphicsPipelineConfig &config,
											const PassManager &passManager,
											const DescriptorSetLayoutManager &descriptorManager) {
		const vk::RenderPass &pass = passManager.getVkPass(config.getPass());

		const auto &program = config.getShaderProgram();

		const bool existsTaskShader = program.existsShader(ShaderStage::TASK);
		const bool existsMeshShader = program.existsShader(ShaderStage::MESH);
		const bool existsVertexShader = program.existsShader(ShaderStage::VERTEX);
		const bool existsFragmentShader = program.existsShader(ShaderStage::FRAGMENT);
		const bool existsGeometryShader = program.existsShader(ShaderStage::GEOMETRY);
		const bool existsTessellationControlShader =
			program.existsShader(ShaderStage::TESS_CONTROL);
		const bool existsTessellationEvaluationShader =
			program.existsShader(ShaderStage::TESS_EVAL);

		const bool validGeometryStages =
			((existsVertexShader
			  && (existsTessellationControlShader == existsTessellationEvaluationShader))
			 || (existsTaskShader && existsMeshShader));

		if (!validGeometryStages) {
			vkcv_log(LogLevel::ERROR, "Requires a valid geometry shader stage");
			return {};
		}

		if (!existsFragmentShader) {
			vkcv_log(LogLevel::ERROR, "Requires fragment shader code");
			return {};
		}

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		auto destroyShaderModules = [&shaderStages, this] {
			for (auto stage : shaderStages) {
				getCore().getContext().getDevice().destroyShaderModule(stage.module);
			}
			shaderStages.clear();
		};

		if (existsVertexShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
				program, ShaderStage::VERTEX, getCore().getContext().getDevice(), &createInfo);

			if (success) {
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}

		if (existsTaskShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
				program, ShaderStage::TASK, getCore().getContext().getDevice(), &createInfo);

			if (success) {
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}

		if (existsMeshShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
				program, ShaderStage::MESH, getCore().getContext().getDevice(), &createInfo);

			if (success) {
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}

		{
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
				program, ShaderStage::FRAGMENT, getCore().getContext().getDevice(), &createInfo);

			if (success) {
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}

		if (existsGeometryShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
				program, ShaderStage::GEOMETRY, getCore().getContext().getDevice(), &createInfo);

			if (success) {
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}

		if (existsTessellationControlShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
				program, ShaderStage::TESS_CONTROL, getCore().getContext().getDevice(),
				&createInfo);

			if (success) {
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}

		if (existsTessellationEvaluationShader) {
			vk::PipelineShaderStageCreateInfo createInfo;
			const bool success = createPipelineShaderStageCreateInfo(
				program, ShaderStage::TESS_EVAL, getCore().getContext().getDevice(), &createInfo);

			if (success) {
				shaderStages.push_back(createInfo);
			} else {
				destroyShaderModules();
				return {};
			}
		}

		const PassConfig &passConfig = passManager.getPassConfig(config.getPass());

		// vertex input state
		// Fill up VertexInputBindingDescription and VertexInputAttributeDescription Containers
		std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
		std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions;
		fillVertexInputDescription(vertexAttributeDescriptions, vertexBindingDescriptions,
								   existsVertexShader, config);

		// Handover Containers to PipelineVertexInputStateCreateIngo Struct
		vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo =
			createPipelineVertexInputStateCreateInfo(vertexAttributeDescriptions,
													 vertexBindingDescriptions);

		// input assembly state
		vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo =
			createPipelineInputAssemblyStateCreateInfo(config);

		// tesselation state
		vk::PipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo =
			createPipelineTessellationStateCreateInfo(config);

		// viewport state
		vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo =
			createPipelineViewportStateCreateInfo(config);

		// rasterization state
		vk::PhysicalDeviceConservativeRasterizationPropertiesEXT conservativeRasterProperties;
		vk::PhysicalDeviceProperties deviceProperties;
		vk::PhysicalDeviceProperties2 deviceProperties2(deviceProperties);
		deviceProperties2.pNext = &conservativeRasterProperties;
		getCore().getContext().getPhysicalDevice().getProperties2(&deviceProperties2);
		vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo =
			createPipelineRasterizationStateCreateInfo(config, conservativeRasterProperties);

		// multisample state
		vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo =
			createPipelineMultisampleStateCreateInfo(config, passConfig);

		// color blend state
		vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo =
			createPipelineColorBlendStateCreateInfo(config, passConfig);

		// Dynamic State
		vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo =
			createPipelineDynamicStateCreateInfo(config);

		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.reserve(config.getDescriptorSetLayouts().size());
		for (const auto &handle : config.getDescriptorSetLayouts()) {
			descriptorSetLayouts.push_back(
				descriptorManager.getDescriptorSetLayout(handle).vulkanHandle);
		}

		// pipeline layout
		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = createPipelineLayoutCreateInfo(
				config,
				descriptorSetLayouts
		);

		const auto& pipelineLayout = (
				getCore().getContext().getDevice().createPipelineLayout(pipelineLayoutCreateInfo)
		);
		
		if (!pipelineLayout) {
			destroyShaderModules();
			return {};
		}

		// Depth Stencil
		const vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo =
			createPipelineDepthStencilStateCreateInfo(config);

		const vk::PipelineDepthStencilStateCreateInfo* p_depthStencilCreateInfo = nullptr;

		for (const auto &attachment : passConfig.getAttachments()) {
			if ((isDepthFormat(attachment.getFormat()))
				|| (isStencilFormat(attachment.getFormat()))) {
				p_depthStencilCreateInfo = &depthStencilCreateInfo;
				break;
			}
		}
		
		const bool usesTesselation = (
				existsTessellationControlShader &&
				existsTessellationEvaluationShader
		);

		// Get all setting structs together and create the Pipeline
		const vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(
			{},
			static_cast<uint32_t>(shaderStages.size()),
			shaderStages.data(),
			existsVertexShader? &pipelineVertexInputStateCreateInfo : nullptr,
			existsVertexShader? &pipelineInputAssemblyStateCreateInfo : nullptr,
			usesTesselation? &pipelineTessellationStateCreateInfo : nullptr,
			&pipelineViewportStateCreateInfo,
			&pipelineRasterizationStateCreateInfo,
			&pipelineMultisampleStateCreateInfo,
			p_depthStencilCreateInfo,
			&pipelineColorBlendStateCreateInfo,
			&dynamicStateCreateInfo,
			pipelineLayout,
			pass,
			0,
			{},
			0
		);
		
		auto pipelineResult = getCore().getContext().getDevice().createGraphicsPipeline(
				nullptr, graphicsPipelineCreateInfo
		);
		
		if (pipelineResult.result != vk::Result::eSuccess) {
			// Catch runtime error if the creation of the pipeline fails.
			// Destroy everything to keep the memory clean.
			getCore().getContext().getDevice().destroy(pipelineLayout);
			destroyShaderModules();
			return {};
		}

		// Clean Up
		destroyShaderModules();

		// Hand over Handler to main Application
		return add({ pipelineResult.value, pipelineLayout, config });
	}

	vk::Pipeline
	GraphicsPipelineManager::getVkPipeline(const GraphicsPipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return pipeline.m_handle;
	}

	vk::PipelineLayout
	GraphicsPipelineManager::getVkPipelineLayout(const GraphicsPipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return pipeline.m_layout;
	}

	const GraphicsPipelineConfig &
	GraphicsPipelineManager::getPipelineConfig(const GraphicsPipelineHandle &handle) const {
		auto &pipeline = (*this) [handle];
		return pipeline.m_config;
	}

} // namespace vkcv
