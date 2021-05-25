#include "DescriptorManager.hpp"
#include <vkcv/ShaderProgram.hpp>


namespace vkcv
{
    DescriptorManager::DescriptorManager(vk::Device device) noexcept:
        m_Device{ device }, m_NextDescriptorSetID{ 1 }
    {
        m_DescriptorTypes = { DescriptorType::UNIFORM_BUFFER, DescriptorType::SAMPLER, DescriptorType::IMAGE };
        uint32_t sizesPerType = 1000;
        uint32_t maxSetsPerPool = 1000;

        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes;
        for (int i = 0; i < m_DescriptorTypes.size(); i++) {
            vk::DescriptorType type = convertDescriptorTypeFlag(m_DescriptorTypes[i]);
            vk::DescriptorPoolSize poolSize(type, sizesPerType);
            descriptorPoolSizes.push_back(poolSize);
        }
        vk::DescriptorPoolCreateInfo poolInfo({}, maxSetsPerPool, descriptorPoolSizes);
        m_Pool = m_Device.createDescriptorPool(poolInfo, nullptr);
    }

    ResourcesHandle DescriptorManager::createResourceDescription(const std::vector<DescriptorSet> &p_descriptorSets) 
    {
        ResourceDescription resource{};

        for (int i = 0; i < p_descriptorSets.size(); i++) {
            DescriptorSet set = p_descriptorSets[i];
            std::vector<vk::DescriptorSetLayoutBinding> setBindings;
            for (int j = 0; j < set.bindings.size(); j++) {
                //creates each binding of the set
                DescriptorBinding binding = set.bindings[j];
                vk::DescriptorType type = convertDescriptorTypeFlag(binding.descriptorType);
                vk::ShaderStageFlagBits stage = convertShaderStageFlag(binding.shaderStage);
                vk::DescriptorSetLayoutBinding descriptorSetLayoutBinding(binding.bindingID, type, binding.descriptorCount, stage);
                setBindings.push_back(descriptorSetLayoutBinding);
            }
            //creates the descriptor set layouts
            vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo({}, setBindings);
            vk::DescriptorSetLayout allocLayout = m_Device.createDescriptorSetLayout(descriptorSetLayoutInfo, nullptr);
            std::vector<vk::DescriptorSetLayout> allocLayouts(set.setCount, allocLayout);
            //binds the layout to the pool
            vk::DescriptorSetAllocateInfo allocInfo(m_Pool, allocLayout);

            //creates and allocates the set(s) based on the layout
            vk::DescriptorSet allocSet;
            std::vector<vk::DescriptorSet> allocSets(set.setCount, allocSet);
            m_Device.allocateDescriptorSets(&allocInfo, allocSets.data());

            //inserts the descriptor sets and layouts into the resources (also considers copies of the same sets)
            resource.descriptorSetLayouts.insert(resource.descriptorSetLayouts.begin(), allocLayouts.begin(), allocLayouts.end());
            resource.descriptorSets.insert(resource.descriptorSets.end(), allocSets.begin(), allocSets.end());
        }
        m_ResourceDescriptions.push_back(resource);
        return ResourcesHandle{m_NextDescriptorSetID++};
    }

    vk::DescriptorType DescriptorManager::convertDescriptorTypeFlag(DescriptorType type) {
        switch (type)
        {
        case vkcv::DescriptorType::UNIFORM_BUFFER:
            return vk::DescriptorType::eUniformBuffer;
        case vkcv::DescriptorType::SAMPLER:
            return vk::DescriptorType::eSampler;
        case vkcv::DescriptorType::IMAGE:
            return vk::DescriptorType::eSampledImage;
        }
    }

    vk::ShaderStageFlagBits DescriptorManager::convertShaderStageFlag(ShaderStage stage) {
        switch (stage) 
        {
        case vkcv::ShaderStage::VERTEX:
            return vk::ShaderStageFlagBits::eVertex;
        case vkcv::ShaderStage::FRAGMENT:
            return vk::ShaderStageFlagBits::eFragment;
        case vkcv::ShaderStage::TESS_CONTROL:
            return vk::ShaderStageFlagBits::eTessellationControl;
        case vkcv::ShaderStage::TESS_EVAL:
            return vk::ShaderStageFlagBits::eTessellationControl;
        case vkcv::ShaderStage::GEOMETRY:
            return vk::ShaderStageFlagBits::eGeometry;
        case vkcv::ShaderStage::COMPUTE:
            return vk::ShaderStageFlagBits::eCompute;
        }
    }

}