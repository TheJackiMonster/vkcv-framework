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

    PipelineHandle PipelineManager::createPipeline(const PipelineConfig &config, PassManager& passManager) {
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
            return PipelineHandle();
        }

        if (!existsFragmentShader) {
            vkcv_log(LogLevel::ERROR, "Requires fragment shader code");
            return PipelineHandle();
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
                return PipelineHandle();
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
                return PipelineHandle();
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
                return PipelineHandle();
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
                return PipelineHandle();
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
                return PipelineHandle();
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
        vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo =
                createPipelineRasterizationStateCreateInfo(config);

        // multisample state
        vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo =
                createPipelineMultisampleStateCreateInfo(config);

        // color blend state
        vk::ColorComponentFlags colorWriteMask(VK_COLOR_COMPONENT_R_BIT |
                                               VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT |
                                               VK_COLOR_COMPONENT_A_BIT);

        // currently set to additive, if not disabled
        // BlendFactors must be set as soon as additional BlendModes are added
        vk::PipelineColorBlendAttachmentState colorBlendAttachmentState(
                config.m_blendMode != BlendMode::None,
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

        const size_t pushConstantSize = config.m_ShaderProgram.getPushConstantSize();
        const vk::PushConstantRange pushConstantRange(vk::ShaderStageFlagBits::eAll, 0, pushConstantSize);

        // pipeline layout
        vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo(
                {},
                (config.m_DescriptorLayouts),
                (pushConstantRange));
        if (pushConstantSize == 0) {
            pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        }

        vk::PipelineLayout vkPipelineLayout{};
        if (m_Device.createPipelineLayout(&pipelineLayoutCreateInfo, nullptr, &vkPipelineLayout) != vk::Result::eSuccess)
        {
            destroyShaderModules();
            return PipelineHandle();
        }

        const vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo(
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

        vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo(
                {},
                static_cast<uint32_t>(dynamicStates.size()),
                dynamicStates.data());

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
            destroyShaderModules();
            return PipelineHandle();
        }

        destroyShaderModules();

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

    void PipelineManager::fillVertexInputDescription(
        std::vector<vk::VertexInputAttributeDescription> &vertexAttributeDescriptions,
        std::vector<vk::VertexInputBindingDescription>   &vertexBindingDescriptions,
        const bool existsVertexShader,
        const PipelineConfig &config) {

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

    vk::PipelineVertexInputStateCreateInfo PipelineManager::createPipelineVertexInputStateCreateInfo(
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

    vk::PipelineInputAssemblyStateCreateInfo
    PipelineManager::createPipelineInputAssemblyStateCreateInfo(const PipelineConfig &config) {
        vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(
                {},
                primitiveTopologyToVulkanPrimitiveTopology(config.m_PrimitiveTopology),
                false
        );
        return pipelineInputAssemblyStateCreateInfo;
    }

    vk::PipelineViewportStateCreateInfo
    PipelineManager::createPipelineViewportStateCreateInfo(const PipelineConfig &config) {
        vk::Viewport viewport(0.f, 0.f,
                              static_cast<float>(config.m_Width),
                              static_cast<float>(config.m_Height),
                              0.f, 1.f);

        vk::Rect2D scissor({ 0,0 },
                           { config.m_Width,
                             config.m_Height });

        vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo({},
                                                                            1,
                                                                            &viewport,
                                                                            1,
                                                                            &scissor);
        return pipelineViewportStateCreateInfo;
    }

    vk::PipelineRasterizationStateCreateInfo
    PipelineManager::createPipelineRasterizationStateCreateInfo(const PipelineConfig &config) {

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

        vk::PipelineRasterizationConservativeStateCreateInfoEXT conservativeRasterization;
        if (config.m_UseConservativeRasterization) {
            conservativeRasterization = vk::PipelineRasterizationConservativeStateCreateInfoEXT(
                    {},
                    vk::ConservativeRasterizationModeEXT::eOverestimate,
                    0.f);
            pipelineRasterizationStateCreateInfo.pNext = &conservativeRasterization;
        }

        return pipelineRasterizationStateCreateInfo;
    }

    vk::PipelineMultisampleStateCreateInfo
    PipelineManager::createPipelineMultisampleStateCreateInfo(const PipelineConfig &config) {
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
}
