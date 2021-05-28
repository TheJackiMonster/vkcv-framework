#include "DescriptorManager.hpp"

namespace vkcv
{
    DescriptorManager::ResourceDescription::ResourceDescription(std::vector<vk::DescriptorSet> sets,
                                                                std::vector<vk::DescriptorSetLayout> layouts) noexcept :
    descriptorSets{std::move(sets)},
    descriptorSetLayouts{std::move(layouts)}
    {}
    DescriptorManager::DescriptorManager(vk::Device device) noexcept:
        m_Device{ device }, m_NextResourceDescriptionID{ 1 }
    {
        /**
         * Allocate a set size for the initial pool, namely 1000 units of each descriptor type below.
         */
        const std::vector<vk::DescriptorPoolSize> poolSizes = {vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 1000),
                                                    vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 1000),
                                                    vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1000),
                                                    vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1000)};

        vk::DescriptorPoolCreateInfo poolInfo({},
                                              1000,
                                              static_cast<uint32_t>(poolSizes.size()),
                                              poolSizes.data());

        if(m_Device.createDescriptorPool(&poolInfo, nullptr, &m_Pool) != vk::Result::eSuccess)
        {
            std::cout << "FAILED TO ALLOCATED DESCRIPTOR POOL." << std::endl;
            m_Pool = nullptr;
        };
    }

    DescriptorManager::~DescriptorManager() noexcept
    {
        for(const auto &resource : m_ResourceDescriptions)
        {
            for(const auto &layout : resource.descriptorSetLayouts)
                m_Device.destroyDescriptorSetLayout(layout);
        }
        m_Device.destroy(m_Pool);
    }

    ResourcesHandle DescriptorManager::createResourceDescription(const std::vector<DescriptorSet> &descriptorSets)
    {
        std::vector<vk::DescriptorSet> vk_sets;
        std::vector<vk::DescriptorSetLayout> vk_setLayouts;

        for (const auto &set : descriptorSets) {
            std::vector<vk::DescriptorSetLayoutBinding> setBindings = {};

            //create each set's binding
            for (uint32_t j = 0; j < set.bindings.size(); j++) {
                vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(
                        j,
                        convertDescriptorTypeFlag(set.bindings[j].descriptorType),
                        set.bindings[j].descriptorCount,
                        convertShaderStageFlag(set.bindings[j].shaderStage));
                setBindings.push_back(descriptorSetLayoutBinding);
            }

            //create the descriptor set's layout from the bindings gathered above
            vk::DescriptorSetLayoutCreateInfo layoutInfo({}, setBindings);
            vk::DescriptorSetLayout layout = nullptr;
            if(m_Device.createDescriptorSetLayout(&layoutInfo, nullptr, &layout) != vk::Result::eSuccess)
            {
                std::cout << "FAILED TO CREATE DESCRIPTOR SET LAYOUT" << std::endl;
                return ResourcesHandle{0};
            };
            vk_setLayouts.push_back(layout);
        }
        //create and allocate the set(s) based on the layouts that have been gathered above
        vk_sets.resize(vk_setLayouts.size());
        vk::DescriptorSetAllocateInfo allocInfo(m_Pool, vk_sets.size(), vk_setLayouts.data());
        auto result = m_Device.allocateDescriptorSets(&allocInfo, vk_sets.data());
        if(result != vk::Result::eSuccess)
        {
            std::cout << "FAILED TO ALLOCATE DESCRIPTOR SET" << std::endl;
            std::cout << vk::to_string(result) << std::endl;
            for(const auto &layout : vk_setLayouts)
                m_Device.destroy(layout);

            return ResourcesHandle{0};
        };

        m_ResourceDescriptions.emplace_back(vk_sets, vk_setLayouts);
        return ResourcesHandle{m_NextResourceDescriptionID++};
    }

    vk::DescriptorType DescriptorManager::convertDescriptorTypeFlag(DescriptorType type) {
        switch (type)
        {
            case DescriptorType::UNIFORM_BUFFER:
                return vk::DescriptorType::eUniformBuffer;
            case DescriptorType::STORAGE_BUFFER:
                return vk::DescriptorType::eStorageBuffer;
            case DescriptorType::SAMPLER:
                return vk::DescriptorType::eSampler;
            case DescriptorType::IMAGE:
                return vk::DescriptorType::eSampledImage;
            default:
                return vk::DescriptorType::eUniformBuffer;
        }
    }

    vk::ShaderStageFlagBits DescriptorManager::convertShaderStageFlag(ShaderStage stage) {
        switch (stage) 
        {
            case ShaderStage::VERTEX:
                return vk::ShaderStageFlagBits::eVertex;
            case ShaderStage::FRAGMENT:
                return vk::ShaderStageFlagBits::eFragment;
            case ShaderStage::TESS_CONTROL:
                return vk::ShaderStageFlagBits::eTessellationControl;
            case ShaderStage::TESS_EVAL:
                return vk::ShaderStageFlagBits::eTessellationEvaluation;
            case ShaderStage::GEOMETRY:
                return vk::ShaderStageFlagBits::eGeometry;
            case ShaderStage::COMPUTE:
                return vk::ShaderStageFlagBits::eCompute;
            default:
                return vk::ShaderStageFlagBits::eAll;
        }
    }

}